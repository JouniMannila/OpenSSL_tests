//----------------------------------------------------------------------------
//
// Module: CommandLineArgs
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

#ifndef CommandLineArgsH
#define CommandLineArgsH

//---------------------------------------------------------------------------
#include <vector>
#include <string>
//---------------------------------------------------------------------------

namespace zutl {

//***************************************************************************
//
// class CArgs
// ----- -----
//***************************************************************************

/*!
 Luokka kasittelee komentorivi parametrit.
 */

 class CArgs {
  public:
    CArgs(int argc, char* argv[]);

    /// Tutkii löytyykö komentoriviparametreista 'name'-parametrin nimistä.
    bool FindArg(const std::string& name) const;

    /// Etsii komentoriviparametreista 'name' nimisen parametria ja palauttaa
    /// sen arvon arg-parametrissa.
    bool FindArg(const std::string& name, std::string& arg) const;

  private:
    typedef std::vector<std::string> QArgs;
    QArgs m_Args;
};

}

#endif

