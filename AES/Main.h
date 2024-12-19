//----------------------------------------------------------------------------
//
// Module: Main
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

#ifndef MainH
#define MainH

//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>

#include <vector>
#include <array>
//---------------------------------------------------------------------------

using QByteVector = std::vector<uint8_t>;

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
  TButton *butEncrypt;
  TButton *butDecrypt;
  TMemo *memo1;
  TMemo *memo2;
  void __fastcall FormShow(TObject *Sender);
  void __fastcall butEncryptClick(TObject *Sender);
  void __fastcall butDecryptClick(TObject *Sender);

private:	// User declarations
  QByteVector m_Crypted {};
  QByteVector m_Decrypted {};

public:		// User declarations
  __fastcall TformMain(TComponent* Owner);
};

//---------------------------------------------------------------------------
extern PACKAGE TformMain *formMain;
//---------------------------------------------------------------------------

#endif
