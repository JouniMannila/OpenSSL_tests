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


////***************************************************************************
////
//// class CClientThread
//// ----- -------------
////***************************************************************************
//
//CClientThread::CClientThread(
//    ztls::COpenSSL_Client* client, CNewMessage messageCb, CCloseNotify closeCb)
//  : TThread(false)
//  , m_Client(client)
//  , m_NewMessageCb(messageCb)
//  , m_CloseNotifyCb(closeCb)
//{
//}
////----------------------------------------------------------------------------
//
//bool __fastcall CClientThread::Empty()
//{
//    std::lock_guard<std::mutex> lock(m_Mutex);
//    return m_Deque.empty();
//}
////----------------------------------------------------------------------------
//
//std::string __fastcall CClientThread::Fetch()
//{
//    std::lock_guard<std::mutex> lock(m_Mutex);
//
//    if (m_Deque.empty())
//        return std::string();
//
//    std::string s = m_Deque.front();
//    m_Deque.pop_front();
//    return s;
//}
////----------------------------------------------------------------------------
//
//void __fastcall CClientThread::Execute()
//{
//    while (!Terminated)
//    {
//        std::string message;
//        int bytes = m_Client->Read(message);
//
//        if (bytes <= 0)
//        {
//            int err = SSL_get_error(m_Client->GetSSL(), bytes);
//            if (err == SSL_ERROR_ZERO_RETURN) // close_notify
//            {
//                if (m_CloseNotifyCb)
//                    m_CloseNotifyCb();
//            }
//            else if (err == SSL_ERROR_SYSCALL)
//            {
//                // timeout or network error
//            }
//            else if (err == SSL_ERROR_WANT_READ)
//            {
//            }
//            else if (err == SSL_ERROR_WANT_WRITE)
//            {
//            }
//            else
//            {
//                std::string s =
//                    ztls::CSSL_GetError::Reason(m_Client->GetSSL(), bytes);
//            }
//        }
//
//        else
//        {
//            Push(message);
//            if (m_NewMessageCb)
//                m_NewMessageCb();
//        }
//
//        Sleep(100);
//    }
//}
////----------------------------------------------------------------------------
//
//void CClientThread::Push(const std::string& message)
//{
//    std::lock_guard<std::mutex> lock(m_Mutex);
//    m_Deque.push_back(message);
//}
////----------------------------------------------------------------------------


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

    m_TlsClient = std::make_unique<ztls::CTlsClient>(
        AnsiString(edAddress->Text).c_str(), udPort->Position, CERTIFICATE);
    m_TlsClient->SetNewMessageCallback(OnNewMessage);
    m_TlsClient->SetCloseNotifyCallback(OnCloseNotify);
    m_TlsClient->SetErrorCallback(OnError);

//    m_TcpClient = std::make_unique<ztls::CTcpClient>(ADDRESS, PORTNO);
//
//    m_SslClient = std::make_unique<ztls::COpenSSL_Client>();
//    m_SslClient->SetCertificate(CERTIFICATE);
}
//---------------------------------------------------------------------------

__fastcall TformMain::~TformMain()
{
//    Disconnect();
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

    if (ztls::CTlsResult r = m_TlsClient->Write("Hello from the client!\n\r"); !r)
    {
        ShowError(r);
        Disconnect();
    }
}
//---------------------------------------------------------------------------

void __fastcall TformMain::timerTimer(TObject *Sender)
{
    if (!m_Connected)
        return;

    if (std::exchange(m_IsError, false))
    {
        memo->Lines->Add("### Error");
        Disconnect();
        Connect();
    }

    else if (std::exchange(m_CloseNotified, false))
    {
        memo->Lines->Add("### Close notified");
//        Disconnect();
    }

    else if (std::exchange(m_NewMessage, false))
    {
        if (!m_TlsClient->HasMessages())
            return;

        while (m_TlsClient->HasMessages())
        {
            memo->Lines->Add("### New Message");
            std::string message = m_TlsClient->Fetch();
            memo->Lines->Add(AnsiString("->") + message.c_str());
        }
    }
}
//---------------------------------------------------------------------------

bool __fastcall TformMain::Connect()
{
    using namespace ztls;

    m_TlsClient->SetAddress(AnsiString(edAddress->Text).c_str());
    m_TlsClient->SetPortNo(udPort->Position);

    m_TlsClient->SetReadTimeout(5000);

    if (CTlsResult r = m_TlsClient->Connect(); !r)
        return ShowError(r);

    m_TlsClient->Write("\n\rHalloota!\n\r");

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

//    if (CTlsResult r = m_TlsClient->Disconnect(); !r)
//        ;

    timer->Enabled = false;

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

void TformMain::OnNewMessage()
{
    m_NewMessage = true;
}
//----------------------------------------------------------------------------

void TformMain::OnCloseNotify()
{
    m_CloseNotified = true;
}
//----------------------------------------------------------------------------

void TformMain::OnError(int errType, int errNo)
{
    std::string e =
        "*** " + std::to_string(errType) + ":" + std::to_string(errNo);
    memo->Lines->Add(e.c_str());

    if (errType == SSL_ERROR_SYSCALL)
    {
        m_IsError = true;
    }
}
//----------------------------------------------------------------------------

void TformMain::MemoWriter(void* _this, const std::string& text)
{
    reinterpret_cast<TformMain*>(_this)->memo->Lines->Add(text.c_str());
}
//----------------------------------------------------------------------------

