#include "useragentrp.h"
#include "conf.h"


UserAgentRP::UserAgentRP(QObject *parent) : QThread(parent)
{
    ipUrna = getConfig("ipUrna").c_str(); //da file di configurazione .config.rp
}

string UserAgentRP::deriveKeyFromPass(string password,string salt)
{
    string encodedDerivedKey;
    try {

        // KDF parameters
        unsigned int iterations = 15000;
        char purpose = 0; // unused by Crypto++

        // 32 bytes of derived material. Used to key the cipher.
        //   16 bytes are for the key, and 16 bytes are for the iv.
        SecByteBlock derived(AES::MAX_KEYLENGTH);

        // KDF function
        CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> kdf;
        kdf.DeriveKey(derived.data(), derived.size(), purpose, (byte*)password.data(), password.size(), (byte *) salt.data(), salt.size(), iterations);




        // Encode derived
        HexEncoder hex(new StringSink(encodedDerivedKey));
        hex.Put(derived.data(), derived.size());
        hex.MessageEnd();

        // Print stuff
        cout << "pass: " << password << endl;
        cout << "derived key: " << encodedDerivedKey << endl;

    }
    catch(CryptoPP::Exception& ex)
    {
        cerr << ex.what() << endl;
    }
    return encodedDerivedKey;
}

vector<ProceduraVoto> UserAgentRP::parsingProcedure(string xmlFileProcedure)
{
    vector <ProceduraVoto> procedure;
    XMLDocument xmlDoc;
    xmlDoc.Parse(xmlFileProcedure.c_str());

    XMLNode *rootNode = xmlDoc.FirstChild(); //procedureVotoRP

    if(rootNode->FirstChildElement("procedura")==nullptr){
        return procedure;
    }

    //primo e ultimo elemento procedura
    XMLElement * firstProceduraElement = rootNode->FirstChildElement("procedura");
    XMLElement * lastProceduraElement = rootNode->LastChildElement("procedura");

    XMLElement *proceduraElement = firstProceduraElement;
    bool lastProcedura = false;
    do{
        ProceduraVoto pv;
        //id
        XMLElement * idElement = proceduraElement->FirstChildElement("id");
        uint id;
        const char * text = idElement->FirstChild()->ToText()->Value();
        id = atoi(text);
        //descrizione
        XMLElement * descrizioneElement = proceduraElement->FirstChildElement("descrizione");
        string descrizione = descrizioneElement->FirstChild()->ToText()->Value();


        //inizio
        XMLElement * inizioElement = proceduraElement->FirstChildElement("inizio");
        string inizio = inizioElement->FirstChild()->ToText()->Value();


        //fine
        XMLElement * fineElement = proceduraElement->FirstChildElement("fine");
        string fine = fineElement->FirstChild()->ToText()->Value();

        //stato
        XMLElement * statoElement = proceduraElement->FirstChildElement("stato");
        const char * s = statoElement->FirstChild()->ToText()->Value();
        uint  stato = atoi(s);

        pv.setIdProceduraVoto(id);
        pv.setDescrizione(descrizione);
        pv.setData_ora_inizio(inizio);
        pv.setData_ora_termine(fine);
        pv.setStato(stato);

        procedure.push_back(pv);

        if(proceduraElement == lastProceduraElement){
            lastProcedura = true;
        }
        else{
            //accediamo alla successiva procedura
            proceduraElement = proceduraElement->NextSiblingElement("procedura");
            cout << "ottengo il puntatore alla successiva procedura" << endl;
        }
    }while(!lastProcedura);
    cout << "non ci sono altre procedure da parsare" << endl;


    return procedure;
}

void UserAgentRP::login(QString username, QString password)
{
    SSLClient * rp_client = new SSLClient(this);
    if(rp_client->connectTo(ipUrna)){
        string xmlFileProcedure;
        string saltScrutinio;
        if(rp_client->queryAutenticazioneRP(username.toStdString(),password.toStdString(),xmlFileProcedure,saltScrutinio)){
            cout << "xml delle procedure ricevuto: " << endl;
            cout << xmlFileProcedure << endl;
            vector <ProceduraVoto> procedureRP = parsingProcedure(xmlFileProcedure);
            emit autenticazione_riuscita(procedureRP);
            this->password = password.toStdString();
            this->userid = username.toStdString();
            this->saltScrutinio = saltScrutinio;
        }
        else{
            emit errorCredenziali();
        }

    }
    else{
        cerr << "collegamento con l'urna non riuscito" << endl;
        emit urnaNonRaggiungibile();
    }
    delete rp_client;
}

void UserAgentRP::doScrutinio(uint idProceduraSelezionata)
{
    string derivedKey = deriveKeyFromPass(password,saltScrutinio);
    SSLClient * rp_client = new SSLClient(this);
    if(rp_client->connectTo(ipUrna)){
        string xmlProcedureAggiornate;
        if(rp_client->queryScrutinio(idProceduraSelezionata,derivedKey, xmlProcedureAggiornate)){
            emit scrutinioOK();
            vector <ProceduraVoto> procedure = this->parsingProcedure(xmlProcedureAggiornate);
            emit this->autenticazione_riuscita(procedure);//riutilizzo il segnale che è collegato allo slot per la visualizzazione delle procedure
        }
        else{
            emit erroreScrutinio();
        }

    }
    else{
        cerr << "collegamento con l'urna non riuscito" << endl;
        emit urnaNonRaggiungibile();
    }
    delete rp_client;
}

void UserAgentRP::visualizzaRisultatiVoto(uint idProceduraSelezionata)
{
    vector <RisultatiSeggio> risultatiSeggi;

    SSLClient * rp_client = new SSLClient(this);
    if(rp_client->connectTo(ipUrna)){
        string risultatiVotoXML,encodedSignRP;
        if(rp_client->queryRisultatiVoto(idProceduraSelezionata,risultatiVotoXML,encodedSignRP)){

            //verifica firma di RP sui risultatiRicevuti
            int verifica = this->verifySignString_RP(risultatiVotoXML+to_string(idProceduraSelezionata),encodedSignRP,publicKeyRP);
            if(verifica == 0){
                this->parsingScrutinioXML(risultatiVotoXML, &risultatiSeggi);
                emit readyRisultatiSeggi(risultatiSeggi);
            }


        }
        else{
            //emit risultatiNonRicevuti();
        }

    }
    else{
        cerr << "collegamento con l'urna non riuscito" << endl;
        emit urnaNonRaggiungibile();
    }
}

void UserAgentRP::parsingScrutinioXML(string &risultatiVotoXML, vector <RisultatiSeggio> *risultatiSeggi)
{
    XMLDocument xmlDoc;
    xmlDoc.Parse(risultatiVotoXML.c_str());

    //nodo Scrutinio
    XMLNode *rootNode = xmlDoc.FirstChild();

    XMLNode *risultatiNode = rootNode->FirstChild();

    XMLElement * risultatoElement = risultatiNode->FirstChildElement("risultatoSeggio");
    bool oneMoreRisultatoElement = true;
    while(oneMoreRisultatoElement){
        oneMoreRisultatoElement = false;
        RisultatiSeggio rs;

        XMLText * textNodeIdSeggio = risultatoElement->FirstChildElement("idSeggio")->FirstChild()->ToText();
        uint idSeggio = atoi(textNodeIdSeggio->Value());
        cout << "Risultati seggio " << idSeggio << endl;
        rs.setIdSeggio(idSeggio);

        XMLElement *schedeElement = risultatoElement->FirstChildElement("schede");

        XMLElement *schedaRisultatoElement = schedeElement->FirstChildElement("schedaRisultato");
        bool oneMoreSchedaRisultato = true;
        vector <SchedaVoto> * schedeRisultato = rs.getPointerSchedeVotoRisultato();
        while(oneMoreSchedaRisultato){
            oneMoreSchedaRisultato = false;
            SchedaVoto svr;

            XMLText * textNodeIdScheda = schedaRisultatoElement->FirstChildElement("id")->FirstChild()->ToText();
            uint id = atoi(textNodeIdScheda->Value());
            svr.setId(id);

            XMLText * textNodeDesc = schedaRisultatoElement->FirstChildElement("descrizioneElezione")->FirstChild()->ToText();
            string descrizioneElezione = textNodeDesc->Value();
            svr.setDescrizioneElezione(descrizioneElezione);

            cout << "scheda " << id << ": " << descrizioneElezione <<  endl;

            XMLElement * listeElement = schedaRisultatoElement->FirstChildElement("liste");

            XMLElement * firstListaElement = listeElement->FirstChildElement("lista");
            XMLElement * lastListaElement = listeElement->LastChildElement("lista");

            XMLElement *listaElement = firstListaElement;
            bool lastLista = false;
            do{

                int idLista = listaElement->IntAttribute("id");
                cout << "PV:  --- lista trovata" << endl;
                cout << "PV: id Lista: " << idLista << endl;
                string nomeLista = listaElement->Attribute("nome");
                cout << "PV: nome: " << nomeLista << endl;

                XMLElement * firstCandidatoElement  = listaElement->FirstChildElement("candidato");
                XMLElement * lastCandidatoElement = listaElement->LastChildElement("candidato");

                XMLElement * candidatoElement = firstCandidatoElement;
                //ottengo tutti i candidati della lista
                bool lastCandidato = false;
                do{

                    XMLElement * matricolaElement = candidatoElement->FirstChildElement("matricola");
                    XMLNode * matricolaInnerNode = matricolaElement->FirstChild();
                    string matricola;
                    if(matricolaInnerNode!=nullptr){
                        matricola = matricolaInnerNode->ToText()->Value();
                    }

                    int numVoti = candidatoElement->IntAttribute("votiRicevuti");
                    //cout << matricola << endl;

                    XMLElement *nomeElement = matricolaElement->NextSiblingElement("nome");
                    XMLNode * nomeInnerNode = nomeElement->FirstChild();
                    string nome;
                    if(nomeInnerNode!=nullptr){
                        nome = nomeInnerNode->ToText()->Value();
                    }
                    //cout << nome << endl;

                    XMLElement *cognomeElement = nomeElement->NextSiblingElement("cognome");
                    XMLNode * cognomeInnerNode = cognomeElement->FirstChild();
                    string cognome;
                    if(cognomeInnerNode!=nullptr){
                        cognome = cognomeInnerNode->ToText()->Value();
                    }
                    //cout << cognome << endl;

                    XMLElement *luogoNascitaElement = cognomeElement->NextSiblingElement("luogoNascita");
                    XMLNode * luogoNascitaInnerNode = luogoNascitaElement->FirstChild();
                    string luogoNascita;
                    if(luogoNascitaInnerNode!=nullptr){
                        luogoNascita = luogoNascitaInnerNode->ToText()->Value();
                    }
                    //cout << luogoNascita << endl;

                    XMLElement *dataNascitaElement = luogoNascitaElement->NextSiblingElement("dataNascita");
                    XMLNode * dataNascitaInnerNode = dataNascitaElement->FirstChild();
                    string dataNascita;
                    if(dataNascitaInnerNode!=nullptr){
                        dataNascita = dataNascitaInnerNode->ToText()->Value();
                    }
                    //cout << dataNascita << endl;

                    cout << "PV: Estratti i dati del candidato id " << id;
                    //creo il candidato
                    cout << ": " << matricola << ", "<< nome << " " << cognome << ", " << luogoNascita << ", " << dataNascita << " ->" << numVoti << " voti." << endl;
                    Candidato c(nome,nomeLista,cognome,dataNascita,luogoNascita,matricola,numVoti);

                    svr.addCandidato(c);

                    //accesso al successivo candidato
                    if(candidatoElement == lastCandidatoElement){
                        lastCandidato = true;
                    }else {
                        candidatoElement = candidatoElement->NextSiblingElement("candidato");
                        cout << "PV: ottengo il puntatore al successivo candidato" << endl;
                    }
                }while(!lastCandidato);

                cout << "PV: non ci sono altri candidati nella lista: " << nomeLista << endl;


                if(listaElement == lastListaElement){
                    lastLista = true;
                }
                else{
                    //accediamo alla successiva lista nella scheda di voto
                    listaElement = listaElement->NextSiblingElement("lista");
                    cout << "PV: ottengo il puntatore alla successiva lista" << endl;
                }
            }while(!lastLista);
            cout << "PV: non ci sono altre liste" << endl;


            schedeRisultato->push_back(svr);

            if(schedaRisultatoElement->NextSiblingElement("schedaRisultato")!=nullptr){
                schedaRisultatoElement = schedaRisultatoElement->NextSiblingElement("schedaRisultato");
                oneMoreSchedaRisultato = true;
            }
        }

        risultatiSeggi->push_back(rs);

        if(risultatoElement->NextSiblingElement("risultatoSeggio")!=nullptr){
            risultatoElement = risultatoElement->NextSiblingElement("risultatoSeggio");
            oneMoreRisultatoElement = true;
        }
    }
}


string UserAgentRP::getPassword() const
{
    return password;
}




void UserAgentRP::setPassword(const string &value)
{
    password = value;
}

void UserAgentRP::oneMoreVoteScrutinato()
{
    emit oneMore();
}

void UserAgentRP::totaleSchede(uint numeroSchede)
{
    emit schedeDaScrutinare(numeroSchede);
}

string UserAgentRP::getUserid() const
{
    return userid;
}

void UserAgentRP::setUserid(const string &value)
{
    userid = value;
}

string UserAgentRP::hashPassword( string plainPass, string salt){
    SecByteBlock result(32);
    string hexResult;
    uint iterations = 10000;

    CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> pbkdf;

    pbkdf.DeriveKey(result, result.size(),0x00,(byte *) plainPass.data(), plainPass.size(),(byte *) salt.data(), salt.size(),iterations);

    //ArraySource resultEncoder(result,result.size(), true, new HexEncoder(new StringSink(hexResult)));

    HexEncoder hex(new StringSink(hexResult));
    hex.Put(result.data(), result.size());
    hex.MessageEnd();

    return hexResult;

}

string UserAgentRP::getPublicKeyRP() const
{
    return publicKeyRP;
}

void UserAgentRP::setPublicKeyRP(const string &value)
{
    publicKeyRP = value;
}

void UserAgentRP::run()
{

}

int UserAgentRP::verifySignString_RP(string data, string encodedSignature,
                                     string encodedPublicKey) {

    int success = 1; //non verificato
    string signature;
    StringSource(encodedSignature,true,
                 new HexDecoder(
                     new StringSink(signature)
                     )//HexDecoder
                 );//StringSource
    cout << "Signature encoded: " << encodedSignature << endl;
    cout << "Signature decoded: " << signature << endl;

    string decodedPublicKey;
    StringSource(encodedPublicKey,true,
                 new HexDecoder(
                     new StringSink(decodedPublicKey)
                     )//HexDecoder
                 );//StringSource

    ////------ verifica signature
    StringSource ss(decodedPublicKey,true /*pumpAll*/);
    CryptoPP::RSA::PublicKey publicKey;
    publicKey.Load(ss);

    cout << "PublicKey encoded: " << encodedPublicKey << endl;
    ////////////////////////////////////////////////
    try{
        // Verify and Recover
        CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA256>::Verifier verifier(publicKey);
        cout << "Data to sign|signature: " << data + signature << endl;
        StringSource(data + signature, true,
                     new SignatureVerificationFilter(verifier, NULL,
                                                     SignatureVerificationFilter::THROW_EXCEPTION) // SignatureVerificationFilter
                     );// StringSource

        cout << "Verified signature on message" << endl;
        success = 0; //verificato
    } // try

    catch (CryptoPP::Exception& e) {
        cerr << "Error: " << e.what() << endl;
        success = 1;
    }
    return success;
}
