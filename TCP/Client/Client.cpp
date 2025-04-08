//----------------------------------------------------------------------------
//
// Module: Client
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#pragma hdrstop

#include "Client.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <stdio.h>
//---------------------------------------------------------------------------

#pragma package(smart_init)

#define DEFAULT_PORT "10001"
//#define DEFAULT_PORT "12300"
#define DEFAULT_BUFLEN 512

//#define IP_ADDR "127.0.0.1"
#define IP_ADDR "172.20.221.88"

TMemo* g_Memo;

//***************************************************************************
//
// class CTcpClient
// ----- ----------
//***************************************************************************

CTcpClient::~CTcpClient()
{
    if (m_AddrInfo)
        freeaddrinfo(m_AddrInfo);
    if (m_Socket != INVALID_SOCKET)
        closesocket(m_Socket);
    WSACleanup();
}
//----------------------------------------------------------------------------

bool CTcpClient::Initialize()
{
    if (!startup())
        return false;
    return getAddrInfo();
}
//----------------------------------------------------------------------------

bool CTcpClient::startup()
{
    WSADATA wsaData;

    // Initialize Winsock
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result == 0)
        return true;

    char buf[64];
    sprintf(buf, "WSAStartup failed: %d", result);
    return SetLastError(buf);
}
//----------------------------------------------------------------------------

bool CTcpClient::getAddrInfo()
{
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    int result = getaddrinfo(IP_ADDR, DEFAULT_PORT, &hints, &m_AddrInfo);
    if (result == 0)
        return true;

    char buf[64];
    sprintf(buf, "getaddrinfo failed: %d", result);
    return SetLastError(buf);
}
//----------------------------------------------------------------------------

bool CTcpClient::Shutdown()
{
    if (shutdown(m_Socket, SD_SEND) != SOCKET_ERROR)
        return true;

    char buf[64];
    sprintf(buf, "shutdown failed: %d", WSAGetLastError());
    return SetLastError(buf);
}
//----------------------------------------------------------------------------

bool CTcpClient::Connect()
{
    // Attempt to connect to the first address returned by
    // the call to getaddrinfo
    struct addrinfo* ptr = m_AddrInfo;

    // Create a SOCKET for connecting to server
    m_Socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (m_Socket == INVALID_SOCKET)
    {
        char buf[64];
        sprintf(buf, "Error at socket(): %ld", WSAGetLastError());
        m_LastError = buf;
        return SetLastError(buf);
    }

    // Connect to server.
    int result = connect(m_Socket, ptr->ai_addr, ptr->ai_addrlen);
    if (result == SOCKET_ERROR)
    {
        m_Socket = INVALID_SOCKET;
        return SetLastError("Unable to connect to server");
    }

    // Should really try the next address returned by getaddrinfo
    // if the connect call failed
    // But for this simple example we just free the resources
    // returned by getaddrinfo and print an error message

    if (m_Socket == INVALID_SOCKET)
    {
        m_LastError = "Invalid socket";
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------

bool CTcpClient::Send(std::string_view msg)
{
    // Send an initial buffer
    int result = send(m_Socket, msg.data(), msg.size(), 0);
    if (result != SOCKET_ERROR)
        return true;

    char buf[64];
    sprintf(buf, "send failed: %d", WSAGetLastError());
    return SetLastError(buf);
}
//----------------------------------------------------------------------------

bool CTcpClient::Receive()
{
    int recvbuflen = DEFAULT_BUFLEN;
    char recvbuf[DEFAULT_BUFLEN];

    int result = 0;

    // Receive data until the server closes the connection
    do {
        result = recv(m_Socket, recvbuf, recvbuflen, 0);
        if (result > 0)
        {
            char buf[64];
            sprintf(buf, "Bytes received: %d", result);
            g_Memo->Lines->Add(buf);

            recvbuf[recvbuflen] = 0;
            g_Memo->Lines->Add(result);
        }
        else if (result == 0)
        {
            g_Memo->Lines->Add("Connection closed");
        }
        else
        {
            char buf[64];
            sprintf(buf, "recv failed: %d", WSAGetLastError());
            return SetLastError(buf);
        }
    } while (result > 0);

    return true;
}
//----------------------------------------------------------------------------

bool CTcpClient::SetLastError(std::string_view error)
{
    m_HasError = true;
    m_LastError = error;
    return false;
}
//----------------------------------------------------------------------------

