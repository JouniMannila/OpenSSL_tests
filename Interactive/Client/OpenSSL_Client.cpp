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

//CFuncPtr g_MemoWriter {};

#ifdef CONSOLE
  #define SHOWFUNC(class)  std::cout << class << "::" << __FUNC__ << std::endl;
  #define SHOW(text)  std::cout << text << std::endl;
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
    ShowFunc("CTcpClient", __FUNC__);

    WSADATA wsaData;
    Show("  - WSAStartup");
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return SetLastError(tlse_TCP_WSAStartup);

    m_WSAStartupCalled = true;
    return CTlsResult();
}
//----------------------------------------------------------------------------

CTlsResult CTcpClient::DoConnect()
{
    ShowFunc("CTcpClient", __FUNC__);

    if (m_PortNo == 0)
        return SetLastError(tlse_TCP_Port);

    if (m_Address.empty())
        return SetLastError(tlse_TCP_Address);

    // luodaan socket
    Show("  - socket");
    m_Socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_Socket == INVALID_SOCKET)
        return SetLastError(tlse_TCP_socket);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_PortNo);
    addr.sin_addr.s_addr = inet_addr(m_Address.c_str());

    // yritetään kytkeytyä
    Show("  - connect");
    if (connect(m_Socket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        return SetLastError(tlse_TCP_connect);

    return CTlsResult();
}
//----------------------------------------------------------------------------

void CTcpClient::Disconnect()
{
    if (m_Socket != INVALID_SOCKET)
    {
        Show("  - closesocket");
        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;
    }

    if (m_WSAStartupCalled)
    {
        Show("  - WSACleanup");
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

void CTcpClient::ShowFunc(const std::string& class_, const std::string& func)
{
    if (m_DebugStringCb)
        m_DebugStringCb(std::string(class_) + "::" + std::string(func));
}
//----------------------------------------------------------------------------

void CTcpClient::Show(const std::string& text)
{
    if (m_DebugStringCb)
        m_DebugStringCb(text.data());
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class COpenSSL_Client
// ----- ---------------
//***************************************************************************

COpenSSL_Client::~COpenSSL_Client()
{
    ShowFunc("COpenSSL_Client", __FUNC__);

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
    ShowFunc("COpenSSL_Client", __FUNC__);

    Show("  - SSL_load_error_strings");
    SSL_load_error_strings();
    Show("  - OpenSSL_add_ssl_algorithms");
    OpenSSL_add_ssl_algorithms();
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::CreateContext()
{
    ShowFunc("COpenSSL_Client", __FUNC__);

    Show("  - TLS_client_method");
    m_CTX = SSL_CTX_new(TLS_client_method());
    if (!m_CTX)
        return SetLastError(tlse_SSL_CTX_new);

    return CTlsResult();
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::SetVersions()
{
    ShowFunc("COpenSSL_Client", __FUNC__);

    Show("  - SSL_CTX_set_min_proto_version");
    if (SSL_CTX_set_min_proto_version(m_CTX, m_TLS_MinVersion) == 0)
        return SetLastError(tlse_SSL_CTX_set_min_proto_version);

    Show("  - SSL_CTX_set_max_proto_version");
    if (SSL_CTX_set_max_proto_version(m_CTX, m_TLS_MaxVersion) == 0)
        return SetLastError(tlse_SSL_CTX_set_max_proto_version);

    return CTlsResult();
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::CreateSSL(int fd)
{
    ShowFunc("COpenSSL_Client", __FUNC__);

    Show("  - SSL_new");
    m_SSL = SSL_new(m_CTX);
    if (!m_SSL)
        return SetLastError(tlse_SSL_new);
    Show("  - SSL_set_fd");
    if (SSL_set_fd(m_SSL, fd) == 0)
        return SetLastError(tlse_SSL_set_fd);
    return CTlsResult();
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::DisplayCerts()
{
    ShowFunc("COpenSSL_Client", __FUNC__);

    Show("  - SSL_get_peer_certificate");
    X509* cert = SSL_get_peer_certificate(m_SSL);
    if (!cert)
        return SetLastError(tlse_SSL_get_peer_certificate);
    CSSLGuard x509Guard(cert);

    Show("  - X509_get_subject_name");
    char* subject = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    if (!subject)
        return SetLastError(tlse_X509_get_subject_name);
    CSSLGuard subjectQuard(subject);

    std::cout << "  = X509_subject_name: " << subject << std::endl;

    Show("  - X509_get_issuer_name");
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
    ShowFunc("COpenSSL_Client", __FUNC__);

    Show("  - SSL_connect");
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
    ShowFunc("COpenSSL_Client", __FUNC__);

    if (m_SSL)
    {
        Show("  - SSL_shutdown");
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
    ShowFunc("COpenSSL_Client", __FUNC__);

    if (m_SSL)
    {
        Show("  - SSL_free");
        SSL_free(m_SSL);
        m_SSL = nullptr;
    }

    if (m_CTX)
    {
        Show("  - SSL_CTX_free");
        SSL_CTX_free(m_CTX);
        m_CTX = nullptr;
    }
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::LoadVerifyLocations()
{
    ShowFunc("COpenSSL_Client", __FUNC__);

    Show("  - SSL_CTX_load_verify_locations");
    if (int r = SSL_CTX_load_verify_locations(
            m_CTX, m_Certificate.c_str(), nullptr); r != 1)
        return SetLastError(tlse_SSL_CTX_load_verify_locations);
    return CTlsResult();
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::VerifyCertification()
{
    ShowFunc("COpenSSL_Client", __FUNC__);

    Show("  - SSL_CTX_set_verify");
    SSL_CTX_set_verify(m_CTX, SSL_VERIFY_PEER, nullptr);

    Show("  - SSL_get_verify_result");
    if (int r = SSL_get_verify_result(m_SSL); r != X509_V_OK)
    {
        return SetLastError(
            tlse_SSL_get_verify_result,
            X509_verify_cert_error_string(SSL_get_verify_result(m_SSL)));
    }
    return CTlsResult();
}
//----------------------------------------------------------------------------

CTlsResult COpenSSL_Client::Write(const std::string& message)
{
    ShowFunc("COpenSSL_Client", __FUNC__);

//    if (!is_connection_active(m_SSL))
//    {
//        return SetLastError("Connection is not active", "");
//        return false;
//    }

    Show("  - SSL_write");
    int ret = SSL_write(m_SSL, message.data(), message.size());
    if (ret > 0)
        return CTlsResult();

    return CTlsResult(tlse_SSL_write);

//    std::string s = CSSL_GetError::Reason(m_SSL, ret);
//    if (!s.empty())
//        return SetLastError(tlse_SSL_write, s);
//    else
//        return SetLastError(tlse_SSL_write);
}
//----------------------------------------------------------------------------

int COpenSSL_Client::Read(std::string& message)
{
    ShowFunc("COpenSSL_Client", __FUNC__);

    char buffer[1024];
    Show("  - SSL_read");
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

void COpenSSL_Client::ShowFunc(const std::string& class_, const std::string& func)
{
    if (m_DebugStringCb)
        m_DebugStringCb(std::string(class_) + "::" + std::string(func));
}
//----------------------------------------------------------------------------

void COpenSSL_Client::Show(const std::string& text)
{
    if (m_DebugStringCb)
        m_DebugStringCb(text);
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class CMessageDeque
// ----- -------------
//***************************************************************************

bool CMessageDeque::IsEmpty()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Deque.empty();
}
//----------------------------------------------------------------------------

size_t CMessageDeque::Count()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Deque.size();
}
//----------------------------------------------------------------------------

std::string CMessageDeque::Fetch()
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_Deque.empty())
        return std::string();

    std::string s = m_Deque.front();
    m_Deque.pop_front();
    return s;
}
//----------------------------------------------------------------------------

void CMessageDeque::Push(const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Deque.push_back(message);
}
//----------------------------------------------------------------------------

void CMessageDeque::Flush()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Deque.clear();
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class CClientReadThread
// ----- -----------------
//***************************************************************************

CClientReadThread::CClientReadThread(
    ztls::COpenSSL_Client* client, CTlsEventCb messageCb, CTlsEventCb closeCb,
    CTlsErrorCb errorCb)
  : TThread(false)
  , m_Client(client)
  , m_NewMessageCb(messageCb)
  , m_CloseNotifyCb(closeCb)
  , m_ErrorCb(errorCb)
{
}
//----------------------------------------------------------------------------

void __fastcall CClientReadThread::Execute()
{
    while (!Terminated)
    {
        std::string message;
        int bytes = m_Client->Read(message);

        if (bytes <= 0)
        {
            int err = SSL_get_error(m_Client->GetSSL(), bytes);

            int errNo = WSAGetLastError();

            if (err == SSL_ERROR_ZERO_RETURN) // close_notify
            {
                if (m_CloseNotifyCb)
                    m_CloseNotifyCb();
            }
            else if (err == SSL_ERROR_WANT_READ
                || (err == SSL_ERROR_SYSCALL && errNo == WSAETIMEDOUT))
            {
                // timeout
            }
            else
            {
                if (m_ErrorCb)
                    m_ErrorCb(err, errNo, "CClientReadThread::Execute");
            }
        }

        else
        {
            m_Deque.Push(message);
            if (m_NewMessageCb)
                m_NewMessageCb();
        }

        Sleep(100);
    }
}
//----------------------------------------------------------------------------


////***************************************************************************
////
//// class CTlsClientTimer
//// ----- ---------------
////***************************************************************************
//
//void CTlsClientTimer::Start(int interval, std::function<void()> task)
//{
//    m_Running = true;
//    m_Worker = std::thread([=]() {
//      while (m_Running) {
//        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
//        if (m_Running)
//            task();
//      }
//    });
//}
////----------------------------------------------------------------------------
//
//void CTlsClientTimer::Stop()
//{
//    m_Running = false;
//    if (m_Worker.joinable())
//        m_Worker.join();
//}
////----------------------------------------------------------------------------


//***************************************************************************
//
// class CTlsClient
// ----- ----------
//***************************************************************************

CTlsClient::~CTlsClient()
{
    Disconnect();
}
//----------------------------------------------------------------------------

CTlsResult CTlsClient::Connect()
{
    using namespace ztls;

    m_TcpClient = std::make_unique<ztls::CTcpClient>(m_Address, m_PortNo);
    m_TcpClient->SetDebugStringCallback(OnDebugString);

    m_SslClient = std::make_unique<ztls::COpenSSL_Client>();
    m_SslClient->SetCertificate(m_Certificate);
    m_SslClient->SetMinVersion(m_TLS_MinVersion);
    m_SslClient->SetDebugStringCallback(OnDebugString);

    if (CTlsResult r = m_TcpClient->Connect(); !r)
        return r;

    if (m_ReadTimeout != 0)
        m_TcpClient->SetReadTimeout(m_ReadTimeout);

    if (CTlsResult r = m_SslClient->CreateContext(); !r)
        return r;

    if (CTlsResult r = m_SslClient->SetVersions(); !r)
        return r;

    if (CTlsResult r = m_SslClient->CreateSSL(m_TcpClient->Socket()); !r)
        return r;

    if (CTlsResult r = m_SslClient->Connect(); !r)
        return r;

    if (CTlsResult r = m_SslClient->LoadVerifyLocations(); !r)
        return r;

//    if (CTlsResult r = sslClient.VerifyCertification(); !r)
//        return r;

//    if (CTlsResult r = m_SslClient->MakeConnection(*m_TcpClient); !r)
//        return r;

    // luodaan thread lukemaan vaataanotettua dataa
    m_ReadThread = std::make_unique<CClientReadThread>(
        m_SslClient.get(), OnNewMessage, OnCloseNotify, OnError);

    m_Connected = true;

    if (m_ConnectedCb)
        m_ConnectedCb();

    return CTlsResult();
}
//----------------------------------------------------------------------------

CTlsResult CTlsClient::Disconnect()
{
    if (!m_Connected)
        return CTlsResult();

    if (CTlsResult r = m_SslClient->Shutdown(); !r)
        ;

    m_TcpClient->Disconnect();
    m_SslClient->Free();

    if (m_ReadThread)
    {
        // terminoidaan thread
        m_ReadThread->Terminate();

        // odotetaan thredin päättymistä
        m_ReadThread->WaitFor();
    }

    m_Connected = false;

    if (m_DisconnectedCb)
        m_DisconnectedCb();

    return CTlsResult();
}
//----------------------------------------------------------------------------

CTlsResult CTlsClient::Write(const std::string& message)
{
    if (CTlsResult r = m_SslClient->Write(message); !r)
    {
        int errNo = WSAGetLastError();

//        std::string e = "*** WriteError: " + std::to_string(errNo);
//        SHOW(e.c_str());

//        if (errNo == WSAECONNRESET)  // 10054
//        {
//        }
//        else if (errNo == WSAENETRESET)
//        {
//        }
//        else
//        {
//        }

        return r;
    }

    return CTlsResult();
}
//----------------------------------------------------------------------------

//void CTlsClient::Timer()
//{
//    if (std::exchange(m_IsError, false))
//    {
//        Disconnect();
//        m_RetryConnect = true;
//    }
//
//    else if (std::exchange(m_CloseNotified, false))
//    {
//        Disconnect();
//    }
//
//    else if (std::exchange(m_RetryConnect, false))
//    {
//        Connect();
//    }
//}
////----------------------------------------------------------------------------

void CTlsClient::OnNewMessage()
{
    assert(m_ReadThread);

    while (!m_ReadThread->IsEmpty())
        m_Deque.Push(m_ReadThread->Fetch());

    if (m_NewMessageCb)
        m_NewMessageCb();
}
//----------------------------------------------------------------------------

void CTlsClient::OnCloseNotify()
{
    m_CloseNotified = true;
    if (m_CloseNotifyCb)
        m_CloseNotifyCb();
}
//----------------------------------------------------------------------------

void CTlsClient::OnError(int errType, int errNo, const std::string& source)
{
    if (errType == SSL_ERROR_SYSCALL)
    {
        DebugString("*** SSL_ERROR_SYSCALL");
    }
    else if (errType == SSL_ERROR_WANT_READ)
    {
        DebugString("*** SSL_ERROR_WANT_READ");
    }
    else if (errType == SSL_ERROR_WANT_WRITE)
    {
        DebugString("*** SSL_ERROR_WANT_WRITE");
    }
    else
    {
        DebugString("*** OTHER");
    }

    m_IsError = true;

    if (m_ErrorCb)
        m_ErrorCb(errType, errNo, source);
}
//----------------------------------------------------------------------------

void CTlsClient::OnDebugString(const std::string& s)
{
    DebugString(s);
}
//----------------------------------------------------------------------------

void CTlsClient::DebugString(const std::string& s)
{
    if (m_DebugStringCb)
        m_DebugStringCb(s);
}
//----------------------------------------------------------------------------

}

