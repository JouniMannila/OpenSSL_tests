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

#include <stdio.h>

#include "main.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"

#pragma comment(lib, "Ws2_32.lib")

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

void __fastcall TformMain::butListenClick(TObject *Sender)
{
    if (m_Listening)
    {
        butListen->Caption = "Listen";
    }

    else
    {
        if (!m_Server.Initialize(12300))
        {
            memo->Lines->Add(m_Server.GetLastError().c_str());
            return;
        }

        if (!m_Server.Connect())
        {
            memo->Lines->Add(m_Server.GetLastError().c_str());
            return;
        }

        memo->Lines->Add("Listening...");

        if (!m_Server.Accept())
        {
            memo->Lines->Add(m_Server.GetLastError().c_str());
            return;
        }

        if (!m_Server.Receive())
        {
            if (m_Server.HasError())
                memo->Lines->Add(m_Server.GetLastError().c_str());
            return;
        }

        butListen->Caption = "Unlisten";
        m_Listening = true;
    }
}
//---------------------------------------------------------------------------

void __fastcall TformMain::Timer1Timer(TObject *Sender)
{
    if (m_Listening && !m_Server.Receiving())
    {
        m_Server.Cleanup();
        m_Listening = false;
        butListen->Caption = "Listen";
    }

    else if (m_Server.DataReceived())
    {
        auto data = m_Server.GetReceivedData();
        std::string s(data.begin(), data.end());
        memo->Lines->Add(AnsiString().sprintf("Bytes received: %d", s.size()));
        memo->Lines->Add(s.c_str());
    }
}
//---------------------------------------------------------------------------

