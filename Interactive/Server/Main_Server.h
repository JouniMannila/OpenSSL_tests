//----------------------------------------------------------------------------
//
// Module: Main
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

#ifndef Main_ServerH
#define Main_ServerH

//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.ExtCtrls.hpp>

#include "OpenSSL_Server.h"
//---------------------------------------------------------------------------

typedef void __fastcall (__closure* CAccepted)();

//***************************************************************************
//
// class CServerThread
// ----- -------------
//***************************************************************************

/*!
 */

class CServerThread : public TThread {
  public:
    CServerThread(ztls::CTcpServer*, CAccepted);

  private:
    ztls::CTcpServer* m_Server {};

    CAccepted m_AcceptedCb { nullptr };

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
  void __fastcall butConnectClick(TObject *Sender);
  void __fastcall butSendClick(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall timerTimer(TObject *Sender);

private:	// User declarations
  ztls::CTcpServer* m_TcpServer {};
  ztls::COpenSSL_Server* m_SslServer {};

  CServerThread* m_ServerThread {};

  bool m_Connected {};
  bool m_Accepted {};

  bool __fastcall Connect();
  void __fastcall Disconnect();

  bool __fastcall ShowError(const ztls::CError&);

  void __fastcall OnAccepted();

public:		// User declarations
  __fastcall TformMain(TComponent* Owner);
  __fastcall ~TformMain();
};

//---------------------------------------------------------------------------
extern PACKAGE TformMain *formMain;
//---------------------------------------------------------------------------

#endif
