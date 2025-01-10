//----------------------------------------------------------------------------
//
// Module: main
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
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

#include <string>
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"
TformMain *formMain;

#define BUFFER_SIZE 1024

const std::string CERT_FILE = "y.pem";
const std::string PRIV_KEY = "y.key";


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

void __fastcall TformMain::FormShow(TObject *Sender)
{
//
}
//---------------------------------------------------------------------------

void __fastcall TformMain::Button1Click(TObject *Sender)
{
    char msg_buf[BUFFER_SIZE];
    int rc;

//    if ( argc != 4 ) {
//        printf("usage: %s <port> <certificate> <private_key>\n", argv[0]);
//        printf("Example : %s 1234 my_certificate.pem my_private_key.pem\n", argv[0]);
//        return 1;
//    }

//    int port_num = std::stoi(argv[1]);
    int port_num = 12340;

    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if (ctx == nullptr ) {
        printf("Unable to create SSL context\n");
        return;
    }

    rc = SSL_CTX_use_certificate_file(ctx, CERT_FILE.c_str(), SSL_FILETYPE_PEM);
    if ( rc <= 0 ) {
        memo->Lines->Add("Set SSL_CTX_use_certificate_file() error");
        return;
    }

    rc = SSL_CTX_use_PrivateKey_file(ctx, PRIV_KEY.c_str(), SSL_FILETYPE_PEM);
    if ( rc <= 0 ) {
        memo->Lines->Add("Set SSL_CTX_use_PrivateKey_file() error");
        return;
    }

    struct sockaddr_in    sin;
    sin.sin_family        = AF_INET;       // <-- IPv4 internet protcol
    sin.sin_addr.s_addr   = INADDR_ANY;    // <-- Accept any incoming messages (0)
    sin.sin_port          = htons(port_num);

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listener == INVALID_SOCKET)
    {
        char buf[128];
        sprintf(buf, "Error at socket(): %ld\n", WSAGetLastError());
        memo->Lines->Add(buf);
        return;
    }

    rc = bind(listener, (struct sockaddr*) &sin, sizeof(sin));
    if ( rc < 0 )
    {
        char buf[128];
        sprintf(buf, "bind socket error, port number : <%d>, listener : %d\n", port_num, listener);
        memo->Lines->Add(buf);
        return;
    }

    rc = listen(listener, 16);
    if ( rc < 0 )
    {
        char buf[128];
        sprintf(buf, "listen for connection error, listener : <%d>\n", listener);
        memo->Lines->Add(buf);
        return;
    }

    printf("SSL/TLS Echo Server started, port number : (%d)\n", port_num);
    while(1)
    {
        struct sockaddr_in addr;
        unsigned int len = sizeof(addr);

        int fd = accept(listener, nullptr, nullptr);
//        int fd =  accept(listener, (struct sockaddr*)&addr, &len);
        if (fd < 0)
        {
            perror("Unable to accept");
            exit(EXIT_FAILURE);
        }

        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, fd);
        if ( SSL_accept(ssl) <= 0 )
        {
            printf("Unable to accept SSL handshake\n");
        }

        for ( ;; )
        {
            memset(msg_buf, '\0', BUFFER_SIZE);
            // ssize_t n_recvd = recv(fd, msg_buf, BUFFER_SIZE, 0);
            int n_recvd = SSL_read(ssl, msg_buf, BUFFER_SIZE);
            if ( n_recvd <= 0 )
                break;
            // ssize_t n_send = send(fd, msg_buf, n_recvd, 0);
            int n_send = SSL_write(ssl, msg_buf, n_recvd);
            if ( n_send <= 0 )
                break;
            printf("Recvd Message  (%d - %d) : %s \n", (int)n_recvd, (int)n_send, msg_buf);
        }

        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(fd);
    }

    close(listener);
    SSL_CTX_free(ctx);
}
//---------------------------------------------------------------------------

