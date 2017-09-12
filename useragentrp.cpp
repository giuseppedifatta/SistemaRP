#include "useragentrp.h"

UserAgentRP::UserAgentRP(QObject *parent) : QObject(parent)
{
    ipUrna = "192.168.19.130";
}

string UserAgentRP::deriveKeyFromPass(string password)
{
    string derivedKey;
    try {

        // KDF parameters
        unsigned int iterations = 15000;
        char purpose = 0; // unused by Crypto++

        // 32 bytes of derived material. Used to key the cipher.
        //   16 bytes are for the key, and 16 bytes are for the iv.
        SecByteBlock derived(AES::MAX_KEYLENGTH);

        // KDF function
        CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> kdf;
        kdf.DeriveKey(derived.data(), derived.size(), purpose, (byte*)password.data(), password.size(), NULL, 0, iterations);




        // Encode derived
        HexEncoder hex(new StringSink(derivedKey));
        hex.Put(derived.data(), derived.size());
        hex.MessageEnd();

        // Print stuff
        cout << "pass: " << password << endl;
        //cout << "derived key: " << derivedKey << endl;

    }
    catch(CryptoPP::Exception& ex)
    {
        cerr << ex.what() << endl;
    }
    return derivedKey;
}

vector<ProceduraVoto> UserAgentRP::parsingProcedure(string xmlFileProcedure)
{

}

void UserAgentRP::login(QString username, QString password)
{
    SSLClient * rp_client = new SSLClient(this);
    if(rp_client->connectTo(ipUrna)){
        string xmlFileProcedure;
        if(rp_client->queryAutenticazioneRP(username.toStdString(),password.toStdString(),xmlFileProcedure)){
            cout << "xml delle procedure riceuto: " << endl;
            cout << xmlFileProcedure << endl;
            vector <ProceduraVoto> procedureRP = parsingProcedure(xmlFileProcedure);
            emit autenticazione_riuscita(procedureRP);
            this->password = password.toStdString();
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
    string derivedKey = deriveKeyFromPass(password);
    SSLClient * rp_client = new SSLClient(this);
    if(rp_client->connectTo(ipUrna)){
        if(rp_client->queryScrutinio(idProceduraSelezionata,derivedKey)){
            emit scrutinioOK();
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
    emit totaleSchede(numeroSchede);
}

