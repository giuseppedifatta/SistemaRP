#ifndef SSLCLIENT_H
#define SSLCLIENT_H
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/conf.h>
#include <string>

#include "useragentrp.h"


using namespace std;
class UserAgentRP;
class SSLClient
{
private:

    //dati membro per la connessione SSL
    SSL_CTX *ctx;
    BIO* outbio;
    int server_sock;
    SSL * ssl;
    const char * Server_IPaddress;

    UserAgentRP *userAgentChiamante;

    //metodi privati
    //funzioni per l'inizializzazione di SSL e la configurazione del contesto SSL
    void init_openssl_library();
    void cleanup_openssl();
    void createClientContext();
    void configure_context(const char *CertFile, const char *KeyFile, const char *ChainFile);
    int create_socket(const char * port);

    //elaborazione certificati
    void ShowCerts();
    void verify_ServerCert();



    void sendString_SSL(SSL *ssl, string s);
    int receiveString_SSL(SSL *ssl, string &s);
public:
    SSLClient(UserAgentRP *userAgentRP);

    ~SSLClient();

    //connessione all'host
    SSL* connectTo(const char* hostIP/*hostname*/);



    //richieste per l'Urna
    bool queryAutenticazioneRP(string username, string password, string &xmlFileProcedure, string &saltScrutinio);
    bool queryScrutinio(uint idProcedura, string derivedKey, string &xmlProcedureAggiornate);
    bool queryRisultatiVoto(uint idProcedura, string &risultatiScrutinioXML, string &encodedSignRP);


    enum serviziUrna { //richiedente
        //attivazionePV = 0, //postazionevoto
        //attivazioneSeggio = 1, //seggio
        //infoProcedura, //seggio
        //infoSessione, //seggio
        risultatiVoto  = 4, //seggio
        //invioSchedeCompilate = 5//, //postazionevoto
        scrutinio = 6, //responsabile procedimento
        autenticazioneRP = 7, //responsabile procedimento
        //tryVoteElettore = 8,
        //infoMatricola = 9,
        //setMatricolaVoted = 10,
        //checkConnection = 11,
        //resetMatricolaStatoVoto = 12
    };

};


#endif // SSLCLIENT_H
