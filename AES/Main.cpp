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

#include "Main.h"
#include "OpenSSL_AES.h"

#include <sstream>
#include <iomanip>
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"
TformMain *formMain;

/* A 256 bit key */
std::vector<uint8_t> Key = {
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
  0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
  0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31
};

/* A 128 bit IV */
std::vector<uint8_t> IV = {
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35
};


std::string vector2string(const QByteVector& v)
{
    std::string s;
    for (auto byte : v)
        s.push_back(byte);
    return s;
}
//----------------------------------------------------------------------------

QByteVector string2vector(const std::string& s)
{
    QByteVector v;
    for (auto byte : s)
        v.push_back(byte);
    return v;
}             
//----------------------------------------------------------------------------

std::string binary2String(const QByteVector& v)
{
    using namespace std;

    string s;
    for (auto byte : v)
    {
        stringstream ss;
        ss.fill('0');
        ss << '[' << setw(2) << hex << (int)byte << ']';
        s += ss.str();
    }
    return s;
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
}
//---------------------------------------------------------------------------

void __fastcall TformMain::FormShow(TObject *Sender)
{
//
}
//---------------------------------------------------------------------------

void __fastcall TformMain::butEncryptClick(TObject *Sender)
{
    std::string plaintext = AnsiString(memo1->Text).c_str();

    zutl::COpenSSL_AES aes(Key, IV);

    if (!aes.Encrypt(string2vector(plaintext)))
    {
        memo2->Text = AnsiString("Error: ") + aes.ErrorText().c_str();
        ::MessageBeep(-1);
        return;
    }

    m_Crypted = aes.CipherText();

    memo2->Text = binary2String(m_Crypted).c_str();
}
//---------------------------------------------------------------------------

void __fastcall TformMain::butDecryptClick(TObject *Sender)
{
    zutl::COpenSSL_AES aes(Key, IV);

    if (!aes.Decrypt(m_Crypted))
    {
        memo1->Text = AnsiString("Error: ") + aes.ErrorText().c_str();
        ::MessageBeep(-1);
        return;
    }

    m_Decrypted = aes.PlainText();

    AnsiString S;
    S += AnsiString("Decrypted:\r\n");
    S += vector2string(m_Decrypted).c_str();
    memo1->Text = S;
}
//---------------------------------------------------------------------------

