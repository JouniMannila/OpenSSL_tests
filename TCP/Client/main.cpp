//----------------------------------------------------------------------------
//
// Module: main
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "main.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"

TformMain *formMain;


//***************************************************************************
//
// class TformMain
// ----- ---------
//***************************************************************************

__fastcall TformMain::TformMain(TComponent* Owner)
  : TForm(Owner)
{
    g_Memo = memo;
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

void __fastcall TformMain::Connect()
{
    if (!m_Client.Initialize())
    {
        memo->Lines->Add(m_Client.GetLastError().c_str());
        return;
    }

    if (!m_Client.Connect())
    {
        memo->Lines->Add(m_Client.GetLastError().c_str());
        return;
    }

    m_Connected = true;
}
//----------------------------------------------------------------------------

void __fastcall TformMain::Disconnect()
{
    // shutdown the connection for sending since no more data will be sent
    // the client can still use the socket for receiving data
    if (!m_Client.Shutdown())
    {
        memo->Lines->Add(m_Client.GetLastError().c_str());
        return;
    }

    m_Connected = false;
}
//----------------------------------------------------------------------------


void __fastcall TformMain::butSendClick(TObject *Sender)
{
    if (!m_Connected)
        return;

    if (!m_Client.Send("this is a test"))
    {
        memo->Lines->Add(m_Client.GetLastError().c_str());
        return;
    }

//    if (!m_Client.Receive())
//    {
//        if (m_Client.HasError())
//        {
//            memo->Lines->Add(m_Client.GetLastError().c_str());
//            return;
//        }
//    }
}
//---------------------------------------------------------------------------

