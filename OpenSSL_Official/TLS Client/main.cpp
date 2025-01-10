//----------------------------------------------------------------------------
//
// Module: main
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "main.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

//#include <openssl/conf.h>
//#include <openssl/evp.h>
//#include <openssl/err.h>
//#include <openssl/ssl.h>
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"
TformMain *formMain;

#if (SSLEAY_VERSION_NUMBER >= 0x0907000L)
# include <openssl/conf.h>
#endif

TMemo* g_Memo {};

#define HOST_RESOURCE "/cgi-bin/randbyte?nbytes=32&format=h"

const char *request_start = "GET / HTTP/1.0\r\nConnection: close\r\nHost: ";
const char *request_end = "\r\n\r\n";
size_t written, readbytes;
char buf[160];
//char *hostname, *port;
int argnext = 1;
int ipv6 = 0;


void init_openssl_library()
{
  SSL_library_init();

  SSL_load_error_strings();

  OPENSSL_config(nullptr);
}
//----------------------------------------------------------------------------

int verify_callback(int preverify, X509_STORE_CTX* x509_ctx)
{
    int depth = X509_STORE_CTX_get_error_depth(x509_ctx);
    int err = X509_STORE_CTX_get_error(x509_ctx);

    X509* cert = X509_STORE_CTX_get_current_cert(x509_ctx);
    X509_NAME* iname = cert ? X509_get_issuer_name(cert) : NULL;
    X509_NAME* sname = cert ? X509_get_subject_name(cert) : NULL;

//    print_cn_name("Issuer (cn)", iname);
//    print_cn_name("Subject (cn)", sname);

    if(depth == 0) {
        /* If depth is 0, its the server's certificate. Print the SANs too */
//        print_san_name("Subject (san)", cert);
    }

    return preverify;
}
//----------------------------------------------------------------------------

/* Helper function to create a BIO connected to the server */
static BIO *create_socket_bio(
    std::string_view hostname, std::string_view port, int family)
{
    int sock = -1;
    BIO_ADDRINFO *res;
//    const BIO_ADDRINFO *ai = nullptr;
    BIO *bio;

    /*
     * Lookup IP address info for the server.
     */
    if (!BIO_lookup_ex(
            hostname.data(), port.data(), BIO_LOOKUP_CLIENT, family,
            SOCK_STREAM, 0, &res))
        return nullptr;

    /*
     * Loop through all the possible addresses for the server and find one
     * we can connect to.
     */
    for (const BIO_ADDRINFO* ai = res;
         ai != nullptr;
         ai = BIO_ADDRINFO_next(ai))
    {
        /*
         * Create a TCP socket. We could equally use non-OpenSSL calls such
         * as "socket" here for this and the subsequent connect and close
         * functions. But for portability reasons and also so that we get
         * errors on the OpenSSL stack in the event of a failure we use
         * OpenSSL's versions of these functions.
         */
        sock = BIO_socket(BIO_ADDRINFO_family(ai), SOCK_STREAM, 0, 0);
        if (sock == -1)
            continue;

        /* Connect the socket to the server's address */
        if (!BIO_connect(sock, BIO_ADDRINFO_address(ai), BIO_SOCK_NODELAY))
        {
            BIO_closesocket(sock);
            sock = -1;
            continue;
        }

        /* We have a connected socket so break out of the loop */
        break;
    }

    /* Free the address information resources we allocated earlier */
    BIO_ADDRINFO_free(res);

    /* If sock is -1 then we've been unable to connect to the server */
    if (sock == -1)
        return nullptr;

    /* Create a BIO to wrap the socket */
    bio = BIO_new(BIO_s_socket());
    if (!bio)
    {
        BIO_closesocket(sock);
        return nullptr;
    }

    /*
     * Associate the newly created BIO with the underlying socket. By
     * passing BIO_CLOSE here the socket will be automatically closed when
     * the BIO is freed. Alternatively you can use BIO_NOCLOSE, in which
     * case you must close the socket explicitly when it is no longer
     * needed.
     */
    BIO_set_fd(bio, sock, BIO_CLOSE);

    return bio;
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class COpenSSL_TLSClient_2
// ----- ------------------
//***************************************************************************

/*!
 */

class COpenSSL_TLSClient_2 {
  public:
    COpenSSL_TLSClient_2();
    ~COpenSSL_TLSClient_2();

    void SetHost(std::string_view host)
        { m_Host = host; }

    void SetPort(std::string_view port)
        { m_Port = port; }

    bool DoStuff();

  private:
    std::string m_Host {};
    std::string m_Port { "443" };

    SSL_CTX* m_CTX {};
    SSL* m_SSL {};
    BIO* m_BIO {};

    bool HandleFailure(std::string_view errorText);
};


COpenSSL_TLSClient_2::COpenSSL_TLSClient_2()
{
}
//----------------------------------------------------------------------------

COpenSSL_TLSClient_2::~COpenSSL_TLSClient_2()
{
    SSL_free(m_SSL);
    SSL_CTX_free(m_CTX);
}
//----------------------------------------------------------------------------

bool COpenSSL_TLSClient_2::DoStuff()
{
    /*
     * Create an SSL_CTX which we can use to create SSL objects from. We
     * want an SSL_CTX for creating clients so we use TLS_client_method()
     * here.
     */
    m_CTX = SSL_CTX_new(TLS_client_method());
    if (!m_CTX)
        return HandleFailure("Failed to create the SSL_CTX");

    /*
     * Configure the client to abort the handshake if certificate
     * verification fails. Virtually all clients should do this unless you
     * really know what you are doing.
     */
    SSL_CTX_set_verify(m_CTX, SSL_VERIFY_PEER, nullptr);
//    SSL_CTX_set_verify(m_CTX, SSL_VERIFY_NONE, nullptr);

    /* Use the default trusted certificate store */
    if (!SSL_CTX_set_default_verify_paths(m_CTX))
        return HandleFailure(
            "Failed to set the default trusted certificate store");

    /*
     * TLSv1.1 or earlier are deprecated by IETF and are generally to be
     * avoided if possible. We require a minimum TLS version of TLSv1.2.
     */
    if (!SSL_CTX_set_min_proto_version(m_CTX, TLS1_2_VERSION))
        return HandleFailure("Failed to set the minimum TLS protocol version");

    /* Create an SSL object to represent the TLS connection */
    m_SSL = SSL_new(m_CTX);
    if (!m_SSL)
        return HandleFailure("Failed to create the SSL object");

    /*
     * Create the underlying transport socket/BIO and associate it with the
     * connection.
     */
    m_BIO = create_socket_bio(
        m_Host.c_str(), m_Port.c_str(), ipv6 ? AF_INET6 : AF_INET);
    if (!m_BIO)
        return HandleFailure("Failed to crete the BIO");
    SSL_set_bio(m_SSL, m_BIO, m_BIO);

    /*
     * Tell the server during the handshake which hostname we are attempting
     * to connect to in case the server supports multiple hosts.
     */
    if (!SSL_set_tlsext_host_name(m_SSL, m_Host.c_str()))
        return HandleFailure("Failed to set the SNI hostname");

    /*
     * Ensure we check during certificate verification that the server has
     * supplied a certificate for the hostname that we were expecting.
     * Virtually all clients should do this unless you really know what you
     * are doing.
     */
    if (!SSL_set1_host(m_SSL, m_Host.c_str()))
        return HandleFailure(
            "Failed to set the certificate verification hostname");

    /* Do the handshake with the server */
    if (SSL_connect(m_SSL) < 1)
    {
        g_Memo->Lines->Add("Failed to connect to the server");

        /*
         * If the failure is due to a verification error we can get more
         * information about it from SSL_get_verify_result().
         */
        if (SSL_get_verify_result(m_SSL) != X509_V_OK)
        {
            char buf[256];
            sprintf(buf, "Verify error: %s",
                X509_verify_cert_error_string(SSL_get_verify_result(m_SSL)));
            g_Memo->Lines->Add(buf);
        }
        return false;
    }

    /* Write an HTTP GET request to the peer */
    if (!SSL_write_ex(m_SSL, request_start, strlen(request_start), &written))
        return HandleFailure("Failed to write start of HTTP request");

    if (!SSL_write_ex(m_SSL, m_Host.c_str(), m_Host.length(), &written))
//    if (!SSL_write_ex(m_SSL, hostname, strlen(hostname), &written))
        return HandleFailure("Failed to write hostname in HTTP request");

    if (!SSL_write_ex(m_SSL, request_end, strlen(request_end), &written))
        return HandleFailure("Failed to write end of HTTP request");

    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_TLSClient_2::HandleFailure(std::string_view errorText)
{
    g_Memo->Lines->Add(errorText.data());
    return false;
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class COpenSSL_TLSClient
// ----- ------------------
//***************************************************************************

/*!
 */

class COpenSSL_TLSClient {
  public:
    COpenSSL_TLSClient();
    ~COpenSSL_TLSClient();

    void SetHost(std::string_view host)
        { m_Host = host; }

    void SetPort(int port)
        { m_Port = port; }

    bool DoStuff();

  private:
    SSL_CTX* m_CTX {};
    BIO* m_WEB {};
    BIO* m_Out {};
    SSL* m_SSL {};

    std::string m_Host {};
    int m_Port { 443 };

    bool CreateCTX();

    bool MakeConfigs();

    bool VerifyLocations();

    bool SSL_Connect();

    bool SetHost();

    bool GetSSL();

    bool SelectCiphers();

    bool HandleFailure();
};


COpenSSL_TLSClient::COpenSSL_TLSClient()
{
}
//----------------------------------------------------------------------------

COpenSSL_TLSClient::~COpenSSL_TLSClient()
{
    if (m_Out)
      BIO_free(m_Out);

    if (m_WEB)
      BIO_free_all(m_WEB);

    if (m_CTX)
      SSL_CTX_free(m_CTX);
}
//----------------------------------------------------------------------------

bool COpenSSL_TLSClient::DoStuff()
{
    long res = 1;

    init_openssl_library();

    if (!CreateCTX())
        return false;

    MakeConfigs();

    if (!VerifyLocations())
        return false;

    if (!SSL_Connect())
        return false;

    if (!SetHost())
        return false;

    if (!GetSSL())
        return false;

    if (!SelectCiphers())
        return false;

    res = SSL_set_tlsext_host_name(m_SSL, m_Host.c_str());
    if (res != 1)
        return HandleFailure();

    m_Out = BIO_new_fp(stdout, BIO_NOCLOSE);
    if (!m_Out)
        return HandleFailure();

    res = BIO_do_handshake(m_WEB);
    if (res != 1)
        return HandleFailure();

    /* Step 1: verify a server certificate was presented during the negotiation */
    X509* cert = SSL_get_peer_certificate(m_SSL);
    if (cert)
        X509_free(cert); /* Free immediately */
    else if (nullptr == cert)
        return HandleFailure();

    /* Step 2: verify the result of chain verification */
    /* Verification performed according to RFC 4158    */
    res = SSL_get_verify_result(m_SSL);
    if (res != X509_V_OK)
        return HandleFailure();

//    /* Step 3: hostname verification */
//    /* An exercise left to the reader */
//
//    BIO_puts(m_WEB, "GET " HOST_RESOURCE " HTTP/1.1\r\n"
//                  "Host: " HOST_NAME "\r\n"
//                  "Connection: close\r\n\r\n");
//    BIO_puts(m_Out, "\n");
//
//    int len = 0;
//    do
//    {
//      char buff[1536] = {};
//      len = BIO_read(m_WEB, buff, sizeof(buff));
//
//      if (len > 0)
//          BIO_write(m_Out, buff, len);
//
//    } while (len > 0 || BIO_should_retry(m_WEB));

    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_TLSClient::CreateCTX()
{
    const SSL_METHOD* method = SSLv23_method();
    if (!method)
        return HandleFailure();

    m_CTX = SSL_CTX_new(method);
    if (!m_CTX)
        return HandleFailure();

    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_TLSClient::MakeConfigs()
{
    /* Cannot fail ??? */
    SSL_CTX_set_verify(m_CTX, SSL_VERIFY_PEER, verify_callback);

    /* Cannot fail ??? */
    SSL_CTX_set_verify_depth(m_CTX, 4);

    /* Cannot fail ??? */
    const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
    SSL_CTX_set_options(m_CTX, flags);

    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_TLSClient::VerifyLocations()
{
    int res = SSL_CTX_load_verify_locations(m_CTX, "y.pem", nullptr);
//    int res = SSL_CTX_load_verify_locations(m_CTX, "yonem.pem", nullptr);
    if (res != 1)
        return HandleFailure();
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_TLSClient::SSL_Connect()
{
    m_WEB = BIO_new_ssl_connect(m_CTX);
    if (!m_WEB)
        return HandleFailure();
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_TLSClient::SetHost()
{
    std::string s = m_Host + ":" + std::to_string(m_Port);
    int res = BIO_set_conn_hostname(m_WEB, s.c_str());
    if (res != 1)
        return HandleFailure();
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_TLSClient::GetSSL()
{
    BIO_get_ssl(m_WEB, &m_SSL);
    if (!m_SSL)
        return HandleFailure();
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_TLSClient::SelectCiphers()
{
    const char PREFERRED_CIPHERS[] = "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4";
    int res = SSL_set_cipher_list(m_SSL, PREFERRED_CIPHERS);
    if (res != 1)
        return HandleFailure();
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_TLSClient::HandleFailure()
{
    return false;
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class TformMain
// ----- ---------
//***************************************************************************

__fastcall TformMain::TformMain(TComponent* Owner)
  : TForm(Owner)
{
    g_Memo = memo;
}
//---------------------------------------------------------------------------

void __fastcall TformMain::FormShow(TObject *Sender)
{
//
}
//---------------------------------------------------------------------------

void __fastcall TformMain::Button1Click(TObject *Sender)
{
    COpenSSL_TLSClient_2 tls;
    tls.SetHost("localhost");
    tls.SetPort("12350");
//    tls.SetPort("443");
    tls.DoStuff();
}
//---------------------------------------------------------------------------

