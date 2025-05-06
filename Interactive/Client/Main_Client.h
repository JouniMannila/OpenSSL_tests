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
#include <Vcl.ComCtrls.hpp>

#include <queue>
#include <mutex>
//---------------------------------------------------------------------------

////***************************************************************************
////
//// class CClientThread
//// ----- --------------
////***************************************************************************
//
///*!
// */
//
//class CClientThread : public TThread {
//  public:
//    CClientThread(ztls::COpenSSL_Client*, CNewMessage, CCloseNotify);
//
//    /// Palauttaa tiedon siitä, onko vastaanottojono tyhjä.
//    bool __fastcall Empty();
//
//    /// Lukee ja poistaa vanhimman viestin jonosta.
//    std::string __fastcall Fetch();
//
//  private:
//    ztls::COpenSSL_Client* m_Client {};
//
//    std::mutex m_Mutex {};
//
//    CNewMessage m_NewMessageCb { nullptr };
//    CCloseNotify m_CloseNotifyCb { nullptr };
//
//    std::deque<std::string> m_Deque {};
//
//    // thread:in suorittava looppi
//    void __fastcall Execute();
//
//    // kopioi viestin jonoon mutex:in sisällä
//    void __fastcall Push(const std::string& message);
//};


struct CStatusFlags {
  bool Connected {};
  bool Disconnected {};
  bool NewMessage {};
  bool CloseNotified {};
  bool Error {};
  bool TryConnect {};
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
  TEdit *edAddress;
  TLabel *lblAdddress;
  TLabel *lblPort;
  TEdit *edPort;
  TUpDown *udPort;
  void __fastcall butConnectClick(TObject *Sender);
  void __fastcall butSendClick(TObject *Sender);
  void __fastcall timerTimer(TObject *Sender);

private:	// User declarations
  std::unique_ptr<ztls::CTlsClient> m_TlsClient {};

  bool m_Connected {};
  bool m_ConnectionFailed {};

  int m_TryCounter {};

  CStatusFlags m_Status {};

  bool __fastcall Connect();
  void __fastcall Disconnect(bool disableTimer=true);

  bool __fastcall ShowError(const ztls::CTlsResult&);

  void OnConnected();
  void OnDisconnected();
  void OnNewMessage();
  void OnCloseNotify();
  void OnError(int errType, int errNo, const std::string& source);

  void OnDebugString(const std::string&);

public:		// User declarations
  __fastcall TformMain(TComponent* Owner);
  __fastcall ~TformMain();
};

//---------------------------------------------------------------------------
extern PACKAGE TformMain *formMain;
//---------------------------------------------------------------------------

#endif

