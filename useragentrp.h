#ifndef USERAGENTRP_H
#define USERAGENTRP_H

#include <QtCore>


#include <string>
#include <iostream>
#include <vector>
#include <mutex>

#include <cryptopp/pwdbased.h>
#include "cryptopp/hex.h"
#include "cryptopp/filters.h"
#include "cryptopp/secblock.h"
#include "cryptopp/hmac.h"
#include "cryptopp/sha.h"
#include "cryptopp/aes.h"
#include "tinyxml2.h"

#include "sslclient.h"
#include "proceduravoto.h"

using namespace std;
using namespace CryptoPP;
using namespace tinyxml2;

class UserAgentRP : public QObject
{
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

    void oneMoreVoteScrutinato();
    void totaleSchede(uint numeroSchede);
    string getUserid() const;
    void setUserid(const string &value);
    string deriveKeyFromPass(string password, string salt);
    string hashPassword(string plainPass, string salt);
private:
    //dati membro
    const char * ipUrna;
    string userid;
    string password;
    string saltScrutinio;
    uint idRP;


    //funzioni membro

    vector <ProceduraVoto> parsingProcedure(string xmlFileProcedure);


signals:
    void autenticazione_riuscita(vector <ProceduraVoto>);
    void errorCredenziali();
    void urnaNonRaggiungibile();
    void oneMore();
    void schedeDaScrutinare(uint numeroSchede);
    void scrutinioOK();
    void erroreScrutinio();

public slots:
    void login(QString username, QString password);
    void doScrutinio(uint idProceduraSelezionata);
    void visualizzaRisultatiVoto(uint idProceduraSelezionata);
};

#endif // USERAGENTRP_H
