#ifndef USERAGENTRP_H
#define USERAGENTRP_H

#include <QtCore>
#include <QThread>


#include <string>
#include <iostream>
#include <vector>
#include <mutex>

#include <cryptopp/pwdbased.h>
#include <cryptopp/hex.h>
#include <cryptopp/filters.h>
#include <cryptopp/secblock.h>
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>
#include <cryptopp/aes.h>
#include <cryptopp/rsa.h>
#include <cryptopp/pssr.h>
#include <tinyxml2.h>

#include "sslclient.h"
#include "proceduravoto.h"
#include "risultatiSeggio.h"

using namespace std;
using namespace CryptoPP;
using namespace tinyxml2;

class UserAgentRP : public QThread{
    Q_OBJECT
public:
    mutex mutex_stdout;
    explicit UserAgentRP(QObject *parent = 0);


    string getPassword() const;
    void setPassword(const string &value);

    enum statiProcedura: unsigned int{
        creazione,
        programmata,
        in_corso,
        conclusa,
        scrutinata,
        undefined
    };

    enum autenticato {
            authenticated,
            not_authenticated
        };


    string getUserid() const;
    void setUserid(const string &value);
    string deriveKeyFromPass(string password, string salt);
    string hashPassword(string plainPass, string salt);
    string getPublicKeyRP() const;
    void setPublicKeyRP(const string &value);

    uint getIdProceduraSelezionata() const;


    void totaleSchede(uint numeroSchede);
private:

    //dati membro
    string ipUrna;
    string userid;
    string password;
    string saltScrutinio;
    uint idRP;
    string publicKeyRP;
    uint idProceduraSelezionata;


    //funzioni membro

    vector <ProceduraVoto> parsingProcedure(string xmlFileProcedure);


    int verifySignString_RP(string data, string encodedSignature, string encodedPublicKey);
    void parsingScrutinioXML(string &risultatiVotoXML, vector<RisultatiSeggio> *risultatiSeggi);
    void doScrutinio();
signals:
    void autenticazione_riuscita(vector <ProceduraVoto>);
    void errorCredenziali();
    void urnaNonRaggiungibile();
    void scrutinioInCorso();
    void scrutinioOK();
    void erroreScrutinio();
    void erroreRicezioneRisultati();
    void readyRisultatiSeggi(vector <RisultatiSeggio> risultatiSeggi);
    void schedeDaScrutinare(uint numeroSchede);
public slots:
    void run();
    void doLogin(QString username, QString password);
    void setIdProceduraSelezionata(const uint &value);

    void visualizzaRisultatiVoto(uint idProceduraSelezionata);
};

#endif // USERAGENTRP_H
