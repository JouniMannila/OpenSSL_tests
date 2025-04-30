//----------------------------------------------------------------------------
//
// Module: OpenSSL_Client
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#pragma hdrstop

#include "OpenSSL_Client.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <winsock.h>
//---------------------------------------------------------------------------

#pragma package(smart_init)

CFuncPtr g_MemoWriter {};

#ifdef CONSOLE

  #define SHOWFUNC(class)  std::cout << class << "::" << __FUNC__ << std::endl;
  #define SHOW(text)  std::cout << text << std::endl;

#else

  #define SHOWFUNC(class) \
    if (g_MemoWriter.Func) \
      g_MemoWriter.Func( \
        g_MemoWriter.This, std::string(class) + "::" + std::string(__FUNC__));

  #define SHOW(text) \
    if (g_MemoWriter.Func) \
      g_MemoWriter.Func(g_MemoWriter.This, text);

#endif


namespace ztls {

//***************************************************************************
//
// class CSSLGuard
// ----- ---------
//***************************************************************************

/*!
 */

class CSSLGuard {
  public:
    CSSLGuard(void* p) : p_(p) {}
    ~CSSLGuard()
        { if (p_) OPENSSL_free(p_); }
  private:
    void* p_ {};
};


//***************************************************************************
//
// class CTcpClient
// ----- ----------
//***************************************************************************

CTcpClient::~CTcpClient()
{
    Disconnect();
}
//----------------------------------------------------------------------------

CTlsResult CTcpClient::Connect()
{
    if (CTlsResult r = Initialize(); !r)
        return r;

    if (CTlsResult r = DoConnect(); !r)
        return r;

    return CTlsResult();
}
//----------------------------------------------------------------------------

CTlsResult CTcpClient::Initialize()
{
    SHOWFUNC("CTcpClient")

    WSADATA wsaData;
    SHOW("  - WSAStartup")
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return SetLastError(tlse_TCP_WSAStartup);

    m_WSAStartupCalled = true;
    return CTlsResult();
}
//----------------------------------------------------------------------------

CTlsResult CTcpClient::DoConnect()
{
    SHOWFUNC("CTcpClient")

    if (m_PortNo == 0)
        return SetLastError(tlse_TCP_Port);

    if (m_Address.empty())
        return SetLastError(tlse_TCP_Address);

    // luodaan socket
    SHOW("  - socket")
    m_Socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_Socket == INVALID_SOCKET)
        return SetLastError(tlse_TCP_socket);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_PortNo);
    addr.sin_addr.s_addr = inet_addr(m_Address.c_str());

    // yritetään kytkeytyä
    SHOW("  - connect")
    if (connect(m_Socket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        return SetLastError(tlse_TCP_connect);

    return CTlsResult();
}
//----------------------------------------------------------------------------

void CTcpClient::Disconnect()
{
    if (m_Socket != INVALID_SOCKET)
    {
        SHOW("  - closesocket")
        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;
    }

    if (m_WSAStartupCalled)
    {
        SHOW("  - WSACleanup")
        WSACleanup();
        m_WSAStartupCalled = false;
    }
}
//----------------------------------------------------------------------------

void CTcpClient::SetReadTimeout(DWORD timeout)
{
    setsockopt(
        m_Socket, SOL_SOCKET, SO_RCVTIMEO,
        reinterpret_cast<const char*>(&timeout), sizeof(timeout));
}
//----------------------------------------------------------------------------

CTlsResult CTcpClient::SetLastError(int errCode, const std::string& msg)
{
    return CTlsResult(errCode, msg);
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class COpenSSL_Client
// ----- ---------------
//***************************************************************************

COpenSSL_Client::~COpenSSL_Client()
{
    SHOWFUNC("COpenSSL_Client")

    Shutdown();
    Free();

    EVP_cleanup();
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::MakeConnection(CTcpClient& tcpClient)
{
    Initialize();

    if (CTlsResult r = CreateContext(); !r)
        return r;

    if (CTlsResult r = SetVersions(); !r)
        return r;

    if (CTlsResult r = CreateSSL(tcpClient.Socket()); !r)
        return r;

    if (CTlsResult r = Connect(); !r)
        return r;

    if (CTlsResult r = LoadVerifyLocations(); !r)
        return r;

    if (CTlsResult r = VerifyCertification(); !r)
        return r;

    return CTlsResult();
}
//----------------------------------------------------------------------------

void COpenSSL_Client::Initialize()
{
    SHOWFUNC("COpenSSL_Client")

    SHOW("  - SSL_load_error_strings")
    SSL_load_error_strings();
    SHOW("  - OpenSSL_add_ssl_algorithms")
    OpenSSL_add_ssl_algorithms();
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::CreateContext()
{
    SHOWFUNC("COpenSSL_Client")

    SHOW("  - TLS_client_method")
    m_CTX = SSL_CTX_new(TLS_client_method());
    if (!m_CTX)
        return SetLastError(tlse_SSL_CTX_new);

    return CTlsResult();
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::SetVersions()
{
    SHOWFUNC("COpenSSL_Client")

    SHOW("  - SSL_CTX_set_min_proto_version")
    if (SSL_CTX_set_min_proto_version(m_CTX, m_TLS_MinVersion) == 0)
        return SetLastError(tlse_SSL_CTX_set_min_proto_version);

    SHOW("  - SSL_CTX_set_max_proto_version")
    if (SSL_CTX_set_max_proto_version(m_CTX, m_TLS_MaxVersion) == 0)
        return SetLastError(tlse_SSL_CTX_set_max_proto_version);

    return CTlsResult();
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::CreateSSL(int fd)
{
    SHOWFUNC("COpenSSL_Client")

    SHOW("  - SSL_new")
    m_SSL = SSL_new(m_CTX);
    if (!m_SSL)
        return SetLastError(tlse_SSL_new);
    SHOW("  - SSL_set_fd")
    if (SSL_set_fd(m_SSL, fd) == 0)
        return SetLastError(tlse_SSL_set_fd);
    return CTlsResult();
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::DisplayCerts()
{
    SHOWFUNC("COpenSSL_Client")

    SHOW("  - SSL_get_peer_certificate")
    X509* cert = SSL_get_peer_certificate(m_SSL);
    if (!cert)
        return SetLastError(tlse_SSL_get_peer_certificate);
    CSSLGuard x509Guard(cert);

    SHOW("  - X509_get_subject_name")
    char* subject = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    if (!subject)
        return SetLastError(tlse_X509_get_subject_name);
    CSSLGuard subjectQuard(subject);

    std::cout << "  = X509_subject_name: " << subject << std::endl;

    SHOW("  - X509_get_issuer_name")
    char* issuer = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    if (!issuer)
        return SetLastError(tlse_X509_get_issuer_name);
    CSSLGuard issuerQuard(issuer);

    std::cout << "  = X509_issuer_name: " << issuer << std::endl;

    return CTlsResult();
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::Connect()
{
    SHOWFUNC("COpenSSL_Client")

    SHOW("  - SSL_connect")
    int ret = SSL_connect(m_SSL);
    if (ret > 0)
        return CTlsResult();

    std::string s1 = CSSL_GetError::Reason(m_SSL, ret);
    s1 += "\n";
    s1 += getOpenSSLError();
    return SetLastError(tlse_SSL_connect, s1);
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::Shutdown()
{
    SHOWFUNC("COpenSSL_Client")

    if (m_SSL)
    {
        SHOW("  - SSL_shutdown")
        int status = SSL_shutdown(m_SSL);

        if (status <= 0)
        {
            std::string s = CSSL_GetError::Reason(m_SSL, status);
            if (!s.empty())
                return SetLastError(tlse_SSL_shutdown, s);
            else
                return SetLastError(tlse_SSL_shutdown);
        }
    }

    return CTlsResult();
}
//----------------------------------------------------------------------------

void COpenSSL_Client::Free()
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
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::LoadVerifyLocations()
{
    SHOWFUNC("COpenSSL_Client")

    SHOW("  - SSL_CTX_load_verify_locations")
    if (int r = SSL_CTX_load_verify_locations(
            m_CTX, m_Certificate.c_str(), nullptr); r != 1)
        return SetLastError(tlse_SSL_CTX_load_verify_locations);
    return CTlsResult();
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::VerifyCertification()
{
    SHOWFUNC("COpenSSL_Client")

    SHOW("  - SSL_CTX_set_verify")
    SSL_CTX_set_verify(m_CTX, SSL_VERIFY_PEER, nullptr);

    SHOW("  - SSL_get_verify_result")
    if (int r = SSL_get_verify_result(m_SSL); r != X509_V_OK)
    {
        return SetLastError(
            tlse_SSL_get_verify_result,
            X509_verify_cert_error_string(SSL_get_verify_result(m_SSL)));
    }
    return CTlsResult();
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::Write(std::string_view message)
{
    SHOWFUNC("COpenSSL_Client")

//    if (!is_connection_active(m_SSL))
//    {
//        return SetLastError("Connection is not active", "");
//        return false;
//    }

    SHOW("  - SSL_write")
    int ret = SSL_write(m_SSL, message.data(), message.size());
    if (ret > 0)
        return CTlsResult();

    std::string s = CSSL_GetError::Reason(m_SSL, ret);
    if (!s.empty())
        return SetLastError(tlse_SSL_write, s);
    else
        return SetLastError(tlse_SSL_write);
}
//----------------------------------------------------------------------------

int COpenSSL_Client::Read(std::string& message)
{
    SHOWFUNC("COpenSSL_Client")

    char buffer[1024];
    SHOW("  - SSL_read")
    int bytes = SSL_read(m_SSL, buffer, sizeof(buffer));
    if (bytes <= 0)
        return bytes;

    buffer[bytes] = 0;
    message = buffer;
    return bytes;
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::SetLastError(
    int errCode, const std::string& msg)
{
    return CTlsResult(errCode, msg);
}
//----------------------------------------------------------------------------

}

