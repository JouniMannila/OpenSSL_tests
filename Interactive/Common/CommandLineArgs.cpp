//----------------------------------------------------------------------------
//
// Module: CommandLineArgs
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#pragma hdrstop

#include "CommandLineArgs.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace zutl {

//***************************************************************************
//
// class CArgs
// ----- -----
//***************************************************************************

CArgs::CArgs(int argc, char* argv[])
{
    for (int i=0; i < argc; ++i)
        m_Args.push_back(argv[i]);
}
//----------------------------------------------------------------------------

bool CArgs::FindArg(const std::string& name) const
{
    QArgs::const_iterator it = m_Args.begin();
    for (; it != m_Args.end(); ++it)
    {
        if (*it == name)
            return true;
    }

    return false;
}
//----------------------------------------------------------------------------

bool CArgs::FindArg(const std::string& name, std::string& arg) const
{
    QArgs::const_iterator it = m_Args.begin();
    for (; it != m_Args.end(); ++it)
    {
        if (*it == name)
        {
            ++it;
            if (it == m_Args.end())
                return false;
            arg = *it;
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------

}

