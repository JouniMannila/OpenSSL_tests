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
//---------------------------------------------------------------------------

typedef void __fastcall (__closure* CNewMessage)();

//***************************************************************************
//
// class CClientThread
// ----- --------------
//***************************************************************************

/*!
 */

class CClientThread : public TThread {
  public:
    CClientThread(ztls::COpenSSL_Client*, CNewMessage);
    __fastcall ~CClientThread();

    CClientThread(const CClientThread&) = delete;
    CClientThread& operator=(const CClientThread&) = delete;

    /// Palauttaa tiedon siitä, onko vastaanottojono tyhjä.
    bool __fastcall Empty();

    /// Lukee ja poistaa vanhimman viestin jonosta.
    std::string __fastcall Fetch();

  private:
    CRITICAL_SECTION m_CriticalSection;

    ztls::COpenSSL_Client* m_Client {};

    CClientThread* m_ClientThread {};

    CNewMessage m_NewMessageCb { nullptr };

    std::deque<std::string> m_Deque;

    // thread:in suorittava looppi
    void __fastcall Execute();
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
  void __fastcall FormShow(TObject *Sender);
  void __fastcall butConnectClick(TObject *Sender);
  void __fastcall butSendClick(TObject *Sender);
  void __fastcall timerTimer(TObject *Sender);

private:	// User declarations
  ztls::CTcpClient* m_TcpClient {};
  ztls::COpenSSL_Client* m_SslClient {};

  CClientThread* m_ClientThread {};

  bool m_Connected {};
  bool m_NewMessage {};

  bool __fastcall Connect();
  void __fastcall Disconnect();

  bool __fastcall ShowError(const ztls::CError&);

  void __fastcall OnNewMessage();

public:		// User declarations
  __fastcall TformMain(TComponent* Owner);
  __fastcall ~TformMain();
};

//---------------------------------------------------------------------------
extern PACKAGE TformMain *formMain;
//---------------------------------------------------------------------------

#endif

