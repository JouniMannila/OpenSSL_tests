//----------------------------------------------------------------------------
//
// Module: OpenSSL_Client
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

#ifndef OpenSSL_ClientH
#define OpenSSL_ClientH

//---------------------------------------------------------------------------
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <string>

#ifndef TlsResultH
#   include "TlsResult.h"
#endif
#ifndef ErrorH
#   include "Error.h"
#endif

#include <deque>
#include <mutex>
#include <thread>
//---------------------------------------------------------------------------

namespace ztls {

typedef void (__closure* CTlsEventCb)();
typedef void (__closure* CTlsErrorCb)(
    int errType, int errNo, const std::string& source);
typedef void (__closure* CDebugStringCb)(const std::string&);

//***************************************************************************
//
// class CTcpClient
// ----- ----------
//***************************************************************************

/*!
 */

class CTcpClient {
  public:
    CTcpClient() = default;

    /// Constructor, joka asettaa serverin osoitteen ja portin.
    CTcpClient(const std::string& address, int portNo)
      : m_Address(address), m_PortNo(portNo) {}

    ~CTcpClient();

    CTcpClient(const CTcpClient&) = delete;
    CTcpClient& operator=(const CTcpClient&) = delete;

    /// Asetetaa serverin portin numeron.
    void SetPortNo(int portNo)
        { m_PortNo = portNo; }

    /// Asettaa serverin osoitteen.
    void SetAddress(const std::string& address)
        { m_Address = address; }

    ///
    void SetDebugStringCallback(CDebugStringCb cb)
        { m_DebugStringCb = cb; }

    /// Palauttaa Connect() metodissa luodun socket:in.
    SOCKET Socket() const
        { return m_Socket; }

    /// Asettaa ...
    void SetReadTimeout(DWORD timeout);

    /// Luo socketin kutsumalla socket() ja yritt‰‰ kytkeyty‰ serveriin
    /// kutsumalla connect().
    CTlsResult Connect();

    /// Jos ollaan kytkeytyneen‰ serveriin, irtikytkeydyt‰‰n kutsumalla
    /// closesocket(), ja jos WSAStartup() kutsuttu aiemmin, siivotaan nyt
    /// kutsumalla WSACleanup().
    void Disconnect();

  private:
    int m_PortNo {};
    std::string m_Address {};

    SOCKET m_Socket { INVALID_SOCKET };

    // tallettaa tietoa siit‰, onko WSAStartup() kutsuttu.
    bool m_WSAStartupCalled {};

    CDebugStringCb m_DebugStringCb{ nullptr };

    /// Kutsuu WSAStartup.
    CTlsResult Initialize();

    /// Luo TCP-socket:in ja yritt‰‰ kytkeyty‰ serveriin. Luodun Socket:in
    /// arvo tallettuu m_Socket:iin.
    CTlsResult DoConnect();

    /// Asettaa viimeisen virheen tekstin ja palauttaa aina false.
    CTlsResult SetLastError(int errCode, const std::string& msg="");

    void ShowFunc(const std::string& class_, const std::string& func);
    void Show(const std::string&);
};


//***************************************************************************
//
// class COpenSSL_Client
// ----- ---------------
//***************************************************************************

/*!
 */

class COpenSSL_Client {
  public:
    COpenSSL_Client() = default;
    COpenSSL_Client(const std::string& cert) : m_Certificate(cert) {}

    ~COpenSSL_Client();

    COpenSSL_Client(const COpenSSL_Client&) = delete;
    COpenSSL_Client& operator=(const COpenSSL_Client&) = delete;

    ///
    void SetCertificate(const std::string& cert)
        { m_Certificate = cert; }

    ///
    void SetMinVersion(int minVersion)
        { m_TLS_MinVersion = minVersion; }

    ///
    void SetDebugStringCallback(CDebugStringCb cb)
        { m_DebugStringCb = cb; }

    ///
    SSL_CTX* GetCTX() const
        { return m_CTX; };

    ///
    SSL* GetSSL() const
        { return m_SSL; }

    ///
    void Initialize();

    ///
    CTlsResult CreateContext();

    ///
    CTlsResult SetVersions();

    ///
    CTlsResult CreateSSL(int serverSocket);

    ///
    CTlsResult DisplayCerts();

    ///
    CTlsResult Connect();

    ///
    CTlsResult Shutdown();

    ///
    void Free();

    ///
    CTlsResult LoadVerifyLocations();

    ///
    CTlsResult VerifyCertification();

    ///
    CTlsResult MakeConnection(CTcpClient&);

    ///
    CTlsResult Write(const std::string& message);

    ///
    int Read(std::string& message);

    /// Palautaa edelliseen virheeseen liittyv‰n tekstin.
    CError GetLastError() const
        { return m_LastError; }

  private:
    SSL_CTX* m_CTX {};
    SSL* m_SSL {};

    int m_TLS_MinVersion { TLS1_VERSION };
    int m_TLS_MaxVersion { TLS1_2_VERSION };

    std::string m_Certificate {};

    CDebugStringCb m_DebugStringCb{ nullptr };

    CError m_LastError {};

    CTlsResult SetLastError(
        int errCode, const std::string& msg=getOpenSSLError());

    void ShowFunc(const std::string& class_, const std::string& func);
    void Show(const std::string& text);
};



//***************************************************************************
//
// class CMessageDeque
// ----- -------------
//***************************************************************************

/*!
 */

class CMessageDeque {
  public:
    /// Palauttaa tiedon siit‰, onko vastaanottojono tyhj‰.
    bool IsEmpty();

    /// Palauttaa tiedon siit‰, montako viesti‰ deque sis‰lt‰‰.
    size_t Count();

    // kopioi viestin jonoon mutex:in sis‰ll‰
    void Push(const std::string& message);

    /// Lukee ja poistaa vanhimman viestin jonosta.
    std::string Fetch();

    ///
    void Flush();

  private:
    std::mutex m_Mutex {};
    std::deque<std::string> m_Deque {};
};


//***************************************************************************
//
// class CClientReadThread
// ----- -----------------
//***************************************************************************

/*!
 */

class CClientReadThread : public TThread {
  public:
    CClientReadThread(
        ztls::COpenSSL_Client*, CTlsEventCb, CTlsEventCb, CTlsErrorCb);

    /// Palauttaa tiedon siit‰, onko vastaanottojono tyhj‰.
    bool __fastcall IsEmpty()
        { return m_Deque.IsEmpty(); }

    /// Lukee ja poistaa vanhimman viestin jonosta.
    std::string __fastcall Fetch()
        { return m_Deque.Fetch(); }

    ///
    void Flush()
        { m_Deque.Flush(); }

  private:
    ztls::COpenSSL_Client* m_Client {};

    CMessageDeque m_Deque {};

    CTlsEventCb m_NewMessageCb { nullptr };
    CTlsEventCb m_CloseNotifyCb { nullptr };
    CTlsErrorCb m_ErrorCb { nullptr };

    // thread:in suorittava looppi
    void __fastcall Execute();
};


////***************************************************************************
////
//// class CTlsClientTimer
//// ----- ---------------
////***************************************************************************
//
///*!
// */
//
//class CTlsClientTimer {
//  public:
//
//    void Start(int interval, std::function<void()> task);
//    void Stop();
//
//  private:
//    std::atomic<bool> m_Running {};
//    std::thread m_Worker {};
//};


//***************************************************************************
//
// class CTlsClient
// ----- ----------
//***************************************************************************

/*!
 */

class CTlsClient {
  public:
    CTlsClient() = default;

    ///
    explicit CTlsClient(
        const std::string& address="", int portNo=0, const std::string& cert="")
      : m_Address(address), m_PortNo(portNo), m_Certificate(cert) {}

    ~CTlsClient();

    CTlsClient(const CTlsClient&) = delete;
    CTlsClient& operator=(const CTlsClient&) = delete;

    /// Asetetaa serverin portin numeron.
    void SetPortNo(int portNo)
        { m_PortNo = portNo; }

    /// Asettaa serverin osoitteen.
    void SetAddress(const std::string& address)
        { m_Address = address; }

    /// Asettaa sertifikaatin.
    void SetCertificate(const std::string& cert)
        { m_Certificate = cert; }

    /// Asettaa ...
    void SetReadTimeout(DWORD timeout)
        { m_ReadTimeout = timeout; }

    ///
    void SetMinVersion(int minVersion)
        { m_TLS_MinVersion = minVersion; }

    ///
    void SetConnectedCallback(CTlsEventCb cb)
        { m_ConnectedCb = cb; }

    ///
    void SetDisconnectedCallback(CTlsEventCb cb)
        { m_DisconnectedCb = cb; }

    ///
    void SetNewMessageCallback(CTlsEventCb cb)
        { m_NewMessageCb = cb; }

    ///
    void SetCloseNotifyCallback(CTlsEventCb cb)
        { m_CloseNotifyCb = cb; }

    ///
    void SetErrorCallback(CTlsErrorCb cb)
        { m_ErrorCb = cb; }

    ///
    void SetDebugStringCallback(CDebugStringCb cb)
        { m_DebugStringCb = cb; }

    ///
    SSL_CTX* GetCTX() const
        { return m_SslClient ? m_SslClient->GetCTX() : nullptr; }

    ///
    SSL* GetSSL() const
        { return m_SslClient ? m_SslClient->GetSSL() : nullptr; }

    ///
    bool HasMessages()
        { return !m_Deque.IsEmpty(); }

    ///
    std::string Fetch()
        { return m_Deque.Fetch(); }

    ///
    bool Connected() const
        { return m_Connected; }

    ///
    CTlsResult Connect();

    ///
    CTlsResult Disconnect();

    ///
    CTlsResult Write(const std::string& message);

  private:
    int m_PortNo {};
    std::string m_Address {};
    std::string m_Certificate {};

    DWORD m_ReadTimeout { 5000 };

    int m_TLS_MinVersion { TLS1_VERSION };

    std::unique_ptr<ztls::CTcpClient> m_TcpClient {};
    std::unique_ptr<ztls::COpenSSL_Client> m_SslClient {};

    std::unique_ptr<CClientReadThread> m_ReadThread {};

    CMessageDeque m_Deque {};

    bool m_Connected {};
    bool m_CloseNotified {};
    bool m_IsError {};

    CTlsEventCb m_ConnectedCb { nullptr };
    CTlsEventCb m_DisconnectedCb { nullptr };
    CTlsEventCb m_NewMessageCb { nullptr };
    CTlsEventCb m_CloseNotifyCb{ nullptr };
    CTlsErrorCb m_ErrorCb{ nullptr };

    CDebugStringCb m_DebugStringCb{ nullptr };

    void OnNewMessage();
    void OnCloseNotify();
    void OnError(int errType, int errNo, const std::string& source);

    void OnDebugString(const std::string&);
    void DebugString(const std::string&);
};

}

#endif

