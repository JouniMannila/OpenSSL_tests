//----------------------------------------------------------------------------
//
// Module: ConsoleColors
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#pragma hdrstop

#include "ConsoleColors.h"
//---------------------------------------------------------------------------

//#pragma package(smart_init)

//***************************************************************************
//
// class CConsoleColors
// ----- --------------
//***************************************************************************

CConsoleColors::~CConsoleColors()
{
    if (m_Handle != INVALID_HANDLE_VALUE)
        RestoreColors();
}
//----------------------------------------------------------------------------

bool CConsoleColors::SetColors(WORD color)
{
    if (m_Handle == INVALID_HANDLE_VALUE)
    {
        m_Handle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (m_Handle == INVALID_HANDLE_VALUE)
            return false;
    }

    // lueteaan vanhat arvot
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(m_Handle, &info))
        return false;

    // v�ri muistiin
    m_Colors = info.wAttributes;

    // vaihdetaan tekstin v�ri
    if (!SetConsoleTextAttribute(m_Handle, color))
        return false;

    return true;
}
//----------------------------------------------------------------------------

bool CConsoleColors::RestoreColors()
{
    if (m_Handle == INVALID_HANDLE_VALUE)
        return false;

    // palautetaan tekstin v�ri
    return SetConsoleTextAttribute(m_Handle, m_Colors);
}
//----------------------------------------------------------------------------
