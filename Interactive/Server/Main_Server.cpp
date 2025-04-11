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

#include <deque>
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"
TformMain *formMain;

const int PORTNO = 12350;

const char* PRIVATEKEY = "certs/hedsam.key";
const char* CERTIFICATE = "certs/hedsam.crt";

//***************************************************************************
//
// class CServerThread
// ----- --------------
//***************************************************************************

CServerThread::CServerThread(ztls::CTcpServer* server, CAccepted cb)
  : TThread(false), m_Server(server), m_AcceptedCb(cb)
{
}
//----------------------------------------------------------------------------

void __fastcall CServerThread::Execute()
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
// class TformMain
// ----- ---------
//***************************************************************************

__fastcall TformMain::TformMain(TComponent* Owner)
  : TForm(Owner)
{
    m_TcpServer = new ztls::CTcpServer;
    m_TcpServer->SetPortNo(PORTNO);

    m_SslServer = new ztls::COpenSSL_Server;
    m_SslServer->SetPrivateKey(PRIVATEKEY);
    m_SslServer->SetCertificate(CERTIFICATE);
}
//---------------------------------------------------------------------------

__fastcall TformMain::~TformMain()
{
    Disconnect();

    delete m_ServerThread;
    delete m_TcpServer;
    delete m_SslServer;
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
//
}
//---------------------------------------------------------------------------

void __fastcall TformMain::timerTimer(TObject *Sender)
{
    if (std::exchange(m_Accepted, false))
    {
        if (!m_SslServer->CreateSSL(m_TcpServer->ClientSocket()))
        {
            ShowError(m_SslServer->GetLastError());
        }
        else if (!m_SslServer->Accept())
        {
            ShowError(m_SslServer->GetLastError());
        }
        else
        {
            m_SslServer->Write("Hello, SSL/TLS world!");
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

    // jos thread oli olemassa, tuhotaan se ensin
    if (m_ServerThread)
    {
        delete m_ServerThread;
        m_ServerThread = nullptr;
    }

    // luodaan thread lukemaan vaataanotettua dataa
    m_ServerThread = new CServerThread(m_TcpServer, OnAccepted);

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

    //
    m_TcpServer->Shutdown();

    // terminoidaan thread
    if (m_ServerThread)
        m_ServerThread->Terminate();

    // odotetaan thread:in päättymistä
    if (m_ServerThread)
    {
        m_ServerThread->WaitFor();
        delete m_ServerThread;
        m_ServerThread = nullptr;
    }

    if (!m_SslServer->Shutdown())
        ShowError(m_SslServer->GetLastError());

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

