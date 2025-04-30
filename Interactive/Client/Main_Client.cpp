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

#include "Error.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"
TformMain *formMain;

const int PORTNO = 12350;
const char* ADDRESS = "127.0.0.1";

const char* CERTIFICATE = "certs/hedsam.crt";


#ifdef CONSOLE

  #define SHOWFUNC(class)  std::cout << class << "::" << __FUNC__ << std::endl;
  #define SHOW(text)  std::cout << text << std::endl;

#else

  #define SHOWFUNC(class) \
    if (g_MemoWriter.Func) \
      g_MemoWriter.Func( \
        g_MemoWriter.This, std::string(class) + "::" + std::string(__FUNC__));

  #define SHOW(text) \
    if (g_MemoWriter.Func) \
      g_MemoWriter.Func(g_MemoWriter.This, text);

#endif


//***************************************************************************
//
// class CClientThread
// ----- -------------
//***************************************************************************

CClientThread::CClientThread(
    ztls::COpenSSL_Client* client, CNewMessage messageCb, CCloseNotify closeCb)
  : TThread(false)
  , m_Client(client)
  , m_NewMessageCb(messageCb)
  , m_CloseNotifyCb(closeCb)
{
}
//----------------------------------------------------------------------------

bool __fastcall CClientThread::Empty()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Deque.empty();
}
//----------------------------------------------------------------------------

std::string __fastcall CClientThread::Fetch()
{
    std::lock_guard<std::mutex> lock(m_Mutex);

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
        int bytes = m_Client->Read(message);

        if (bytes <= 0)
        {
            int err = SSL_get_error(m_Client->GetSSL(), bytes);
            if (err == SSL_ERROR_ZERO_RETURN) // close_notify
            {
                if (m_CloseNotifyCb)
                    m_CloseNotifyCb();
            }
            else if (err == SSL_ERROR_SYSCALL)
            {
                // timeout or network error
            }
            else if (err == SSL_ERROR_WANT_READ)
            {
            }
            else if (err == SSL_ERROR_WANT_WRITE)
            {
            }
            else
            {
                std::string s =
                    ztls::CSSL_GetError::Reason(m_Client->GetSSL(), bytes);
            }
        }

        else
        {
            Push(message);
            if (m_NewMessageCb)
                m_NewMessageCb();
        }

        Sleep(100);
    }
}
//----------------------------------------------------------------------------

void CClientThread::Push(const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Deque.push_back(message);
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
    g_MemoWriter.Func = MemoWriter;
    g_MemoWriter.This = this;

    m_TcpClient = std::make_unique<ztls::CTcpClient>(ADDRESS, PORTNO);

    m_SslClient = std::make_unique<ztls::COpenSSL_Client>();
    m_SslClient->SetCertificate(CERTIFICATE);
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
    if (!m_Connected)
        return;

    if (ztls::CTlsResult r = m_SslClient->Write("Hello from the client!"); !r)
        ShowError(r);
}
//---------------------------------------------------------------------------

void __fastcall TformMain::timerTimer(TObject *Sender)
{
    if (!m_Connected)
        return;

    if (std::exchange(m_CloseNotified, false))
    {
        memo->Lines->Add("### Close notified");
        Disconnect();
    }

    if (std::exchange(m_NewMessage, false))
    {
        if (m_ClientThread->Empty())
            return;

        memo->Lines->Add("### New Message");
        std::string message = m_ClientThread->Fetch();

        memo->Lines->Add(AnsiString("->") + message.c_str());
    }
}
//---------------------------------------------------------------------------

bool __fastcall TformMain::Connect()
{
    using namespace ztls;

    memo->Lines->Add("### Connect");

    if (CTlsResult r = m_TcpClient->Connect(); !r)
        return ShowError(r);

    m_TcpClient->SetReadTimeout(5000);

    if (CTlsResult r = m_SslClient->CreateContext(); !r)
        return ShowError(r);

    if (CTlsResult r = m_SslClient->SetVersions(); !r)
        return ShowError(r);

    if (CTlsResult r = m_SslClient->CreateSSL(m_TcpClient->Socket()); !r)
        return ShowError(r);

    if (CTlsResult r = m_SslClient->Connect(); !r)
        return ShowError(r);

    if (CTlsResult r = m_SslClient->LoadVerifyLocations(); !r)
        return ShowError(r);

//    if (CTlsResult r = sslClient.VerifyCertification(); !r)
//        return showError(r);

//    if (CTlsResult r = m_SslClient->MakeConnection(*m_TcpClient); !r)
//        return ShowError(r);

    // luodaan thread lukemaan vaataanotettua dataa
    m_ClientThread = std::make_unique<CClientThread>(
        m_SslClient.get(), OnNewMessage, OnCloseNotify);

    timer->Enabled = true;

    m_Connected = true;
    butConnect->Caption = "Disconnect";

    return true;
}
//----------------------------------------------------------------------------

void __fastcall TformMain::Disconnect()
{
    using namespace ztls;

    if (!m_Connected)
        return;

    memo->Lines->Add("### Disconnect");

    timer->Enabled = false;

    if (CTlsResult r = m_SslClient->Shutdown(); !r)
        ShowError(r);

    m_TcpClient->Disconnect();
    m_SslClient->Free();

    if (m_ClientThread)
    {
        // terminoidaan thread
        m_ClientThread->Terminate();

        // odotetaan thredin päättymistä
        m_ClientThread->WaitFor();
    }

    m_Connected = false;
    butConnect->Caption = "Connect";
}
//----------------------------------------------------------------------------

bool __fastcall TformMain::ShowError(const ztls::CTlsResult& result)
{
    memo->Lines->Add(ztls::TlsResultText(result.ErrCode()).c_str());
    memo->Lines->Add(result.Message().c_str());
    return false;
}
//----------------------------------------------------------------------------

void __fastcall TformMain::OnNewMessage()
{
    m_NewMessage = true;
}
//----------------------------------------------------------------------------

void __fastcall TformMain::OnCloseNotify()
{
    m_CloseNotified = true;
}
//----------------------------------------------------------------------------

void TformMain::MemoWriter(void* _this, const std::string& text)
{
    reinterpret_cast<TformMain*>(_this)->memo->Lines->Add(text.c_str());
}
//----------------------------------------------------------------------------

