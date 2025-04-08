//----------------------------------------------------------------------------
//
// Module: main
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

//----------------------------------------------------------------------------
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <winsock2.h>
//----------------------------------------------------------------------------

#pragma comment(lib, "ws2_32.lib")
//#pragma comment(lib, "libssl.lib")
//#pragma comment(lib, "libcrypto.lib")

#define SHOWFUNC(class)  std::cout << class << "::" << __FUNC__ << std::endl;
#define SHOW(text)  std::cout << text << std::endl;

#define NEW


std::string getOpenSSLError()
{
    BIO *bio = BIO_new(BIO_s_mem());
    ERR_print_errors(bio);

    char *buf;
    size_t len = BIO_get_mem_data(bio, &buf);
    std::string s(buf, len);

    BIO_free(bio);
    return s;
}
//----------------------------------------------------------------------------


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

    /// Luo TCP-socket:in ja yrittää kytkeytyä serveriin. Luodun Socket:in
    /// arvo tallettuu m_ServerSocket:iin.
    bool Connect();

    ///
    bool Accept();

    /// Palautaa edelliseen virheeseen liittyvän tekstin.
    std::string GetLastError() const
        { return m_LastError; }

  private:
    int m_PortNo {};

    int m_ServerSocket {};
    int m_ClientSocket {};

    std::string m_LastError {};

    /// Asettaa viimeisen virheen tekstin ja palauttaa aina false.
    bool SetLastError(std::string_view msg);
};


CTcpServer::~CTcpServer()
{
    SHOWFUNC("CTcpServer")

    if (m_ServerSocket)
        closesocket(m_ServerSocket);
    WSACleanup();
}
//----------------------------------------------------------------------------

bool CTcpServer::Initialize()
{
    SHOWFUNC("CTcpServer")

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return SetLastError("ERROR: WSAStartup failed.");
    return true;
}
//----------------------------------------------------------------------------

bool CTcpServer::Connect()
{
    SHOWFUNC("CTcpServer")

    if (m_PortNo == 0)
        return SetLastError("ERROR: Port == 0.");

    // luodaan socket
    m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_ServerSocket < 0)
        return SetLastError("ERROR: Create socket failed.");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_PortNo);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(m_ServerSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        return SetLastError("ERROR: Unable to bind.");

    if (listen(m_ServerSocket, 1) < 0)
        return SetLastError("ERROR: Unable to listen.");

    return true;
}
//----------------------------------------------------------------------------

bool CTcpServer::Accept()
{
    SHOWFUNC("CTcpServer")

    struct sockaddr_in addr;
    int len = sizeof(addr);

    m_ClientSocket = accept(m_ServerSocket, (struct sockaddr*)&addr, &len);
    if (m_ClientSocket < 0)
        return SetLastError("ERROR: Unable to accept.");

    return true;
}
//----------------------------------------------------------------------------

bool CTcpServer::SetLastError(std::string_view msg)
{
    SHOWFUNC("CTcpServer")

    m_LastError = msg;
    return false;
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class COpenSSL_Server
// ----- ---------------
//***************************************************************************

/*!
 */

class COpenSSL_Server {
  public:
    ~COpenSSL_Server();

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
    bool Write(std::string_view msg);

    SSL* GetSSL() const
        { return m_SSL; }

    /// Palautaa edelliseen virheeseen liittyvän tekstin.
    std::string GetLastError() const
        { return m_LastError; }

  private:
    SSL_CTX* m_CTX {};
    SSL* m_SSL {};

    std::string m_LastError {};

    /// Asettaa viimeisen virheen tekstin ja palauttaa aina false.
    bool SetLastError(std::string_view msg);
};


COpenSSL_Server::~COpenSSL_Server()
{
    SHOWFUNC("COpenSSL_Server")

    if (m_SSL)
    {
        SHOW("  - SSL_shutdown")
        SSL_shutdown(m_SSL);
        SHOW("  - SSL_free")
        SSL_free(m_SSL);
        SHOW("  - SSL_CTX_free")
        SSL_CTX_free(m_CTX);
    }
    EVP_cleanup();
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
    if (SSL_CTX_use_certificate_file(m_CTX, "certificate.pem", SSL_FILETYPE_PEM) <= 0)
        return SetLastError(getOpenSSLError());

    SHOW("  - SSL_CTX_use_PrivateKey_file")
    if (SSL_CTX_use_PrivateKey_file(m_CTX, "privatekey.pem", SSL_FILETYPE_PEM) <= 0)
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
        return SetLastError("ERROR: SSL_set_fd failed.");
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Server::Accept()
{
    SHOWFUNC("COpenSSL_Server")

    SHOW("  - SSL_accept")
    if (SSL_accept(m_SSL) <= 0)
        return SetLastError("ERROR: SSL_accept failed.");
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Server::Write(std::string_view msg)
{
    SHOWFUNC("COpenSSL_Server")

    SHOW("  - SSL_write")
    if (SSL_write(m_SSL, msg.data(), msg.size()) == 0)
        return SetLastError("ERROR: SSL_accept failed.");
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Server::SetLastError(std::string_view msg)
{
    SHOWFUNC("COpenSSL_Server")

    m_LastError = msg;
    return false;
}
//----------------------------------------------------------------------------


#ifdef OLD

void initialize_winsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void cleanup_winsock() {
    WSACleanup();
}

void initialize_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl() {
    EVP_cleanup();
}

SSL_CTX* create_context() {
    const SSL_METHOD* method;
    SSL_CTX* ctx;

//    method = SSLv23_server_method();
//    ctx = SSL_CTX_new(method);

    ctx = SSL_CTX_new(TLS_server_method());

    if (!ctx) {
        std::cerr << "Unable to create SSL context" << std::endl;
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_set_min_proto_version(ctx, TLS1_VERSION) == 0) {
        std::cerr << "ERROR: Unable to set min proto version." << std::endl;
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_set_max_proto_version(ctx, TLS1_2_VERSION) == 0) {
        std::cerr << "ERROR: Unable to set max proto version." << std::endl;
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX* ctx) {
    SSL_CTX_set_ecdh_auto(ctx, 1);

    // Set the key and cert
    if (SSL_CTX_use_certificate_file(ctx, "csr.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "privatekey.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

#endif


int main()
{
  #ifdef NEW
    CTcpServer tcpServer;
    tcpServer.SetPortNo(12350);
    tcpServer.Initialize();

    COpenSSL_Server sslServer;
    sslServer.Initialize();

    if (!sslServer.CreateContext())
    {
        std::cout << sslServer.GetLastError() << std::endl;
        return 0;
    }

    if (!sslServer.ConfigureContext())
    {
        std::cout << sslServer.GetLastError() << std::endl;
        return 0;
    }

    if (!tcpServer.Connect())
    {
        std::cout << tcpServer.GetLastError() << std::endl;
        return 0;
    }

    while (1)
    {
        if (!tcpServer.Accept())
        {
            std::cout << tcpServer.GetLastError() << std::endl;
            return 0;
        }

        if (!sslServer.CreateSSL(tcpServer.ClientSocket()))
        {
            std::cout << sslServer.GetLastError() << std::endl;
            return 0;
        }

        if (!sslServer.Accept())
        {
            std::cout << sslServer.GetLastError() << std::endl;
            return 0;
        }
        else
        {
            sslServer.Write("Hello, SSL/TLS world!");
        }
    }

  #endif

  #ifdef OLD

    initialize_winsock();
    initialize_openssl();

    SSL_CTX* ctx = create_context();
    configure_context(ctx);

    int server_fd;
    struct sockaddr_in addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(12350);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) < 0) {
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        struct sockaddr_in addr;
        int len = sizeof(addr);
        SSL* ssl;

        int client = accept(server_fd, (struct sockaddr*)&addr, &len);
        if (client < 0) {
            perror("Unable to accept");
            exit(EXIT_FAILURE);
        }

        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client);

        if (SSL_accept(ssl) <= 0)
        {
            std::cout << getOpenSSLError() << std::endl;
        }
        else
        {
            const char reply[] = "Hello, SSL/TLS world!";
            SSL_write(ssl, reply, strlen(reply));
        }

        SSL_shutdown(ssl);
        SSL_free(ssl);
        closesocket(client);
    }

    closesocket(server_fd);
    SSL_CTX_free(ctx);
    cleanup_openssl();
    cleanup_winsock();

  #endif
}

