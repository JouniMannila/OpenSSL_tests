//----------------------------------------------------------------------------
//
// Module: OpenSSL_Server
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

#ifndef OpenSSL_ServerH
#define OpenSSL_ServerH

//---------------------------------------------------------------------------
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <string>

#include "Error.h"
//---------------------------------------------------------------------------

namespace ztls {

//***************************************************************************
//
// class CTcpServer
// ----- ----------
//***************************************************************************

/*!
 */

class CTcpServer {
  public:
    CTcpServer() = default;
    CTcpServer(int portNo) : m_PortNo(portNo) {}

    ~CTcpServer();

    void SetPortNo(int portNo)
        { m_PortNo = portNo; }

    /// Palauttaa Connect() metodissa luodun socket:in.
    int ServerSocket() const
        { return m_ServerSocket; }

    /// Palauttaa Accept() metodissa luodun socket:in.
    int ClientSocket() const
        { return m_ClientSocket; }

    /// Kutsuu WSAStartup.
    bool Initialize();

    /// Luo TCP-socket:in ja yritt‰‰ kytkeyty‰ serveriin. Luodun Socket:in
    /// arvo tallettuu m_ServerSocket:iin.
    bool Connect();

    ///
    bool Accept();

    ///
    bool Shutdown();

    /// Palautaa edelliseen virheeseen liittyv‰n tekstin.
    CError GetLastError() const
        { return m_LastError; }

  private:
    int m_PortNo {};

    int m_ServerSocket {};
    int m_ClientSocket {};

    CError m_LastError {};

    /// Asettaa viimeisen virheen tekstin ja palauttaa aina false.
    bool SetLastError(std::string_view caption, std::string_view msg="");
};


//***************************************************************************
//
// class COpenSSL_Server
// ----- ---------------
//***************************************************************************

/*!
 */

class COpenSSL_Server {
  public:
    COpenSSL_Server() = default;
    ~COpenSSL_Server();

    COpenSSL_Server(const COpenSSL_Server&) = delete;
    COpenSSL_Server& operator=(const COpenSSL_Server&) = delete;

    ///
    void SetPrivateKey(const std::string& key)
        { m_PrivateKey = key; }

    ///
    void SetCertificate(const std::string& cert)
        { m_Certificate = cert; }

    ///
    void Initialize();

    ///
    bool ConfigureContext();

    ///
    bool CreateContext();

    ///
    bool CreateSSL(int fd);

    ///
    bool Accept();

    ///
    bool Shutdown();

    ///
    bool Write(std::string_view msg);

    ///
    bool Read(std::string&);

    ///
    SSL* GetSSL() const
        { return m_SSL; }

    /// Palautaa edelliseen virheeseen liittyv‰n tekstin.
    CError GetLastError() const
        { return m_LastError; }

  private:
    SSL_CTX* m_CTX {};
    SSL* m_SSL {};

    std::string m_PrivateKey {};
    std::string m_Certificate {};

    CError m_LastError {};

    bool SetLastError(
        std::string_view caption, std::string_view msg=getOpenSSLError());
};

}

#endif
