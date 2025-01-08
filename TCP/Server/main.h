//----------------------------------------------------------------------------
//
// Module: main
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

#ifndef mainH
#define mainH

//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>

#include "server.h"
#include <Vcl.ExtCtrls.hpp>
//---------------------------------------------------------------------------

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
  TButton *butListen;
  TMemo *memo;
  TTimer *Timer1;
  void __fastcall butListenClick(TObject *Sender);
  void __fastcall Timer1Timer(TObject *Sender);

private:	// User declarations
  CTcpServer m_Server {};

  bool m_Listening {};

public:		// User declarations
  __fastcall TformMain(TComponent* Owner);
};

//---------------------------------------------------------------------------
extern PACKAGE TformMain *formMain;
//---------------------------------------------------------------------------

#endif
