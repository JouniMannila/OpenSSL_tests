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
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace ztls {

#define SHOWFUNC(class)  std::cout << class << "::" << __FUNC__ << std::endl;
#define SHOW(text)  std::cout << text << std::endl;

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
    SHOWFUNC("CTcpClient")

    if (m_ServerSocket)
    {
        SHOW("  - closesocket")
        closesocket(m_ServerSocket);
    }
    SHOW("  - WSACleanup")
    WSACleanup();
}
//----------------------------------------------------------------------------

bool CTcpClient::Connect()
{
    if (!Initialize())
        return false;

    if (!DoConnect())
        return false;

    return true;
}
//----------------------------------------------------------------------------

bool CTcpClient::Initialize()
{
    SHOWFUNC("CTcpClient")

    WSADATA wsaData;
    SHOW("  - WSAStartup")
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return SetLastError("WSAStartup failed.");
    return true;
}
//----------------------------------------------------------------------------

bool CTcpClient::DoConnect()
{
    SHOWFUNC("CTcpClient")

    if (m_PortNo == 0)
        return SetLastError("Port == 0.");

    if (m_Address.empty())
        return SetLastError("Address == \"\".");

    // luodaan socket
    SHOW("  - socket")
    m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_ServerSocket < 0)
        return SetLastError("Create socket failed.");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_PortNo);
    addr.sin_addr.s_addr = inet_addr(m_Address.c_str());

    // yritetään kytkeytyä
    SHOW("  - connect")
    if (connect(m_ServerSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        return SetLastError("Unable to connect.");

    return true;
}
//----------------------------------------------------------------------------

bool CTcpClient::SetLastError(
    const std::string& caption, const std::string& msg)
{
    m_LastError.Caption = caption;
    m_LastError.Message = msg;
    return false;
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class CUdpClient
// ----- ----------
//***************************************************************************

CUdpClient::~CUdpClient()
{
    SHOWFUNC("CUdpClient")

    if (m_UdpSocket)
    {
        SHOW("  - closesocket")
        closesocket(m_UdpSocket);
    }
    SHOW("  - WSACleanup")
    WSACleanup();
}
//----------------------------------------------------------------------------

bool CUdpClient::Connect()
{
    if (!Initialize())
        return false;

    if (!DoConnect())
        return false;

    return true;
}
//----------------------------------------------------------------------------

bool CUdpClient::Initialize()
{
    SHOWFUNC("CUdpClient")

    WSADATA wsaData;
    SHOW("  - WSAStartup")
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return SetLastError("WSAStartup failed.");
    return true;
}
//----------------------------------------------------------------------------

bool CUdpClient::DoConnect()
{
    SHOWFUNC("CUdpClient")

    if (m_PortNo == 0)
        return SetLastError("Port == 0.");

    if (m_Address.empty())
        return SetLastError("Address == \"\".");

    // luodaan socket
    SHOW("  - socket")
    m_UdpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_UdpSocket < 0)
        return SetLastError("Create socket failed.");

    m_ServerAddr.sin_family = AF_INET;
    m_ServerAddr.sin_addr.s_addr = inet_addr(m_Address.c_str());
    m_ServerAddr.sin_port = htons(m_PortNo);

//    // yritetään kytkeytyä
//    SHOW("  - connect")
//    if (connect(m_ServerSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
//        return SetLastError("Unable to connect.");

    return true;
}
//----------------------------------------------------------------------------

bool CUdpClient::SetLastError(
    const std::string& caption, const std::string& msg)
{
    m_LastError.Caption = caption;
    m_LastError.Message = msg;
    return false;
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

    if (m_SSL)
    {
        SHOW("  - SSL_shutdown")
        SSL_shutdown(m_SSL);
//        SHOW("  - BIO_free")
//        BIO_free(m_BIO);
        SHOW("  - SSL_free")
        SSL_free(m_SSL);
        SHOW("  - SSL_CTX_free")
        SSL_CTX_free(m_CTX);
    }
    EVP_cleanup();
}
//----------------------------------------------------------------------------

//bool COpenSSL_Client::MakeConnection(CTcpClient& tcpClient)
//{
//    Initialize();
//
//    if (!CreateContext())
//        return false;
//
//    if (!SetVersions())
//        return false;
//
//    if (!CreateSSL(tcpClient.ServerSocket()))
//        return false;
//
//    if (!Connect())
//        return false;
//
//    if (!LoadVerifyLocations())
//        return false;
//
//    if (!VerifyCertification())
//        return false;
//
//    return true;
//}
//----------------------------------------------------------------------------

void COpenSSL_Client::Initialize()
{
    SHOWFUNC("COpenSSL_Client")

    SSL_library_init();

    SHOW("  - SSL_load_error_strings")
    SSL_load_error_strings();
    SHOW("  - OpenSSL_add_ssl_algorithms")
    OpenSSL_add_ssl_algorithms();
}
//----------------------------------------------------------------------------

bool COpenSSL_Client::CreateContext()
{
    SHOWFUNC("COpenSSL_Client")

    SHOW("  - DTLS_client_method")
    m_CTX = SSL_CTX_new(DTLS_client_method());
//    m_CTX = SSL_CTX_new(TLS_client_method());
    if (!m_CTX)
        return SetLastError("Unable to create SSL context.");

    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Client::SetVersions()
{
    SHOWFUNC("COpenSSL_Client")

    SHOW("  - SSL_CTX_set_min_proto_version")
    if (SSL_CTX_set_min_proto_version(m_CTX, m_TLS_MinVersion) == 0)
        return SetLastError("Unable to set min proto version.");

    SHOW("  - SSL_CTX_set_max_proto_version")
    if (SSL_CTX_set_max_proto_version(m_CTX, m_TLS_MaxVersion) == 0)
        return SetLastError("Unable to set max proto version.");

    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Client::CreateSSL(
    SOCKET udpSocket, struct sockaddr_in serverAddr)
{
    SHOWFUNC("COpenSSL_Client")

    SHOW("  - SSL_new")
    m_SSL = SSL_new(m_CTX);
    if (!m_SSL)
        return SetLastError("SSL_new failed.");

//    SHOW("  - SSL_set_fd")
//    if (SSL_set_fd(m_SSL, fd) == 0)
//        return SetLastError("SSL_set_fd failed.");

    m_BIO = BIO_new_dgram(udpSocket, BIO_NOCLOSE);
    if (!m_BIO)
        return SetLastError("BIO_new_dgram failed.");

    if (BIO_ctrl(m_BIO, BIO_CTRL_DGRAM_SET_CONNECTED, 0, &serverAddr) <= 0)
        return SetLastError("BIO_ctrl failed.");

    SSL_set_bio(m_SSL, m_BIO, m_BIO);

    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Client::DisplayCerts()
{
    SHOWFUNC("COpenSSL_Client")

    SHOW("  - SSL_get_peer_certificate")
    X509* cert = SSL_get_peer_certificate(m_SSL);
    if (!cert)
        return SetLastError("SSL_set_fd SSL_get_peer_certificate failed.");
    CSSLGuard x509Guard(cert);

    SHOW("  - X509_get_subject_name")
    char* subject = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    if (!subject)
        return SetLastError("X509_get_subject_name failed.");
    CSSLGuard subjectQuard(subject);

    std::cout << "  = X509_subject_name: " << subject << std::endl;

    SHOW("  - X509_get_issuer_name")
    char* issuer = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    if (!issuer)
        return SetLastError("X509_get_issuer_name failed.");
    CSSLGuard issuerQuard(issuer);

    std::cout << "  = X509_issuer_name: " << issuer << std::endl;

    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Client::Connect()
{
    SHOWFUNC("COpenSSL_Client")

    SHOW("  - SSL_connect")
    int ret = SSL_connect(m_SSL);
    if (ret > 0)
        return true;

    std::string s1 = CSSL_GetError::Reason(m_SSL, ret);
    s1 += "\n";
    s1 += getOpenSSLError();
    return SetLastError("SSL_connect failed.", s1);
}
//----------------------------------------------------------------------------

bool COpenSSL_Client::LoadVerifyLocations()
{
    SHOWFUNC("COpenSSL_Client")

    SHOW("  - SSL_CTX_load_verify_locations")
    if (int r = SSL_CTX_load_verify_locations(
            m_CTX, m_Certificate.c_str(), nullptr); r != 1)
        return SetLastError("SSL_CTX_load_verify_locations failed.");
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Client::VerifyCertification()
{
    SHOWFUNC("COpenSSL_Client")

    SHOW("  - SSL_CTX_set_verify")
    SSL_CTX_set_verify(m_CTX, SSL_VERIFY_PEER, nullptr);

    SHOW("  - SSL_get_verify_result")
    if (int r = SSL_get_verify_result(m_SSL); r != X509_V_OK)
    {
        return SetLastError(
            "SSL_get_verify_result failed.",
            X509_verify_cert_error_string(SSL_get_verify_result(m_SSL)));
    }
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Client::Write(std::string_view message)
{
    SHOWFUNC("COpenSSL_Client")

    SHOW("  - SSL_write")
    int ret = SSL_write(m_SSL, message.data(), message.size());
    if (ret > 0)
        return true;

    std::string s = CSSL_GetError::Reason(m_SSL, ret);
    if (!s.empty())
        return SetLastError("SSL_write failed.", s);
    else
        return SetLastError("SSL_write failed.");
}
//----------------------------------------------------------------------------

bool COpenSSL_Client::Read(std::string& message)
{
    SHOWFUNC("COpenSSL_Client")

    char buffer[1024];
    SHOW("  - SSL_read")
    int bytes = SSL_read(m_SSL, buffer, sizeof(buffer));
    if (bytes <= 0)
        return false;

    buffer[bytes] = 0;
    message = buffer;
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Client::SetLastError(
    const std::string& caption, const std::string& msg)
{
    m_LastError.Caption = caption;
    m_LastError.Message = msg;
    return false;
}
//----------------------------------------------------------------------------

}

