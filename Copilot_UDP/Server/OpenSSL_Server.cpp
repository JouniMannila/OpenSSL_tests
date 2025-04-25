//----------------------------------------------------------------------------
//
// Module: OpenSSL_Server
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#pragma hdrstop

#include "OpenSSL_Server.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace ztls {

#define SHOWFUNC(class)  std::cout << class << "::" << __FUNC__ << std::endl;
#define SHOW(text)  std::cout << text << std::endl;

//***************************************************************************
//
// class CTcpServer
// ----- ----------
//***************************************************************************

CTcpServer::~CTcpServer()
{
    SHOWFUNC("CTcpServer")

    if (m_ServerSocket)
    {
        SHOW("  - closesocket")
        closesocket(m_ServerSocket);
    }
    SHOW("  - WSACleanup")
    WSACleanup();
}
//----------------------------------------------------------------------------

bool CTcpServer::Initialize()
{
    SHOWFUNC("CTcpServer")

    WSADATA wsaData;
    SHOW("  - WSAStartup")
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return SetLastError("WSAStartup failed.");
    return true;
}
//----------------------------------------------------------------------------

bool CTcpServer::Connect()
{
    SHOWFUNC("CTcpServer")

    if (m_PortNo == 0)
        return SetLastError("ERROR: Port == 0.");

    // luodaan socket
    SHOW("  - socket")
    m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_ServerSocket < 0)
        return SetLastError("Create socket failed.");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_PortNo);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    SHOW("  - bind")
    if (bind(m_ServerSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        return SetLastError("Unable to bind.");

    SHOW("  - listen")
    if (listen(m_ServerSocket, 1) < 0)
        return SetLastError("Unable to listen.");

    return true;
}
//----------------------------------------------------------------------------

bool CTcpServer::Accept()
{
    SHOWFUNC("CTcpServer")

    struct sockaddr_in addr;
    int len = sizeof(addr);

    SHOW("  - accept")
    m_ClientSocket = accept(m_ServerSocket, (struct sockaddr*)&addr, &len);
    if (m_ClientSocket < 0)
        return SetLastError("Unable to accept.");

    return true;
}
//----------------------------------------------------------------------------

bool CTcpServer::SetLastError(std::string_view caption, std::string_view msg)
{
    m_LastError.Caption = caption;
    m_LastError.Message = msg;
    return false;
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class CUdpServer
// ----- ----------
//***************************************************************************

CUdpServer::~CUdpServer()
{
    SHOWFUNC("CUdpServer")

    if (m_UdpSocket)
    {
        SHOW("  - closesocket")
        closesocket(m_UdpSocket);
    }
    SHOW("  - WSACleanup")
    WSACleanup();
}
//----------------------------------------------------------------------------

bool CUdpServer::Initialize()
{
    SHOWFUNC("CUdpServer")

    WSADATA wsaData;
    SHOW("  - WSAStartup")
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return SetLastError("WSAStartup failed.");
    return true;
}
//----------------------------------------------------------------------------

bool CUdpServer::Connect()
{
    SHOWFUNC("CUdpServer")

    if (m_PortNo == 0)
        return SetLastError("ERROR: Port == 0.");

    // luodaan socket
    SHOW("  - socket")
    m_UdpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_UdpSocket < 0)
        return SetLastError("Create socket failed.");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(m_PortNo);

    SHOW("  - bind")
    if (bind(m_UdpSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        return SetLastError("Unable to bind.");

    return true;
}
//----------------------------------------------------------------------------

bool CUdpServer::SetLastError(std::string_view caption, std::string_view msg)
{
    m_LastError.Caption = caption;
    m_LastError.Message = msg;
    return false;
}
//----------------------------------------------------------------------------



#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib") // Link with ws2_32.lib

#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    SOCKET udpSocket;
    sockaddr_in serverAddr, clientAddr;
    char buffer[BUFFER_SIZE];
    int clientAddrLen = sizeof(clientAddr);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed. Error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Create UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed. Error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Configure server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(udpSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed. Error: " << WSAGetLastError() << std::endl;
        closesocket(udpSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "UDP server is running on port " << PORT << "..." << std::endl;

    // Receive data
    while (true) {
        int bytesReceived = recvfrom(udpSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrLen);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recvfrom failed. Error: " << WSAGetLastError() << std::endl;
            break;
        }

        buffer[bytesReceived] = '\0'; // Null-terminate the received data
        std::cout << "Received message: " << buffer << std::endl;

        // Send response
        const char* response = "Message received!";
        sendto(udpSocket, response, strlen(response), 0, (sockaddr*)&clientAddr, clientAddrLen);
    }

    // Cleanup
    closesocket(udpSocket);
    WSACleanup();
    return 0;
}

//***************************************************************************
//
// class COpenSSL_Server
// ----- ---------------
//***************************************************************************

COpenSSL_Server::~COpenSSL_Server()
{
    SHOWFUNC("COpenSSL_Server")

    if (m_SSL)
    {
        SHOW("  - SSL_shutdown")
        SSL_shutdown(m_SSL);
        SHOW("  - SSL_free")
        SSL_free(m_SSL);
        SHOW("  - BIO_free")
        BIO_free(m_BIO);
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

    SHOW("  - DTLS_server_method")
    m_CTX = SSL_CTX_new(DTLS_server_method());
//    m_CTX = SSL_CTX_new(TLS_server_method());
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
    if (SSL_CTX_use_certificate_file(
            m_CTX, m_Certificate.c_str(), SSL_FILETYPE_PEM) <= 0)
        return SetLastError(getOpenSSLError());

    SHOW("  - SSL_CTX_use_PrivateKey_file")
    if (SSL_CTX_use_PrivateKey_file(
            m_CTX, m_PrivateKey.c_str(), SSL_FILETYPE_PEM) <= 0)
        return SetLastError(getOpenSSLError());

    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Server::CreateSSL(SOCKET socket)
{
    SHOWFUNC("COpenSSL_Server")

    m_SSL = SSL_new(m_CTX);
    if (!m_SSL)
        return SetLastError("SSL_new failed.");

//    SHOW("  - SSL_set_fd")
//    if (SSL_set_fd(m_SSL, fd) == 0)
//        return SetLastError("SSL_set_fd failed.");

    m_BIO = BIO_new_dgram(socket, BIO_NOCLOSE);
    if (!m_BIO)
        return SetLastError("BIO_new_dgram failed.");

    SSL_set_bio(m_SSL, m_BIO, m_BIO);

    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Server::Accept()
{
    SHOWFUNC("COpenSSL_Server")

    SHOW("  - SSL_accept")
    if (SSL_accept(m_SSL) <= 0)
        return SetLastError("SSL_accept failed.");
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Server::Write(std::string_view msg)
{
    SHOWFUNC("COpenSSL_Server")

    SHOW("  - SSL_write")
    if (SSL_write(m_SSL, msg.data(), msg.size()) == 0)
        return SetLastError("SSL_accept failed.");
    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_Server::SetLastError(
    std::string_view caption, std::string_view msg)
{
    m_LastError.Caption = caption;
    m_LastError.Message = msg;
    return false;
}
//----------------------------------------------------------------------------

}

