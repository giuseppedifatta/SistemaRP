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

#include "sslclient.h"

using namespace std;
using namespace CryptoPP;

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
private:
    const char * ipUrna;
    string deriveKeyFromPass(string password);
    string password;
signals:

public slots:
    void login(QString username, QString password);
};

#endif // USERAGENTRP_H
