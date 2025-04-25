//----------------------------------------------------------------------------
//
// Module: Main
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Main_Client.h"

#include "CriticalSection.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"
TformMain *formMain;

const int PORTNO = 12350;
const char* ADDRESS = "127.0.0.1";

const char* CERTIFICATE = "certs/hedsam.crt";

//***************************************************************************
//
// class CClientThread
// ----- --------------
//***************************************************************************

CClientThread::CClientThread(ztls::COpenSSL_Client* client, CNewMessage cb)
  : TThread(false)
  , m_CriticalSection()
  , m_Client(client)
  , m_NewMessageCb(cb)
{
    InitializeCriticalSection(&m_CriticalSection);
}
//----------------------------------------------------------------------------

__fastcall CClientThread::~CClientThread()
{
    DeleteCriticalSection(&m_CriticalSection);
}
//----------------------------------------------------------------------------

bool __fastcall CClientThread::Empty()
{
    zutl::CCriticalSection cs(&m_CriticalSection);
    return m_Deque.empty();
}
//----------------------------------------------------------------------------

std::string __fastcall CClientThread::Fetch()
{
    zutl::CCriticalSection cs(&m_CriticalSection);

    if (m_Deque.empty())
        return std::string();

    std::string s = m_Deque.front();
    m_Deque.pop_front();
    return s;
}
//----------------------------------------------------------------------------

void __fastcall CClientThread::Execute()
{
    while (!Terminated)
    {
        std::string message;
        if (!m_Client->Read(message))
            return;

        m_Deque.push_back(message);

        if (m_NewMessageCb)
            m_NewMessageCb();

        Sleep(100);
    }
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
    m_TcpClient = new ztls::CTcpClient;
    m_TcpClient->SetAddress(ADDRESS);
    m_TcpClient->SetPortNo(PORTNO);

    m_SslClient = new ztls::COpenSSL_Client;
    m_SslClient->SetCertificate(CERTIFICATE);
}
//---------------------------------------------------------------------------

__fastcall TformMain::~TformMain()
{
    delete m_ClientThread;
    delete m_SslClient;
    delete m_TcpClient;
}
//----------------------------------------------------------------------------

void __fastcall TformMain::FormShow(TObject *Sender)
{
//
}
//---------------------------------------------------------------------------

void __fastcall TformMain::butConnectClick(TObject *Sender)
{
    if (m_Connected)
        Disconnect();
    else
        Connect();
}
//---------------------------------------------------------------------------

void __fastcall TformMain::butSendClick(TObject *Sender)
{
    if (!m_Connected)
        return;

    if (!m_SslClient->Write("Hello from the client!"))
        ShowError(m_SslClient->GetLastError());
}
//---------------------------------------------------------------------------

void __fastcall TformMain::timerTimer(TObject *Sender)
{
    if (!m_Connected)
        return;

    if (std::exchange(m_NewMessage, false))
    {
        if (m_ClientThread->Empty())
            return;

        std::string message = m_ClientThread->Fetch();

        memo->Lines->Add(message.c_str());
    }
}
//---------------------------------------------------------------------------

bool __fastcall TformMain::Connect()
{
    memo->Lines->Add("### Connect");

    if (!m_TcpClient->Connect())
        return ShowError(m_TcpClient->GetLastError());

    if (!m_SslClient->CreateContext())
        return ShowError(m_SslClient->GetLastError());

    if (!m_SslClient->SetVersions())
        return ShowError(m_SslClient->GetLastError());

    if (!m_SslClient->CreateSSL(m_TcpClient->ServerSocket()))
        return ShowError(m_SslClient->GetLastError());

    if (!m_SslClient->Connect())
        return ShowError(m_SslClient->GetLastError());

    if (!m_SslClient->LoadVerifyLocations())
        return ShowError(m_SslClient->GetLastError());

//    if (!sslClient.VerifyCertification())
//        return showError(sslClient.GetLastError());

//    if (!m_SslClient->MakeConnection(*m_TcpClient))
//        return ShowError(m_SslClient->GetLastError());

//    m_TcpClient->Disconnect();

    // jos thread oli olemassa, tuhotaan se ensin
    if (m_ClientThread)
    {
        delete m_ClientThread;
        m_ClientThread = nullptr;
    }

    // luodaan thread lukemaan vaataanotettua dataa
    m_ClientThread = new CClientThread(m_SslClient, OnNewMessage);

    timer->Enabled = true;

    m_Connected = true;
    butConnect->Caption = "Disconnect";

    return true;
}
//----------------------------------------------------------------------------

void __fastcall TformMain::Disconnect()
{
    memo->Lines->Add("### Disconnect");

    timer->Enabled = false;

    // terminoidaan thread
    if (m_ClientThread)
        m_ClientThread->Terminate();

    // odotetaan thread:in päättymistä
    if (m_ClientThread)
    {
        m_ClientThread->WaitFor();
        delete m_ClientThread;
        m_ClientThread = nullptr;
    }

    m_Connected = false;
    butConnect->Caption = "Connect";
}
//----------------------------------------------------------------------------

bool __fastcall TformMain::ShowError(const ztls::CError& error)
{
    memo->Lines->Add(error.Caption.c_str());
    memo->Lines->Add(error.Message.c_str());
    return false;
}
//----------------------------------------------------------------------------

void __fastcall TformMain::OnNewMessage()
{
    m_NewMessage = true;
}
//----------------------------------------------------------------------------


