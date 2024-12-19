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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <thread>

#include "main.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"

#pragma comment(lib, "Ws2_32.lib")

TformMain *formMain;
TMemo* g_Memo {};

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

#define IP_ADDR "127.0.0.1"


//***************************************************************************
//
// class CListenerThread
// ----- ---------------
//***************************************************************************

/*!
 */

class CListenerThread {
  public:
    CListenerThread() {}

    void operator()(SOCKET m_Socket);

  private:
    SOCKET m_Socket { INVALID_SOCKET };
};


void CListenerThread::operator()(SOCKET m_Socket)
{
    int recvbuflen = DEFAULT_BUFLEN;
    char recvbuf[DEFAULT_BUFLEN];
    int result = 0;

    // Receive until the peer shuts down the connection
    do {
        int result = recv(m_Socket, recvbuf, recvbuflen, 0);

        if (result > 0)
        {
            g_Memo->Lines->Add(AnsiString().sprintf("Bytes received: %d", result));
            g_Memo->Lines->Add(recvbuf);
        }

        else if (result == 0)
        {
            g_Memo->Lines->Add("Connection closing...");
        }

        else
        {
            char buf[64];
            sprintf(buf, "recv failed: %d", WSAGetLastError());
            g_Memo->Lines->Add(buf);
            return;
        }

    } while (result > 0);
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class CTcpServer
// ----- -----------
//***************************************************************************

/*!
 */

class CTcpServer {
  public:
    ~CTcpServer();

    ///
    bool Initialize();

    ///
    bool Connect();

    ///
    bool Accept();

    ///
    bool Receive();

    ///
    bool HasError() const
        { return m_HasError; }

    ///
    std::string GetLastError() const
        { return m_LastError; }

  private:
    struct addrinfo* m_AddrInfo = nullptr;

    SOCKET m_ListenSocket { INVALID_SOCKET };
    SOCKET m_ClientSocket { INVALID_SOCKET };

    bool m_HasError {};
    std::string m_LastError {};

    bool startup();
    bool getAddrInfo();

    bool SetLastError(std::string_view error);
};


CTcpServer::~CTcpServer()
{
    if (m_AddrInfo)
        freeaddrinfo(m_AddrInfo);
    if (m_ListenSocket != INVALID_SOCKET)
        closesocket(m_ListenSocket);
    if (m_ClientSocket != INVALID_SOCKET)
        closesocket(m_ClientSocket);
    WSACleanup();
    g_Memo->Lines->Add("Connection closed");
}
//----------------------------------------------------------------------------

bool CTcpServer::Initialize()
{
    if (!startup())
        return false;
    return getAddrInfo();
}
//----------------------------------------------------------------------------

bool CTcpServer::startup()
{
    WSADATA wsaData;

    // Initialize Winsock
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result == 0)
        return true;

    char buf[64];
    sprintf(buf, "WSAStartup failed: %d", result);
    m_LastError = buf;
    return false;
}
//----------------------------------------------------------------------------

bool CTcpServer::getAddrInfo()
{
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    int result = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &m_AddrInfo);
    if (result == 0)
        return true;

    char buf[64];
    sprintf(buf, "getaddrinfo failed: %d", result);
    return SetLastError(buf);
}
//----------------------------------------------------------------------------

bool CTcpServer::Connect()
{
    // Create a SOCKET for the server to listen for client connections
    m_ListenSocket = socket(
        m_AddrInfo->ai_family, m_AddrInfo->ai_socktype,
        m_AddrInfo->ai_protocol);

    if (m_ListenSocket == INVALID_SOCKET)
    {
        char buf[64];
        sprintf(buf, "Error at socket(): %d", WSAGetLastError());
        return SetLastError(buf);
    }

    // Setup the TCP listening socket
    int result = bind(
        m_ListenSocket, m_AddrInfo->ai_addr, m_AddrInfo->ai_addrlen);
    if (result == SOCKET_ERROR)
    {
        char buf[64];
        sprintf(buf, "bind failed with error: %d", WSAGetLastError());
        return SetLastError(buf);
    }

    if (listen(m_ListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        char buf[64];
        sprintf(buf, "Listen failed with error: %d", WSAGetLastError());
        return SetLastError(buf);
    }

    return true;
}
//----------------------------------------------------------------------------

bool CTcpServer::Accept()
{
    // Accept a client socket
    m_ClientSocket = accept(m_ListenSocket, nullptr, nullptr);
    if (m_ClientSocket != INVALID_SOCKET)
        return true;

    char buf[64];
    sprintf(buf, "accept failed: %d", WSAGetLastError());
    return SetLastError(buf);
}
//----------------------------------------------------------------------------

bool CTcpServer::Receive()
{
//    std::thread listener(CListenerThread(), m_ClientSocket);
//
//    listener.join();
//    g_Memo->Lines->Add("joined");


    char recvbuf[DEFAULT_BUFLEN];
    int result = 0;
    int sendResult = 0;
    int recvbuflen = DEFAULT_BUFLEN;

    // Receive until the peer shuts down the connection
    do {

        result = recv(m_ClientSocket, recvbuf, recvbuflen, 0);

        if (result > 0)
        {
            g_Memo->Lines->Add(AnsiString().sprintf("Bytes received: %d", result));
            g_Memo->Lines->Add(recvbuf);

            // Echo the buffer back to the sender
            sendResult = send(m_ClientSocket, recvbuf, result, 0);
            if (sendResult == SOCKET_ERROR)
            {
                char buf[64];
                sprintf(buf, "send failed: %d", WSAGetLastError());
                return SetLastError(buf);
            }

            g_Memo->Lines->Add(AnsiString().sprintf("Bytes sent: %d", sendResult));
        }

        else if (result == 0)
        {
            g_Memo->Lines->Add("Connection closing...");
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

bool CTcpServer::SetLastError(std::string_view error)
{
    m_HasError = true;
    m_LastError = error;
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

void __fastcall TformMain::butListenClick(TObject *Sender)
{
    CTcpServer server;

    if (!server.Initialize())
    {
        memo->Lines->Add(server.GetLastError().c_str());
        return;
    }

    if (!server.Connect())
    {
        memo->Lines->Add(server.GetLastError().c_str());
        return;
    }

    memo->Lines->Add("Listening...");

    if (!server.Accept())
    {
        memo->Lines->Add(server.GetLastError().c_str());
        return;
    }

    if (!server.Receive())
    {
        if (server.HasError())
            memo->Lines->Add(server.GetLastError().c_str());
        return;
    }
}
//---------------------------------------------------------------------------

