#include "sslclient.h"

/*
 * sslClient.h
 *
 *  Created on: 02/apr/2017
 *      Author: giuseppe
 */

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/conf.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#define SERVER_PORT "4433"
#define LOCALHOST "127.0.0.1"

SSLClient::SSLClient(UserAgentRP * userAgentRP){
    //this->hostname = IP_PV;
    this->userAgentChiamante = userAgentRP;

    /* ---------------------------------------------------------- *
     * Create the Input/Output BIO's.                             *
     * ---------------------------------------------------------- */
    this->outbio = BIO_new_fp(stdout, BIO_NOCLOSE);
    this->ssl = nullptr;

    /* ---------------------------------------------------------- *
     * Function that initialize openssl for correct work.		  *
     * ---------------------------------------------------------- */
    this->init_openssl_library();

    createClientContext();

    char certFile[] = "/home/giuseppe/myCA/intermediate/certs/client.cert.pem";
    char keyFile[] = "/home/giuseppe/myCA/intermediate/private/client.key.pem";
    char chainFile[] =
            "/home/giuseppe/myCA/intermediate/certs/ca-chain.cert.pem";

    this->configure_context(certFile, keyFile, chainFile);
    userAgentChiamante->mutex_stdout.lock();
    cout << "ClientSeggio: context configured" << endl;
    userAgentChiamante->mutex_stdout.unlock();
}

SSLClient::~SSLClient(){
    /* ---------------------------------------------------------- *
     * Free the structures we don't need anymore                  *
     * -----------------------------------------------------------*/
    BIO_free_all(this->outbio);
    SSL_CTX_free(this->ctx);

    //usare con cura, cancella gli algoritmi e non funziona più nulla
    // this->cleanup_openssl();

}

int SSLClient::create_socket(/*const char * hostIP*//*hostname*/const char * port) {
    /* ---------------------------------------------------------- *
     * create_socket() creates the socket & TCP-connect to server *
     * non specifica per SSL                                      *
     * ---------------------------------------------------------- */

    const char *address_printable = NULL;

    /* decomentare per usare l'hostname
     * struct hostent *host;
     */
    struct sockaddr_in dest_addr;

    unsigned int portCod = atoi(port);
    /* decommentare la sezione se si passa alla funzione l'hostname invece dell'ip dell'host
    if ((host = gethostbyname(hostname)) == NULL) {
        BIO_printf(this->outbio, "Error: Cannot resolve hostname %s.\n", hostname);
        abort();
    }
    */

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(portCod);
    /* decommentare la sezione se si passa all'hostname invece dell'ip dell'host
    dest_addr.sin_addr.s_addr = *(long*) (host->h_addr);
    */

    // commentare la riga sotto se non si vuole usare l'ip dell'host, ma l'hostname
    dest_addr.sin_addr.s_addr = inet_addr(this->PV_IPaddress);

    /* ---------------------------------------------------------- *
     * create the basic TCP socket                                *
     * ---------------------------------------------------------- */
    this->server_sock = socket(AF_INET, SOCK_STREAM, 0);

    /* ---------------------------------------------------------- *
     * Zeroing the rest of the struct                             *
     * ---------------------------------------------------------- */
    memset(&(dest_addr.sin_zero), '\0', 8);

    address_printable = inet_ntoa(dest_addr.sin_addr);

    /* ---------------------------------------------------------- *
     * Try to make the host connect here                          *
    * ---------------------------------------------------------- */

    int res = connect(this->server_sock, (struct sockaddr *) &dest_addr, sizeof(struct sockaddr));

    if (res  == -1) {
        userAgentChiamante->mutex_stdout.lock();
        BIO_printf(this->outbio, "ClientSeggio: Error: Cannot connect to host %s [%s] on port %d.\n",
                   this->PV_IPaddress/*hostname*/, address_printable, portCod);
        userAgentChiamante->mutex_stdout.unlock();


    }
    else {
        userAgentChiamante->mutex_stdout.lock();
        BIO_printf(this->outbio, "ClientSeggio: Successfully connect to host %s [%s] on port %d.\n",
                   this->PV_IPaddress/*hostname*/, address_printable, portCod);
        cout << "ClientSeggio: Descrittore socket: " << this->server_sock << endl;
        userAgentChiamante->mutex_stdout.unlock();

    }

    return res;
}

SSL * SSLClient::connectTo(const char* hostIP/*hostname*/){
    userAgentChiamante->mutex_stdout.lock();
    cout << "ClientSeggio: Connecting to " << hostIP << endl;
    userAgentChiamante->mutex_stdout.unlock();

    this->PV_IPaddress = hostIP;

    const char  port[] = SERVER_PORT;

    /* ---------------------------------------------------------- *
     * Create new SSL connection state object                     *
     * ---------------------------------------------------------- */
    this->ssl = SSL_new(this->ctx);
    //    userAgentChiamante->mutex_stdout.lock();
    //    cout << "ClientSeggio: ssl pointer: " << this->ssl << endl;
    //    userAgentChiamante->mutex_stdout.unlock();


    /* ---------------------------------------------------------- *
     * Make the underlying TCP socket connection                  *
     * ---------------------------------------------------------- */
    int ret = create_socket(/*this->PV_IPaddress *//*hostname,*/port);


    if (ret == 0){

        userAgentChiamante->mutex_stdout.lock();
        BIO_printf(this->outbio,"ClientSeggio: Successfully create the socket for TCP connection to: %s \n",
                   this->PV_IPaddress /*hostname*/);
        userAgentChiamante->mutex_stdout.unlock();
    }
    else {


        userAgentChiamante->mutex_stdout.lock();
        BIO_printf(this->outbio,"ClientSeggio: Unable to create the socket for TCP connection to: %s \n",
                   this->PV_IPaddress /*hostname*/);
        userAgentChiamante->mutex_stdout.unlock();
        SSL_free(this->ssl);
        this->ssl = nullptr;
        if(close(this->server_sock) == 0){
            cerr << "ClientSeggio: chiusura 1 socket server" << endl;
        }
        return this->ssl;
    }

    /* ---------------------------------------------------------- *
     * Attach the SSL session to the socket descriptor            *
     * ---------------------------------------------------------- */

    if (SSL_set_fd(this->ssl, this->server_sock) != 1) {

        userAgentChiamante->mutex_stdout.lock();
        BIO_printf(this->outbio, "ClientSeggio: Error: Connection to %s failed \n", this->PV_IPaddress /*hostname*/);
        userAgentChiamante->mutex_stdout.unlock();

        SSL_free(this->ssl);
        this->ssl = nullptr;
        if(close(this->server_sock) == 0){
            cerr << "ClientSeggio: chiusura 2 socket server" << endl;
        }
        return this->ssl;
    }
    else
        userAgentChiamante->mutex_stdout.lock();
    BIO_printf(this->outbio, "ClientSeggio: Ok: Connection to %s \n", this->PV_IPaddress /*hostname*/);
    userAgentChiamante->mutex_stdout.unlock();
    /* ---------------------------------------------------------- *
     * Try to SSL-connect here, returns 1 for success             *
     * ---------------------------------------------------------- */
    ret = SSL_connect(this->ssl);
    if (ret != 1){ //SSL handshake

        userAgentChiamante->mutex_stdout.lock();
        BIO_printf(this->outbio, "ClientSeggio: Error: Could not build a SSL session to: %s\n",
                   this->PV_IPaddress /*hostname*/);
        userAgentChiamante->mutex_stdout.unlock();

        SSL_free(this->ssl);
        this->ssl = nullptr;
        if(close(this->server_sock) == 0){
            cerr << "ClientSeggio: chiusura 3 socket server" << endl;
        }
        return this->ssl;
    }
    else{
        userAgentChiamante->mutex_stdout.lock();
        BIO_printf(this->outbio, "ClientSeggio: Successfully enabled SSL/TLS session to: %s\n",
                   this->PV_IPaddress /*hostname*/);
        userAgentChiamante->mutex_stdout.unlock();
    }
    this->ShowCerts();
    this->verify_ServerCert(/*this->PV_IPaddress *//*hostname*/);

    return this->ssl;
}

bool SSLClient::queryAutenticazioneRP(string username, string password, string &xmlFileProcedure)
{
    //richiesta servizio
    int serviceCod = serviziUrna::autenticazioneRP;
    stringstream ssCod;
    ssCod << serviceCod;
    string strCod = ssCod.str();
    const char * charCod = strCod.c_str();
    userAgentChiamante->mutex_stdout.lock();
    cout << "ClientPV: richiedo il servizio: " << charCod << endl;
    userAgentChiamante->mutex_stdout.unlock();
    SSL_write(ssl,charCod,strlen(charCod));

    //invia username
    sendString_SSL(ssl,username);
    //invia password
    sendString_SSL(ssl,password);

    //ricevi esito autenticazione
    string str;
    receiveString_SSL(ssl, str);
    int esito = atoi(str.c_str());

    if(esito == 0){
        //ricevi stringa contenente il file xml con i dati delle procedure di cui RP è responsabile
        receiveString_SSL(ssl,xmlFileProcedure);
        return true;
    }
    else return false;


}

bool SSLClient::queryScrutinio(uint idProcedura, string derivedKey)
{
    //richiesta servizio
    int serviceCod = serviziUrna::scrutinio;
    stringstream ssCod;
    ssCod << serviceCod;
    string strCod = ssCod.str();
    const char * charCod = strCod.c_str();
    userAgentChiamante->mutex_stdout.lock();
    cout << "ClientPV: richiedo il servizio: " << charCod << endl;
    userAgentChiamante->mutex_stdout.unlock();
    SSL_write(ssl,charCod,strlen(charCod));

    //invia idProcedura da scrutinare
    sendString_SSL(ssl,to_string(idProcedura));

    //invia derivedKey per decifrare la chiave privata di RP
    sendString_SSL(ssl,derivedKey);

    //ricevi numero schede da scrutinare e segnalo alla view il numero di schede da scrutinare
    string s;
    receiveString_SSL(ssl,s);
    uint numeroSchede = atoi(s.c_str());
    userAgentChiamante->totaleSchede(numeroSchede);
    //ciclo for, ad ogni iterazione
    //ricevo che un'altra scheda è stata scrutinata
    // segnalo alla view che è stato scrutinato un'altro voto rispetto al totale, così da aggiornare la progress bar
    for(uint i=0; i < numeroSchede; i++){
        string onemore;
        receiveString_SSL(ssl,onemore);
        userAgentChiamante->oneMoreVoteScrutinato();
    }
    //terminato il ciclo for lo scrutinio è terminato

    //ricevi esitoScrutinio
    string str;
    receiveString_SSL(ssl, str);
    int esito = atoi(str.c_str());

    if(esito == 0){
        return true;
    }
    else return false;

}

bool SSLClient::queryRisultatiVoto(uint idProcedura)
{
    //richiesta servizio
    int serviceCod = serviziUrna::risultatiVoto;
    stringstream ssCod;
    ssCod << serviceCod;
    string strCod = ssCod.str();
    const char * charCod = strCod.c_str();
    userAgentChiamante->mutex_stdout.lock();
    cout << "ClientPV: richiedo il servizio: " << charCod << endl;
    userAgentChiamante->mutex_stdout.unlock();
    SSL_write(ssl,charCod,strlen(charCod));


    //ricevi file xml dei risultati di voto
}

void SSLClient::ShowCerts() {
    X509 *peer_cert;
    char *line;

    peer_cert = SSL_get_peer_certificate(this->ssl); /* Get certificates (if available) */

    //ERR_print_errors_fp(stderr);
    if (peer_cert != NULL) {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(peer_cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(peer_cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(peer_cert);
    } else
        printf("No certificates.\n");
}

void SSLClient::verify_ServerCert(/*const char * hostIPhostname*/) {
    // Declare X509 structure
    X509 *error_cert = NULL;
    X509 *peer_cert = NULL;
    X509_NAME *certsubject = NULL;
    X509_STORE *store = NULL;
    X509_STORE_CTX *vrfy_ctx = NULL;
    //X509_NAME *certname = NULL;
    int ret;
    //BIO *certbio = NULL;
    //certbio = BIO_new(BIO_s_file());


    // Get the remote certificate into the X509 structure

    peer_cert = SSL_get_peer_certificate(this->ssl);
    if (peer_cert == NULL){
        userAgentChiamante->mutex_stdout.lock();
        BIO_printf(this->outbio, "ClientSeggio: Error: Could not get a certificate from: %s.\n",
                   this->PV_IPaddress/*hostname*/);
        userAgentChiamante->mutex_stdout.unlock();
    }
    else{
        userAgentChiamante->mutex_stdout.lock();
        BIO_printf(this->outbio, "ClientSeggio: Retrieved the server's certificate from: %s.\n",
                   this->PV_IPaddress/*hostname*/);
        userAgentChiamante->mutex_stdout.unlock();
    }

    // extract various certificate information

    //certname = X509_NAME_new();
    //certname = X509_get_subject_name(peer_cert);

    // display the cert subject here

    //    BIO_printf(this->outbio, "ClientSeggio: Displaying the certificate subject data:\n");
    //    X509_NAME_print_ex(this->outbio, certname, 0, 0);
    //    BIO_printf(this->outbio, "\n");

    //Initialize the global certificate validation store object.
    if (!(store = X509_STORE_new())){
        userAgentChiamante->mutex_stdout.lock();
        BIO_printf(this->outbio, "ClientSeggio: Error creating X509_STORE_CTX object\n");
        userAgentChiamante->mutex_stdout.unlock();
    }
    // Create the context structure for the validation operation.
    vrfy_ctx = X509_STORE_CTX_new();

    // Load the certificate and cacert chain from file (PEM).
    /*
         ret = BIO_read_filename(certbio, certFile);
         if (!(cert = PEM_read_bio_X509(certbio, NULL, 0, NULL)))
         BIO_printf(this->outbio, "ClientSeggio: Error loading cert into memory\n");
         */
    char chainFile[] =
            "/home/giuseppe/myCA/intermediate/certs/ca-chain.cert.pem";

    ret = X509_STORE_load_locations(store, chainFile, NULL);
    if (ret != 1){
        userAgentChiamante->mutex_stdout.lock();
        BIO_printf(this->outbio, "ClientSeggio: Error loading CA cert or chain file\n");
        userAgentChiamante->mutex_stdout.unlock();
    }
    /* Initialize the ctx structure for a verification operation:
      Set the trusted cert store, the unvalidated cert, and any  *
     * potential certs that could be needed (here we set it NULL) */
    X509_STORE_CTX_init(vrfy_ctx, store, peer_cert, NULL);

    /* Check the complete cert chain can be build and validated.  *
     * Returns 1 on success, 0 on verification failures, and -1   *
     * for trouble with the ctx object (i.e. missing certificate) */
    ret = X509_verify_cert(vrfy_ctx);

    userAgentChiamante->mutex_stdout.lock();
    BIO_printf(this->outbio, "ClientSeggio: Verification return code: %d\n", ret);
    userAgentChiamante->mutex_stdout.unlock();

    if (ret == 0 || ret == 1){
        userAgentChiamante->mutex_stdout.lock();
        BIO_printf(this->outbio, "ClientSeggio: Verification result text: %s\n",
                   X509_verify_cert_error_string(vrfy_ctx->error));
        userAgentChiamante->mutex_stdout.unlock();
    }

    /* The error handling below shows how to get failure details  *
     * from the offending certificate.                            */
    if (ret == 0) {
        //get the offending certificate causing the failure
        error_cert = X509_STORE_CTX_get_current_cert(vrfy_ctx);
        certsubject = X509_NAME_new();
        certsubject = X509_get_subject_name(error_cert);
        userAgentChiamante->mutex_stdout.lock();
        BIO_printf(this->outbio, "ClientSeggio: Verification failed cert:\n");
        X509_NAME_print_ex(this->outbio, certsubject, 0, XN_FLAG_MULTILINE);
        BIO_printf(this->outbio, "\n");
        userAgentChiamante->mutex_stdout.unlock();
    }

    // Free up all structures need for verify certs
    X509_STORE_CTX_free(vrfy_ctx);
    X509_STORE_free(store);
    X509_free(peer_cert);
    //X509_NAME_free(certname);

    //BIO_free_all(certbio);

}

void SSLClient::init_openssl_library() {
    /* https://www.openssl.org/docs/ssl/SSL_library_init.html */
    SSL_library_init();
    /* Cannot fail (always returns success) ??? */

    /* https://www.openssl.org/docs/crypto/ERR_load_crypto_strings.html */
    SSL_load_error_strings();
    /* Cannot fail ??? */

    ERR_load_BIO_strings();
    /* SSL_load_error_strings loads both libssl and libcrypto strings */
    //ERR_load_crypto_strings();
    /* Cannot fail ??? */

    /* OpenSSL_config may or may not be called internally, based on */
    /*  some #defines and internal gyrations. Explicitly call it    */
    /*  *IF* you need something from openssl.cfg, such as a         */
    /*  dynamically configured ENGINE.                              */
    OPENSSL_config(NULL);

}

void SSLClient::createClientContext(){
    const SSL_METHOD *method;
    method = TLSv1_2_client_method();

    /* ---------------------------------------------------------- *
     * Try to create a new SSL context                            *
     * ---------------------------------------------------------- */
    if ((this->ctx = SSL_CTX_new(method)) == NULL){
        userAgentChiamante->mutex_stdout.lock();
        BIO_printf(this->outbio, "ClientSeggio: Unable to create a new SSL context structure.\n");
        userAgentChiamante->mutex_stdout.unlock();
    }
    /* ---------------------------------------------------------- *
     * Disabling SSLv2 and SSLv3 will leave TSLv1 for negotiation    *
     * ---------------------------------------------------------- */
    SSL_CTX_set_options(this->ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
}

void SSLClient::configure_context(char* CertFile, char* KeyFile, char * ChainFile) {
    SSL_CTX_set_ecdh_auto(this->ctx, 1);

    //---il chainfile dovrà essere ricevuto dal peer che si connette? non so se è necessario su entrambi i peer----
    SSL_CTX_load_verify_locations(this->ctx,ChainFile, NULL);
    //SSL_CTX_use_certificate_chain_file(ctx,"/home/giuseppe/myCA/intermediate/certs/ca-chain.cert.pem");
    /*The final step of configuring the context is to specify the certificate and private key to use.*/
    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(this->ctx, CertFile, SSL_FILETYPE_PEM) < 0) {
        //ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(this->ctx, KeyFile, SSL_FILETYPE_PEM) < 0) {
        //ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (!SSL_CTX_check_private_key(this->ctx)) {
        fprintf(stderr, "ClientSeggio: Private key does not match the public certificate\n");
        abort();
    }

}

void SSLClient::cleanup_openssl() {
    EVP_cleanup();
}

void SSLClient::sendString_SSL(SSL* ssl, string s) {
    int length = strlen(s.c_str());
    string length_str = std::to_string(length);
    const char *num_bytes = length_str.c_str();
    SSL_write(ssl, num_bytes, strlen(num_bytes));
    SSL_write(ssl, s.c_str(), length);
}

int SSLClient::receiveString_SSL(SSL* ssl, string &s){

    char dim_string[16];
    memset(dim_string, '\0', sizeof(dim_string));
    int bytes = SSL_read(ssl, dim_string, sizeof(dim_string));
    if (bytes > 0) {
        dim_string[bytes] = 0;
        //lunghezza fileScheda da ricevere
        uint length = atoi(dim_string);
        char buffer[length + 1];
        memset(buffer, '\0', sizeof(buffer));
        bytes = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytes > 0) {
            buffer[bytes] = 0;
            s = buffer;
        }
    }
    return bytes; //bytes read for the string received
}
