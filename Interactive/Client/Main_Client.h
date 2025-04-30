//----------------------------------------------------------------------------
//
// Module: Main
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

#ifndef Main_ClientH
#define Main_ClientH

//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.ExtCtrls.hpp>

#include "OpenSSL_Client.h"

#include <queue>
#include <mutex>
//---------------------------------------------------------------------------

typedef void __fastcall (__closure* CNewMessage)();
typedef void __fastcall (__closure* CCloseNotify)();

//***************************************************************************
//
// class CClientThread
// ----- --------------
//***************************************************************************

/*!
 */

class CClientThread : public TThread {
  public:
    CClientThread(ztls::COpenSSL_Client*, CNewMessage, CCloseNotify);

    /// Palauttaa tiedon siitä, onko vastaanottojono tyhjä.
    bool __fastcall Empty();

    /// Lukee ja poistaa vanhimman viestin jonosta.
    std::string __fastcall Fetch();

  private:
    ztls::COpenSSL_Client* m_Client {};

    std::mutex m_Mutex {};

    CNewMessage m_NewMessageCb { nullptr };
    CCloseNotify m_CloseNotifyCb { nullptr };

    std::deque<std::string> m_Deque {};

    // thread:in suorittava looppi
    void __fastcall Execute();

    // kopioi viestin jonoon mutex:in sisällä
    void __fastcall Push(const std::string& message);
};


//***************************************************************************
//
// class TformMain
// ----- ---------
//***************************************************************************

/*!
 */

class TformMain : public TForm
{
__published:	// IDE-managed Components
  TMemo *memo;
  TPanel *panTop;
  TBitBtn *butConnect;
  TBitBtn *butSend;
  TTimer *timer;
  void __fastcall butConnectClick(TObject *Sender);
  void __fastcall butSendClick(TObject *Sender);
  void __fastcall timerTimer(TObject *Sender);

private:	// User declarations
  std::unique_ptr<ztls::CTcpClient> m_TcpClient {};
  std::unique_ptr<ztls::COpenSSL_Client> m_SslClient {};
  std::unique_ptr<CClientThread> m_ClientThread {};

  bool m_Connected {};
  bool m_NewMessage {};
  bool m_CloseNotified {};

  bool __fastcall Connect();
  void __fastcall Disconnect();

  bool __fastcall ShowError(const ztls::CTlsResult&);

  void __fastcall OnNewMessage();
  void __fastcall OnCloseNotify();

  static void MemoWriter(void* _this, const std::string& text);

public:		// User declarations
  __fastcall TformMain(TComponent* Owner);
  __fastcall ~TformMain();
};

//---------------------------------------------------------------------------
extern PACKAGE TformMain *formMain;
//---------------------------------------------------------------------------

#endif

