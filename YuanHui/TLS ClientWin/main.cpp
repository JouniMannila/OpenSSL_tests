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
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"
TformMain *formMain;

#define IP_ADDR "127.0.0.1"
#define PORTNUM 12350


//***************************************************************************
//
// class CTcpClient
// ----- ----------
//***************************************************************************

CTcpClient::~CTcpClient()
{
    if (m_AddrInfo)
        freeaddrinfo(m_AddrInfo);
    if (m_Socket != INVALID_SOCKET)
        closesocket(m_Socket);
    WSACleanup();
}
//----------------------------------------------------------------------------

bool CTcpClient::Initialize()
{
    if (!startup())
        return false;
    return getAddrInfo();
}
//----------------------------------------------------------------------------

bool CTcpClient::startup()
{
    WSADATA wsaData;

    // Initialize Winsock
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result == 0)
        return true;

    char buf[64];
    sprintf(buf, "WSAStartup failed: %d", result);
    return false;
}
//----------------------------------------------------------------------------

bool CTcpClient::getAddrInfo()
{
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    std::string port = std::to_string(m_PortNo);

    // Resolve the server address and port
    int result = getaddrinfo(IP_ADDR, port.c_str(), &hints, &m_AddrInfo);
    if (result == 0)
        return true;

    char buf[64];
    sprintf(buf, "getaddrinfo failed: %d", result);
    return false;
}
//----------------------------------------------------------------------------

bool CTcpClient::Connect()
{
    // Attempt to connect to the first address returned by
    // the call to getaddrinfo
    struct addrinfo* ptr = m_AddrInfo;

    // Create a SOCKET for connecting to server
    m_Socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (m_Socket == INVALID_SOCKET)
    {
        char buf[64];
        sprintf(buf, "Error at socket(): %d", WSAGetLastError());
        return false;
    }

    // Connect to server.
    int result = connect(m_Socket, ptr->ai_addr, ptr->ai_addrlen);
    if (result == SOCKET_ERROR)
    {
        m_Socket = INVALID_SOCKET;
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class CSSLClient
// ----- ----------
//***************************************************************************

CSSLClient::~CSSLClient()
{
    if (m_SSL)
    {
        SSL_set_shutdown(m_SSL, SSL_RECEIVED_SHUTDOWN | SSL_SENT_SHUTDOWN);
        SSL_shutdown(m_SSL);
        SSL_free(m_SSL);
    }
}
//----------------------------------------------------------------------------

void CSSLClient::Initialize()
{
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
}
//----------------------------------------------------------------------------

bool CSSLClient::CreateCTX()
{
    m_CTX = SSL_CTX_new(TLS_client_method());
    return m_CTX != nullptr;
}
//----------------------------------------------------------------------------

void CSSLClient::CreateSSL(SOCKET socket)
{
    m_SSL = SSL_new(m_CTX);
    SSL_set_fd(m_SSL, socket);
}
//----------------------------------------------------------------------------

bool CSSLClient::Connect()
{
    return SSL_connect(m_SSL) >= 0;
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
    m_TCPClient.SetPortNo(PORTNUM);

    if (!m_TCPClient.Initialize())
    {
        memo->Lines->Add("ERROR: initialize failed");
        return;
    }

    if (!m_TCPClient.Connect())
    {
        memo->Lines->Add("ERROR: connect failed");
        return;
    }

    m_SSLClient.Initialize();

    if (!m_SSLClient.CreateCTX())
    {
        memo->Lines->Add("ERROR: could not initialize the SSL context");
        return;
    }

    m_SSLClient.CreateSSL(m_TCPClient.Socket());

    if (!m_SSLClient.Connect())
    {
        memo->Lines->Add("ERROR: could not complete TLS handshake via SSL");
        return;
    }

    memo->Lines->Add("SSL_Connected");
}
//---------------------------------------------------------------------------

