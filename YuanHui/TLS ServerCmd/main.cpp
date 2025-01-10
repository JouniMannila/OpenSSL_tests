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
#include <tchar.h>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
//----------------------------------------------------------------------------

const std::string CERT_FILE = "y.pem";
const std::string PRIV_KEY = "y.key";

const int PORT_NUMBER = 12350;


//***************************************************************************
//
// class CTcpServer
// ----- ----------
//***************************************************************************

/*!
 */

class CTcpServer {
  public:
    ~CTcpServer();

    bool Connect(int portNo);
    bool Accept();

    SOCKET ListenerSocket() const
        { return m_ListenerSocket; }

    SOCKET ClientSocket() const
        { return m_ClientSocket; }

  private:
    struct addrinfo* m_AddrInfo = nullptr;

    SOCKET m_ListenerSocket {};
    SOCKET m_ClientSocket {};

    bool startup();
    bool getAddrInfo(int port);
};


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

    m_ListenerSocket = socket(
        m_AddrInfo->ai_family, m_AddrInfo->ai_socktype,
        m_AddrInfo->ai_protocol);
    if (m_ListenerSocket == INVALID_SOCKET)
    {
        char buf[64];
        sprintf(buf, "Error at socket(): %d", WSAGetLastError());
        return false;
    }

    int result = bind(
        m_ListenerSocket, m_AddrInfo->ai_addr, m_AddrInfo->ai_addrlen);
    if (result == SOCKET_ERROR)
    {
        char buf[64];
        sprintf(buf, "bind failed with error: %d", WSAGetLastError());
        return false;
    }

    if (listen(m_ListenerSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        char buf[64];
        sprintf(buf, "listen failed with error: %d", WSAGetLastError());
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------

bool CTcpServer::Accept()
{
    m_ClientSocket = accept(m_ListenerSocket, nullptr, nullptr);
    return m_ClientSocket != INVALID_SOCKET;
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
    return false;
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class CSSLServer
// ----- ----------
//***************************************************************************

/*!
 */

class CSSLServer {
  public:
    ~CSSLServer();

    bool CreateCTX();

    bool CreateSSL(SOCKET clientSocket);

    bool HandleCertificate();

    bool Accept();

  private:
    SSL_CTX* m_CTX {};
    SSL* m_SSL {};
};


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

bool CSSLServer::CreateSSL(SOCKET clientSocket)
{
    m_SSL = SSL_new(m_CTX);
    SSL_set_fd(m_SSL, clientSocket);
    return true;
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
        return false;
    }

    rc = SSL_CTX_use_PrivateKey_file(m_CTX, PRIV_KEY.c_str(), SSL_FILETYPE_PEM);
    if ( rc <= 0 )
    {
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------

int _tmain(int argc, _TCHAR* argv[])
{
    CTcpServer tcpServer {};
    CSSLServer sslServer {};

    if (!sslServer.CreateCTX())
    {
        std::cout << "Unable to create SSL context\n";
        return 0;
    }

    if (!sslServer.HandleCertificate())
    {
        return 0;
    }

    if (!tcpServer.Connect(PORT_NUMBER))
    {
        return 0;
    }

    while(1)
    {
        std::cout << "Listening...\n";

        if (!tcpServer.Accept())
        {
            std::cout << "Unable to accept\n";
            return 0;
        }

        std::cout << "Connected...\n";

        sslServer.CreateSSL(tcpServer.ClientSocket());

        if (!sslServer.Accept())
        {
            std::cout << "Unable to accept SSL handshake\n";
            return 0;
        }

        std::cout << "Accepted...\n";
    }

    return 0;
}
