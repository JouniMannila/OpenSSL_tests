//----------------------------------------------------------------------------
//
// Module: OpenSSL_Server
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#pragma hdrstop

#include "OpenSSL_Server.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace ztls {

#define SHOWFUNC(class)  std::cout << class << "::" << __FUNC__ << std::endl;
#define SHOW(text)  std::cout << text << std::endl;

//***************************************************************************
//
// class CTcpServer
// ----- ----------
//***************************************************************************

CTcpServer::~CTcpServer()
{
    Shutdown();
}
//----------------------------------------------------------------------------

bool CTcpServer::Initialize()
{
    SHOWFUNC("CTcpServer")

    WSADATA wsaData;
    SHOW("  - WSAStartup")
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return SetLastError("WSAStartup failed.");

    m_WSAStartupCalled = true;
    return true;
}
//----------------------------------------------------------------------------

bool CTcpServer::Connect()
{
    SHOWFUNC("CTcpServer")

    if (m_PortNo == 0)
        return SetLastError("ERROR: Port == 0.");

    // luodaan socket
    SHOW("  - socket")
    m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_ServerSocket < 0)
        return SetLastError("Create socket failed.");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_PortNo);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    SHOW("  - bind")
    if (bind(m_ServerSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        return SetLastError("Unable to bind.");

    SHOW("  - listen")
    if (listen(m_ServerSocket, 1) < 0)
        return SetLastError("Unable to listen.");

    return true;
}
//----------------------------------------------------------------------------

bool CTcpServer::Accept()
{
    SHOWFUNC("CTcpServer")

    struct sockaddr_in addr;
    int len = sizeof(addr);

    SHOW("  - accept")
    m_ClientSocket = accept(m_ServerSocket, (struct sockaddr*)&addr, &len);
    if (m_ClientSocket < 0)
        return SetLastError("Unable to accept.");

    return true;
}
//----------------------------------------------------------------------------

bool CTcpServer::Shutdown()
{
    if (m_ClientSocket != INVALID_SOCKET)
    {
//        shutdown(m_ClientSocket, SHUT_RDWR);
        SHOW("  - closesocket (client)")
        closesocket(m_ClientSocket);
        m_ClientSocket = INVALID_SOCKET;
    }
    if (m_ServerSocket != INVALID_SOCKET)
    {
        SHOW("  - closesocket (server)")
        closesocket(m_ServerSocket);
        m_ServerSocket = INVALID_SOCKET;
    }

    if (m_WSAStartupCalled)
    {
        SHOW("  - WSACleanup")
        WSACleanup();
        m_WSAStartupCalled = false;
    }

    return true;
}
//----------------------------------------------------------------------------

bool CTcpServer::SetLastError(std::string_view caption, std::string_view msg)
{
    m_LastError.Caption = caption;
    m_LastError.Message = msg;
    return false;
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class COpenSSL_Server
// ----- ---------------
//***************************************************************************

COpenSSL_Server::~COpenSSL_Server()
{
    SHOWFUNC("COpenSSL_Server")

//    if (m_SSL)
//    {
//        SHOW("  - SSL_shutdown")
//        SSL_shutdown(m_SSL);
//
//        SHOW("  - SSL_free")
//        SSL_free(m_SSL);
//        m_SSL = nullptr;
//
//        SHOW("  - SSL_CTX_free")
//        SSL_CTX_free(m_CTX);
//        m_CTX = nullptr;
//    }
//
//    EVP_cleanup();
}
//----------------------------------------------------------------------------

void COpenSSL_Server::Initialize()
{
    SHOWFUNC("COpenSSL_Server")

    SHOW("  - SSL_load_error_strings")
    SSL_load_error_strings();
    SHOW("  - OpenSSL_add_ssl_algorithms")
    OpenSSL_add_ssl_algorithms();
}
//----------------------------------------------------------------------------

bool COpenSSL_Server::CreateContext()
{
    SHOWFUNC("COpenSSL_Server")

    SHOW("  - TLS_server_method")
    m_CTX = SSL_CTX_new(TLS_server_method());
    if (!m_CTX)
        return SetLastError(getOpenSSLError());

    SHOW("  - SSL_CTX_set_min_proto_version")
    if (SSL_CTX_set_min_proto_version(m_CTX, TLS1_VERSION) == 0)
        return SetLastError(getOpenSSLError());

    SHOW("  - SSL_CTX_set_max_proto_version")
    if (SSL_CTX_set_max_proto_version(m_CTX, TLS1_2_VERSION) == 0)
        return SetLastError(getOpenSSLError());

    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Server::ConfigureContext()
{
    SHOWFUNC("COpenSSL_Server")

    SHOW("  - SSL_CTX_set_ecdh_auto")
    SSL_CTX_set_ecdh_auto(m_CTX, 1);

    // Set the key and cert
    SHOW("  - SSL_CTX_use_certificate_file")
    if (SSL_CTX_use_certificate_file(
            m_CTX, m_Certificate.c_str(), SSL_FILETYPE_PEM) <= 0)
        return SetLastError(getOpenSSLError());

    SHOW("  - SSL_CTX_use_PrivateKey_file")
    if (SSL_CTX_use_PrivateKey_file(
            m_CTX, m_PrivateKey.c_str(), SSL_FILETYPE_PEM) <= 0)
        return SetLastError(getOpenSSLError());

    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Server::CreateSSL(int fd)
{
    SHOWFUNC("COpenSSL_Server")

    m_SSL = SSL_new(m_CTX);
    SHOW("  - SSL_set_fd")
    if (SSL_set_fd(m_SSL, fd) == 0)
        return SetLastError("SSL_set_fd failed.");
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Server::Accept()
{
    SHOWFUNC("COpenSSL_Server")

    SHOW("  - SSL_accept")
    if (SSL_accept(m_SSL) <= 0)
        return SetLastError("SSL_accept failed.");
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Server::Shutdown()
{
    SHOWFUNC("COpenSSL_Server")

    SHOW("  - SSL_shutdown")
    int status = SSL_shutdown(m_SSL);

    if (status == 0)
        status = SSL_shutdown(m_SSL);
    else if (status <= 0)
    {
        std::string s = CSSL_GetError::Reason(m_SSL, status);
        if (!s.empty())
            return SetLastError("SSL_shutdown failed.", s);
        else
            return SetLastError("SSL_shutdown failed.");
    }

    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Server::Free()
{
    if (m_SSL)
    {
        SHOW("  - SSL_free")
        SSL_free(m_SSL);
        m_SSL = nullptr;
    }

    if (m_CTX)
    {
        SHOW("  - SSL_CTX_free")
        SSL_CTX_free(m_CTX);
        m_CTX = nullptr;
    }

    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Server::Write(std::string_view msg)
{
    SHOWFUNC("COpenSSL_Server")

    SHOW("  - SSL_write")
    int bytesWritten = SSL_write(m_SSL, msg.data(), msg.size());
    if (bytesWritten <= 0)
        return SetLastError("SSL_write failed.");
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Server::Read(std::string& msg)
{
    SHOWFUNC("COpenSSL_Server")

    SHOW("  - SSL_read")

    char buffer[1014];
    int bytes = SSL_read(m_SSL, buffer, sizeof(buffer));
    if (bytes <= 0)
        return false;
    msg = std::string(buffer);
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Server::SetLastError(
    std::string_view caption, std::string_view msg)
{
    m_LastError.Caption = caption;
    m_LastError.Message = msg;
    return false;
}
//----------------------------------------------------------------------------

}

