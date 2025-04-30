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
//---------------------------------------------------------------------------

using QMemoWriter = void (*)(void* _this, const std::string& text);

struct CFuncPtr {
  void* This {};
  QMemoWriter Func {};
};

extern CFuncPtr g_MemoWriter;


namespace ztls {

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

//    /// Palautaa edelliseen virheeseen liittyv‰n tekstin.
//    CTlsResult GetLastError() const
//        { return m_LastResult; }

  private:
    int m_PortNo {};
    std::string m_Address {};

    SOCKET m_Socket { INVALID_SOCKET };

    // tallettaa tietoa siit‰, onko WSAStartup() kutsuttu.
    bool m_WSAStartupCalled {};

//    CTlsResult m_LastResult {};

    /// Kutsuu WSAStartup.
    CTlsResult Initialize();

    /// Luo TCP-socket:in ja yritt‰‰ kytkeyty‰ serveriin. Luodun Socket:in
    /// arvo tallettuu m_Socket:iin.
    CTlsResult DoConnect();

    /// Asettaa viimeisen virheen tekstin ja palauttaa aina false.
    CTlsResult SetLastError(int errCode, const std::string& msg="");
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
    CTlsResult Write(std::string_view message);

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

    CError m_LastError {};

    CTlsResult SetLastError(
        int errCode, const std::string& msg=getOpenSSLError());
};

}

#endif

