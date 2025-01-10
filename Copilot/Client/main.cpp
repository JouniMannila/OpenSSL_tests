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

#include <ws2tcpip.h>
//----------------------------------------------------------------------------

//#pragma comment(lib, "ws2_32.lib")
//#pragma comment(lib, "libssl.lib")
//#pragma comment(lib, "libcrypto.lib")


#define NEW

//***************************************************************************
//
// class CTcpClient
// ----- ----------
//***************************************************************************

/*!
 */

class CTcpClient {
  public:
    ~CTcpClient();

    void SetPortNo(int portNo)
        { m_PortNo = portNo; }

    void SetAddress(std::string_view address)
        { m_Address = address; }

    /// Palauttaa Connect() metodissa luodun socket:in.
    int ServerSocket() const
        { return m_ServerSocket; }

    /// Kutsuu WSAStartup.
    bool Initialize();

    /// Luo TCP-socket:in ja yritt‰‰ kytkeyty‰ serveriin. Luodun Socket:in
    /// arvo tallettuu m_ServerSocket:iin.
    bool Connect();

    /// Palautaa edelliseen virheeseen liittyv‰n tekstin.
    std::string GetLastError() const
        { return m_LastError; }

  private:
    int m_PortNo {};
    std::string m_Address {};

    int m_ServerSocket {};

    std::string m_LastError {};

    /// Asettaa viimeisen virheen tekstin ja palauttaa aina false.
    bool SetLastError(std::string_view msg);
};


CTcpClient::~CTcpClient()
{
    if (m_ServerSocket)
        closesocket(m_ServerSocket);
    WSACleanup();
}
//----------------------------------------------------------------------------

bool CTcpClient::Initialize()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return SetLastError("ERROR: WSAStartup failed.");
    return true;
}
//----------------------------------------------------------------------------

bool CTcpClient::Connect()
{
    if (m_PortNo == 0)
        return SetLastError("ERROR: Port == 0.");

    if (m_Address.empty())
        return SetLastError("ERROR: Address == \"\".");

    // luodaan socket
    m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_ServerSocket < 0)
        return SetLastError("ERROR: Create socket failed.");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_PortNo);
    addr.sin_addr.s_addr = inet_addr(m_Address.c_str());

    // yritet‰‰n kytkeyty‰
    if (connect(m_ServerSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        return SetLastError("ERROR: Unable to connect.");

    return true;
}
//----------------------------------------------------------------------------

bool CTcpClient::SetLastError(std::string_view msg)
{
    m_LastError = msg;
    return false;
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class COpenSSL_Client
// ----- ---------------
//***************************************************************************

/*!
 */

class COpenSSL_Client {
  public:
    ~COpenSSL_Client();

    ///
    void Initialize();

    ///
    bool CreateContext();

    ///
    bool CreateSSL(int serverSocket);

    ///
    bool Connect();

    ///
    void Write(std::string_view message);

    ///
    bool Read(std::string& message);

    /// Palautaa edelliseen virheeseen liittyv‰n tekstin.
    std::string GetLastError() const
        { return m_LastError; }

  private:
    SSL_CTX* m_CTX {};
    SSL* m_SSL {};

    std::string m_LastError {};

    bool SetLastError(std::string_view msg);
};


COpenSSL_Client::~COpenSSL_Client()
{
    if (m_SSL)
    {
        SSL_shutdown(m_SSL);
        SSL_free(m_SSL);
        SSL_CTX_free(m_CTX);
    }
    EVP_cleanup();
}
//----------------------------------------------------------------------------

void COpenSSL_Client::Initialize()
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}
//----------------------------------------------------------------------------

bool COpenSSL_Client::CreateContext()
{
//    const SSL_METHOD* method = SSLv23_client_method();
//
//    m_CTX = SSL_CTX_new(method);


    m_CTX = SSL_CTX_new(TLS_client_method());

    if (!m_CTX)
        return SetLastError("ERROR: Unable to create SSL context.");

    if (SSL_CTX_set_min_proto_version(m_CTX, TLS1_VERSION) == 0)
        return SetLastError("ERROR: Unable to set min proto version.");

    if (SSL_CTX_set_max_proto_version(m_CTX, TLS1_2_VERSION) == 0)
        return SetLastError("ERROR: Unable to set max proto version.");

    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Client::CreateSSL(int fd)
{
    m_SSL = SSL_new(m_CTX);
    if (SSL_set_fd(m_SSL, fd) == 0)
        return SetLastError("ERROR: SSL_set_fd failed.");
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Client::Connect()
{
    if (SSL_connect(m_SSL) <= 0)
        return SetLastError("ERROR: SSL_connect failed.");
    return true;
}
//----------------------------------------------------------------------------

void COpenSSL_Client::Write(std::string_view message)
{
    SSL_write(m_SSL, message.data(), message.size());
}
//----------------------------------------------------------------------------

bool COpenSSL_Client::Read(std::string& message)
{
    char buffer[1024];
    int bytes = SSL_read(m_SSL, buffer, sizeof(buffer));
    if (bytes <= 0)
        return false;

    buffer[bytes] = 0;
    message = buffer;
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Client::SetLastError(std::string_view msg)
{
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

    method = SSLv23_client_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        std::cerr << "Unable to create SSL context" << std::endl;
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

#endif

int main()
{
  #ifdef NEW
    CTcpClient tcpClient;
    COpenSSL_Client sslClient;

    tcpClient.SetPortNo(12350);
    tcpClient.SetAddress("127.0.0.1");

    if (!tcpClient.Initialize())
    {
        std::cout << tcpClient.GetLastError() << std::endl;
        return 0;
    }

    if (!tcpClient.Connect())
    {
        std::cout << tcpClient.GetLastError() << std::endl;
        return 0;
    }

    sslClient.Initialize();

    if (!sslClient.CreateContext())
    {
        std::cout << sslClient.GetLastError() << std::endl;
        return 0;
    }

    if (!sslClient.CreateSSL(tcpClient.ServerSocket()))
    {
        std::cout << sslClient.GetLastError() << std::endl;
        return 0;
    }

    if (!sslClient.Connect())
    {
        std::cout << sslClient.GetLastError() << std::endl;
        return 0;
    }

    sslClient.Write("Hello from the client!");

    std::string message;
    if (sslClient.Read(message))
    {
        std::cout << "Received: " << message << std::endl;
    }
  #endif

  #ifdef OLD
    initialize_winsock();
    initialize_openssl();

    SSL_CTX* ctx = create_context();

    int server_fd;
    struct sockaddr_in addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(12350);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to connect");
        exit(EXIT_FAILURE);
    }

    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, server_fd);

    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
    } else {
        const char request[] = "Hello from the client!";
        SSL_write(ssl, request, strlen(request));

        char reply[256];
        int bytes = SSL_read(ssl, reply, sizeof(reply));
        if (bytes > 0) {
            reply[bytes] = 0;
            std::cout << "Received: " << reply << std::endl;
        }
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    closesocket(server_fd);
    SSL_CTX_free(ctx);
    cleanup_openssl();
    cleanup_winsock();
  #endif

    return 0;
}

