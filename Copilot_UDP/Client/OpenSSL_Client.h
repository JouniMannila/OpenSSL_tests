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

#include "Error.h"
//---------------------------------------------------------------------------

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
    CTcpClient(const std::string& address, int portNo)
      : m_Address(address), m_PortNo(portNo) {}

    ~CTcpClient();

    CTcpClient(const CTcpClient&) = delete;
    CTcpClient& operator=(const CTcpClient&) = delete;

    ///
    void SetPortNo(int portNo)
        { m_PortNo = portNo; }

    ///
    void SetAddress(const std::string& address)
        { m_Address = address; }

    /// Palauttaa Connect() metodissa luodun socket:in.
    int ServerSocket() const
        { return m_ServerSocket; }

    ///
    bool Connect();

    /// Palautaa edelliseen virheeseen liittyv‰n tekstin.
    CError GetLastError() const
        { return m_LastError; }

  private:
    int m_PortNo {};
    std::string m_Address {};

    int m_ServerSocket {};

    CError m_LastError {};

    /// Kutsuu WSAStartup.
    bool Initialize();

    /// Luo TCP-socket:in ja yritt‰‰ kytkeyty‰ serveriin. Luodun Socket:in
    /// arvo tallettuu m_ServerSocket:iin.
    bool DoConnect();

    /// Asettaa viimeisen virheen tekstin ja palauttaa aina false.
    bool SetLastError(const std::string& caption, const std::string& msg="");
};


//***************************************************************************
//
// class CUdpClient
// ----- ----------
//***************************************************************************

/*!
 */

class CUdpClient {
  public:
    CUdpClient() = default;
    CUdpClient(const std::string& address, int portNo)
      : m_Address(address), m_PortNo(portNo) {}

    ~CUdpClient();

    CUdpClient(const CUdpClient&) = delete;
    CUdpClient& operator=(const CUdpClient&) = delete;

    ///
    void SetPortNo(int portNo)
        { m_PortNo = portNo; }

    ///
    void SetAddress(const std::string& address)
        { m_Address = address; }

    struct sockaddr_in ServerAddr() const
        { return m_ServerAddr; };

    /// Palauttaa Connect() metodissa luodun socket:in.
    int UdpSocket() const
        { return m_UdpSocket; }

    ///
    bool Connect();

    /// Palautaa edelliseen virheeseen liittyv‰n tekstin.
    CError GetLastError() const
        { return m_LastError; }

  private:
    int m_PortNo {};
    std::string m_Address {};

    struct sockaddr_in m_ServerAddr {};

    int m_UdpSocket {};

    CError m_LastError {};

    /// Kutsuu WSAStartup.
    bool Initialize();

    /// Luo TCP-socket:in ja yritt‰‰ kytkeyty‰ serveriin. Luodun Socket:in
    /// arvo tallettuu m_ServerSocket:iin.
    bool DoConnect();

    /// Asettaa viimeisen virheen tekstin ja palauttaa aina false.
    bool SetLastError(const std::string& caption, const std::string& msg="");
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
    bool CreateContext();

    ///
    bool SetVersions();

    ///
    bool CreateSSL(SOCKET udpSocket, struct sockaddr_in);

    ///
    bool DisplayCerts();

    ///
    bool Connect();

    ///
    bool LoadVerifyLocations();

    ///
    bool VerifyCertification();

    ///
//    bool MakeConnection(CTcpClient&);

    ///
    bool Write(std::string_view message);

    ///
    bool Read(std::string& message);

    /// Palautaa edelliseen virheeseen liittyv‰n tekstin.
    CError GetLastError() const
        { return m_LastError; }

  private:
    SSL_CTX* m_CTX {};
    SSL* m_SSL {};
    BIO* m_BIO {};

    int m_TLS_MinVersion { TLS1_VERSION };
    int m_TLS_MaxVersion { TLS1_2_VERSION };

    std::string m_Certificate {};

    CError m_LastError {};

    bool SetLastError(
        const std::string& caption, const std::string& msg=getOpenSSLError());
};

}

#endif

