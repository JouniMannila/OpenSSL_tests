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

#include <string>
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"
TformMain *formMain;

TMemo* g_Memo {};

#define BUFFER_SIZE 1024

const int PORT_NUMBER = 12350;


const std::string CERT_FILE = "y.pem";
const std::string PRIV_KEY = "y.key";


//***************************************************************************
//
// class CTcpServer
// ----- ----------
//***************************************************************************

CTcpServer::~CTcpServer()
{
    if (m_AddrInfo)
        freeaddrinfo(m_AddrInfo);
    if (m_ListenerSocket != INVALID_SOCKET)
        closesocket(m_ListenerSocket);
    if (m_ClientSocket != INVALID_SOCKET)
        closesocket(m_ClientSocket);

    WSACleanup();
}
//----------------------------------------------------------------------------

bool CTcpServer::Connect(int portNo)
{
    if (!startup())
        return false;

    if (!getAddrInfo(portNo))
        return false;

    SOCKET listener;

    m_ListenerSocket = socket(
        m_AddrInfo->ai_family, m_AddrInfo->ai_socktype,
        m_AddrInfo->ai_protocol);
    if (m_ListenerSocket == INVALID_SOCKET)
    {
        char buf[64];
        sprintf(buf, "Error at socket(): %d", WSAGetLastError());
        g_Memo->Lines->Add(buf);
        return false;
    }

    int result = bind(
        m_ListenerSocket, m_AddrInfo->ai_addr, m_AddrInfo->ai_addrlen);
    if (result == SOCKET_ERROR)
    {
        char buf[64];
        sprintf(buf, "bind failed with error: %d", WSAGetLastError());
        g_Memo->Lines->Add(buf);
        return false;
    }

    if (listen(m_ListenerSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        char buf[64];
        sprintf(buf, "listen failed with error: %d", WSAGetLastError());
        g_Memo->Lines->Add(buf);
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------

SOCKET CTcpServer::Accept()
{
    return accept(m_ListenerSocket, nullptr, nullptr);
}
//----------------------------------------------------------------------------

bool CTcpServer::startup()
{
    WSADATA wsaData;

    // Initialize Winsock
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result == 0)
        return true;

    char buf[64];
    sprintf(buf, "WSAStartup failed: %d", result);
    g_Memo->Lines->Add(buf);
    return false;
}
//----------------------------------------------------------------------------

bool CTcpServer::getAddrInfo(int port)
{
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    std::string s = std::to_string(port);

    // Resolve the local address and port to be used by the server
    int result = getaddrinfo(nullptr, s.c_str(), &hints, &m_AddrInfo);
    if (result == 0)
        return true;

    char buf[64];
    sprintf(buf, "getaddrinfo failed: %d", result);
    g_Memo->Lines->Add(buf);
    return false;
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class CSSLServer
// ----- ----------
//***************************************************************************

CSSLServer::~CSSLServer()
{
    if (m_SSL)
    {
        SSL_shutdown(m_SSL);
        SSL_free(m_SSL);
    }
}
//----------------------------------------------------------------------------

bool CSSLServer::CreateCTX()
{
    m_CTX = SSL_CTX_new(TLS_server_method());
    return m_CTX != nullptr;
}
//----------------------------------------------------------------------------

bool CSSLServer::CreateSSL(SOCKET socket)
{
    m_SSL = SSL_new(m_CTX);
    SSL_set_fd(m_SSL, socket);
}
//----------------------------------------------------------------------------

bool CSSLServer::Accept()
{
    return SSL_accept(m_SSL) >= 0;
}
//----------------------------------------------------------------------------

bool CSSLServer::HandleCertificate()
{
    int rc;

    rc = SSL_CTX_use_certificate_file(m_CTX, CERT_FILE.c_str(), SSL_FILETYPE_PEM);
    if ( rc <= 0 )
    {
        g_Memo->Lines->Add("Set SSL_CTX_use_certificate_file() error");
        return false;
    }

    rc = SSL_CTX_use_PrivateKey_file(m_CTX, PRIV_KEY.c_str(), SSL_FILETYPE_PEM);
    if ( rc <= 0 )
    {
        g_Memo->Lines->Add("Set SSL_CTX_use_PrivateKey_file() error");
        return false;
    }

    return true;
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
    g_Memo = memo;
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

    if (!m_SSLServer.CreateCTX())
    {
        memo->Lines->Add("Unable to create SSL context");
        return;
    }

    if (!m_SSLServer.HandleCertificate())
    {
        return;
    }

//    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
//    if (ctx == nullptr )
//    {
//        printf("Unable to create SSL context\n");
//        return;
//    }
//
//    rc = SSL_CTX_use_certificate_file(ctx, CERT_FILE.c_str(), SSL_FILETYPE_PEM);
//    if ( rc <= 0 )
//    {
//        memo->Lines->Add("Set SSL_CTX_use_certificate_file() error");
//        return;
//    }
//
//    rc = SSL_CTX_use_PrivateKey_file(ctx, PRIV_KEY.c_str(), SSL_FILETYPE_PEM);
//    if ( rc <= 0 )
//    {
//        memo->Lines->Add("Set SSL_CTX_use_PrivateKey_file() error");
//        return;
//    }

//    CTcpServer server;

    if (!m_TCPServer.Connect(PORT_NUMBER))
    {
        return;
    }

//    printf("SSL/TLS Echo Server started, port number : (%d)\n", port_num);
    while(1)
    {
        memo->Lines->Add("Listening...");

        SOCKET client = m_TCPServer.Accept();
        if (client == INVALID_SOCKET)
        {
            memo->Lines->Add("Unable to accept");
            return;
        }

        memo->Lines->Add("Connected...");

        m_SSLServer.CreateSSL(client);

        if (!m_SSLServer.Accept())
        {
            memo->Lines->Add("Unable to accept SSL handshake");
            return;
        }

//        auto e =  SSL_accept(ssl);
//        if (e <= 0)
//        {
//            memo->Lines->Add("Unable to accept SSL handshake");
//            return;
//        }

//        for ( ;; )
//        {
//            memset(msg_buf, '\0', BUFFER_SIZE);
//
//            // ssize_t n_recvd = recv(fd, msg_buf, BUFFER_SIZE, 0);
//            int n_recvd = SSL_read(ssl, msg_buf, BUFFER_SIZE);
//            if (n_recvd <= 0)
//                break;
//
//            // ssize_t n_send = send(fd, msg_buf, n_recvd, 0);
//            int n_send = SSL_write(ssl, msg_buf, n_recvd);
//            if ( n_send <= 0 )
//                break;
//
////            memo->Lines->Add("Unable to accept SSL handshake");
//            printf("Recvd Message  (%d - %d) : %s \n", (int)n_recvd, (int)n_send, msg_buf);
//        }

//        SSL_shutdown(ssl);
//        SSL_free(ssl);
//        close(client);
    }

//    SSL_CTX_free(ctx);
}
//---------------------------------------------------------------------------

