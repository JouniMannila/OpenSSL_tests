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
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"
TformMain *formMain;

const int PORTNO = 12350;
const char* ADDRESS = "127.0.0.1";

const char* CERTIFICATE = "certs/hedsam.crt";

//***************************************************************************
//
// class CTcpReadThread
// ----- --------------
//***************************************************************************

CTcpReadThread::CTcpReadThread(ztls::CTcpClient* client)
  : TThread(false)
//  , m_CriticalSection()
  , m_Client(client)
{
//    InitializeCriticalSection(&m_CriticalSection);
}
//----------------------------------------------------------------------------

__fastcall CTcpReadThread::~CTcpReadThread()
{
//    DeleteCriticalSection(&m_CriticalSection);
}
//----------------------------------------------------------------------------

bool __fastcall CTcpReadThread::Empty()
{
//    zutl::CCriticalSection cs(&m_CriticalSection);
    return m_Deque.empty();
}
//----------------------------------------------------------------------------

std::string __fastcall CTcpReadThread::Fetch()
{
//    zutl::CCriticalSection cs(&m_CriticalSection);

    if (m_Deque.empty())
        return std::string();

    std::string s = m_Deque.front();
    m_Deque.pop_front();
    return s;
}
//----------------------------------------------------------------------------

void __fastcall CTcpReadThread::Execute()
{
    while (!Terminated)
    {
//        try
//        {
//            AnsiString S = m_Client->IOHandler->ReadLn();
//
////            zutl::CCriticalSection cs(&m_CriticalSection);
//            m_Deque.push_back(S.c_str());
//        }
//
//        catch (EIdClosedSocket& e) {
//            OutputDebugString("EIdClosedSocket&");
//            Terminate();
//        }
//
//        catch (EIdNotConnected& e) {
//            OutputDebugString("EIdNotConnected");
//            Terminate();
//        }
//
//        catch (EIdException& e) {
//            OutputDebugString("EIdException");
//            Terminate();
//        }

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
    char buffer[1024];
    SHOW("  - SSL_read")
    int bytes = SSL_read(m_SSL, buffer, sizeof(buffer));
    if (bytes <= 0)
        return false;

    buffer[bytes] = 0;
    message = buffer;
}
//---------------------------------------------------------------------------

bool __fastcall TformMain::Connect()
{
    if (!m_TcpClient->Connect())
        return ShowError(m_TcpClient->GetLastError());

    if (!m_SslClient->MakeConnection(*m_TcpClient))
        return ShowError(m_SslClient->GetLastError());

    timer->Enabled = true;

    m_Connected = true;
    butConnect->Caption = "Disconnect";

    return true;
}
//----------------------------------------------------------------------------

void __fastcall TformMain::Disconnect()
{
    timer->Enabled = false;

    m_Connected = false;
    butConnect->Caption = "Connect";
}
//----------------------------------------------------------------------------

bool __fastcall TformMain::ShowError(const ztls::CError& error)
{
    memo->Lines->Add(error.Caption.c_str());
    return false;
}
//----------------------------------------------------------------------------

