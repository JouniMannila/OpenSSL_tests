//----------------------------------------------------------------------------
//
// Module: ConsoleColors
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

#ifndef ConsoleColorsH
#define ConsoleColorsH

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//***************************************************************************
//
// class CConsoleColors
// ----- --------------
//***************************************************************************

/*!
 */

class CConsoleColors {
  public:
    CConsoleColors() = default;
    ~CConsoleColors();

    CConsoleColors(const CConsoleColors&) = delete;
    CConsoleColors& operator=(const CConsoleColors&) = delete;

    bool SetColors(WORD color);
    bool RestoreColors();

  private:
    HANDLE m_Handle { INVALID_HANDLE_VALUE };
    WORD m_Colors {};
};

#endif
