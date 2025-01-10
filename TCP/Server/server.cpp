//----------------------------------------------------------------------------
//
// Module: server
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#pragma hdrstop

#include "server.h"

#include <thread>
#include <chrono>
//---------------------------------------------------------------------------
#pragma package(smart_init)

TMemo* g_Memo {};

//***************************************************************************
//
// class CThread
// ----- -------
//***************************************************************************

CThread::~CThread()
{
    if (m_Thread->joinable())
        m_Thread->join();
    delete m_Thread;
}
//----------------------------------------------------------------------------

void CThread::Execute()
{
    if (m_Thread)
        return;
    m_Thread = new std::thread(Run);
    m_Terminated = false;
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class CListenerThread
// ----- ---------------
//***************************************************************************

void CListenerThread::Run()
{
    int result = 0;

    while (!Terminated())
    {
        std::array<char, DEFAULT_BUFLEN> arr;

        result = recv(*m_Socket, arr.data(), arr.size(), 0);

        if (result > 0)
        {
            m_Data.clear();
            for (int i=0; i < result; ++i)
                m_Data.push_back(arr[i]);
            m_DataReceived = true;
            if (m_ReceivedCallback)
                m_ReceivedCallback(m_OwnerThis);
        }

        else if (result == 0)
        {
            Terminate();
        }

        else
        {
            Terminate();
        }
    }

    if (result == 0)
    {
        if (m_TerminatedCallback)
            m_TerminatedCallback(m_OwnerThis);
    }
    else
    {
        if (m_ErrorCallback)
            m_ErrorCallback(m_OwnerThis);
    }
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class CTcpServer
// ----- ----------
//***************************************************************************

CTcpServer::~CTcpServer()
{
    Cleanup();
    g_Memo->Lines->Add("Connection closed");
}
//----------------------------------------------------------------------------

bool CTcpServer::Initialize()
{
    if (m_PortNo == 0)
        return false;
    if (!startup())
        return false;
    return getAddrInfo(m_PortNo);
}
//----------------------------------------------------------------------------

void CTcpServer::Cleanup()
{
    delete m_ListenerThread;
    m_ListenerThread = nullptr;

    if (m_AddrInfo)
        freeaddrinfo(m_AddrInfo);
    if (m_ListenSocket != INVALID_SOCKET)
        closesocket(m_ListenSocket);
    if (m_ClientSocket != INVALID_SOCKET)
        closesocket(m_ClientSocket);

    WSACleanup();
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

bool CTcpServer::getAddrInfo(int port)
{
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    std::string s = std::to_string(port);

    // Resolve the local address and port to be used by the server
    int result = getaddrinfo(nullptr, s.c_str(), &hints, &m_AddrInfo);
    if (result == 0)
        return true;

    char buf[64];
    sprintf(buf, "getaddrinfo failed: %d", result);
    return SetLastError(buf);
}
//----------------------------------------------------------------------------

bool CTcpServer::Listen()
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
        sprintf(buf, "listen failed with error: %d", WSAGetLastError());
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
    {
        g_Memo->Lines->Add("accepted");
        return true;
    }

    char buf[64];
    sprintf(buf, "accept failed: %d", WSAGetLastError());
    return SetLastError(buf);
}
//----------------------------------------------------------------------------

bool CTcpServer::Receive()
{
    g_Memo->Lines->Add("Receiving");

    if (!m_ListenerThread)
        m_ListenerThread = new CListenerThread(&m_ClientSocket);

    m_ListenerThread->SetReceivedCallback(
        this, OnDataReceived, OnTerminated, OnError);
    m_ListenerThread->Execute();

    m_Connected = true;

    return true;
}
//----------------------------------------------------------------------------

bool CTcpServer::Connect()
{
    if (!Initialize())
        return false;
    if (!Listen())
        return false;
    if (!Accept())
        return false;
    if (!Receive())
        return false;
    return true;
}
//----------------------------------------------------------------------------

void CTcpServer::Terminate()
{
    m_Connected = false;
}
//----------------------------------------------------------------------------

bool CTcpServer::SetLastError(std::string_view error)
{
    m_HasError = true;
    m_LastError = error;
    return false;
}
//----------------------------------------------------------------------------

void CTcpServer::OnDataReceived(void* this_)
{
    auto This = static_cast<CTcpServer*>(this_);
    if (This)
    {
        This->SetDataReceived();
        This->SetReceivedData(This->ListenerThread()->Data());
    }
}
//----------------------------------------------------------------------------

void CTcpServer::OnTerminated(void* this_)
{
    if (auto This = static_cast<CTcpServer*>(this_))
        This->Terminate();
}
//----------------------------------------------------------------------------

void CTcpServer::OnError(void* this_)
{
    if (auto This = static_cast<CTcpServer*>(this_))
        ;
}
//----------------------------------------------------------------------------


