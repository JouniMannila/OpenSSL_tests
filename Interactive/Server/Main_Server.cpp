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

#include "Main_Server.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"
TformMain *formMain;

const int PORTNO = 12350;

const char* PRIVATEKEY = "certs/hedsam.key";
const char* CERTIFICATE = "certs/hedsam.crt";

//***************************************************************************
//
// class CAcceptThread
// ----- -------------
//***************************************************************************

CAcceptThread::CAcceptThread(ztls::CTcpServer* server, CAccepted cb)
  : TThread(false), m_Server(server), m_AcceptedCb(cb)
{
}
//----------------------------------------------------------------------------

void __fastcall CAcceptThread::Execute()
{
    while (!Terminated)
    {
        if (!m_Server->Accept())
            return;   // virhe

        if (m_AcceptedCb)
            m_AcceptedCb();

        Sleep(100);
    }
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class CReadThread
// ----- -----------
//***************************************************************************

CReadThread::CReadThread(ztls::COpenSSL_Server* server, CNewData cb)
  : TThread(false), m_Server(server), m_NewDataCb(cb)
{
}
//----------------------------------------------------------------------------

bool __fastcall CReadThread::Empty()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Deque.empty();
}
//----------------------------------------------------------------------------

std::string __fastcall CReadThread::Fetch()
{
    const std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_Deque.empty())
        return std::string();

    std::string s = m_Deque.front();
    m_Deque.pop_front();
    return s;
}
//----------------------------------------------------------------------------

void __fastcall CReadThread::Execute()
{
    while (!Terminated)
    {
        std::string s;
        if (m_Server->Read(s))
        {
            Push(s);

            if (m_NewDataCb)
                m_NewDataCb();
        }

        Sleep(100);
    }
}
//----------------------------------------------------------------------------

void __fastcall CReadThread::Push(const std::string& s)
{
    const std::lock_guard<std::mutex> lock(m_Mutex);
    m_Deque.push_back(s);
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
    m_TcpServer = std::make_unique<ztls::CTcpServer>();
    m_TcpServer->SetPortNo(PORTNO);

    m_SslServer = std::make_unique<ztls::COpenSSL_Server>();
    m_SslServer->SetPrivateKey(PRIVATEKEY);
    m_SslServer->SetCertificate(CERTIFICATE);
}
//---------------------------------------------------------------------------

__fastcall TformMain::~TformMain()
{
    Disconnect();
}
//----------------------------------------------------------------------------

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
    if (m_ClientConnected)
    {
        if (!m_SslServer->Write("Hello from server."))
        {
            m_ClientConnected = false;
            ShowError(m_SslServer->GetLastError());
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TformMain::timerTimer(TObject *Sender)
{
    if (std::exchange(m_Accepted, false))
    {
        if (!m_SslServer->CreateSSL(m_TcpServer->ClientSocket()))
        {
            m_ClientConnected = false;
            ShowError(m_SslServer->GetLastError());
        }
        else if (!m_SslServer->Accept())
        {
            m_ClientConnected = false;
            ShowError(m_SslServer->GetLastError());
        }
        else
        {
            m_SslServer->Write("Hello, SSL/TLS world!");
            m_ClientConnected = true;
        }
    }

    else if (m_ClientConnected)
    {
        if (std::exchange(m_NewData, false))
        {
            if (!m_ReadThread->Empty())
            {
                std::string s = m_ReadThread->Fetch();
                memo->Lines->Add(s.c_str());
            }
        }
    }
}
//---------------------------------------------------------------------------

bool __fastcall TformMain::Connect()
{
    memo->Lines->Add("### Connect");

    m_TcpServer->Initialize();
    m_SslServer->Initialize();

    if (!m_SslServer->CreateContext())
        return ShowError(m_TcpServer->GetLastError());

    if (!m_SslServer->ConfigureContext())
        return ShowError(m_SslServer->GetLastError());

    if (!m_TcpServer->Connect())
        return ShowError(m_TcpServer->GetLastError());

    // luodaan thread ...
    m_AcceptThread =
        std::make_unique<CAcceptThread>(m_TcpServer.get(), OnAccepted);

    // luodaan thread lukemaan vastaanotettua dataa
    m_ReadThread = std::make_unique<CReadThread>(m_SslServer.get(), OnNewData);

    timer->Enabled = true;

    m_Connected = true;
    butConnect->Caption = "Disconnect";

    return true;
}
//----------------------------------------------------------------------------

void __fastcall TformMain::Disconnect()
{
    if (!m_Connected)
        return;

    memo->Lines->Add("### Disconnect");

    timer->Enabled = false;

    if (!m_SslServer->Shutdown())
        ShowError(m_SslServer->GetLastError());

    m_SslServer->Free();

    //
    m_TcpServer->Shutdown();

    // terminoidaan thread
    if (m_AcceptThread)
        m_AcceptThread->Terminate();

    // terminoidaan thread
    if (m_ReadThread)
        m_ReadThread->Terminate();

    // odotetaan thread:in päättymistä
    if (m_AcceptThread)
    {
        m_AcceptThread->WaitFor();
        m_AcceptThread = nullptr;
    }

    // odotetaan thread:in päättymistä
    if (m_ReadThread)
    {
        m_ReadThread->WaitFor();
        m_ReadThread = nullptr;
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

void __fastcall TformMain::OnAccepted()
{
    m_Accepted = true;
}
//----------------------------------------------------------------------------

void __fastcall TformMain::OnNewData()
{
    m_NewData = true;
}
//----------------------------------------------------------------------------

