//----------------------------------------------------------------------------
//
// Module: main
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

//----------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "main.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
//#include <sys/socket.h>
//#include <arpa/inet.h>

#include <Winsock2.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
//----------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"
TformMain *formMain;
//---------------------------------------------------------------------------

//***************************************************************************
//
// class COpenSSL_TLS_Server
// ----- -------------------
//***************************************************************************

/*!
 */

class COpenSSL_TLS_Server {
  public:
    COpenSSL_TLS_Server() {}

    SSL_CTX* CreateContext();
    void ConfigureContext(SSL_CTX *ctx);

    int CreateSocket(int port);

  private:
};


int COpenSSL_TLS_Server::CreateSocket(int port)
{
    int s;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    if (listen(s, 1) < 0) {
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }

    return s;}
//----------------------------------------------------------------------------

SSL_CTX* COpenSSL_TLS_Server::CreateContext()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}
//----------------------------------------------------------------------------

void COpenSSL_TLS_Server::ConfigureContext(SSL_CTX *ctx)
{
    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class TformMain
// ----- ---------
//***************************************************************************

__fastcall TformMain::TformMain(TComponent* Owner)
  : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TformMain::Button1Click(TObject *Sender)
{
    int sock;
    SSL_CTX *ctx;

    /* Ignore broken pipe signals */
//    signal(SIGPIPE, SIG_IGN);

    COpenSSL_TLS_Server server;

    ctx = server.CreateContext();

    int ret = server.CreateSocket(4433);


    /* Handle connections */
    while(1) {
        struct sockaddr_in addr;
        unsigned int len = sizeof(addr);
        SSL *ssl;
        const char reply[] = "test\n";

        int client = accept(sock, (struct sockaddr*)&addr, &len);
        if (client < 0) {
            perror("Unable to accept");
            exit(EXIT_FAILURE);
        }

        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
        } else {
            SSL_write(ssl, reply, strlen(reply));
        }

        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(client);
    }

    close(sock);
    SSL_CTX_free(ctx);
}
//---------------------------------------------------------------------------

