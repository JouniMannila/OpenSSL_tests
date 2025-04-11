//----------------------------------------------------------------------------
//
// Module: CriticalSection
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

#ifndef CriticalSectionH
#define CriticalSectionH

//---------------------------------------------------------------------------
#include <windows.h>
//---------------------------------------------------------------------------

namespace zutl {

//***************************************************************************
//
// class CCriticalSection
// ----- ----------------
//***************************************************************************

/*!
 Luokka kapseloi turvallisesti Windows-API:n Critical Section:in.

 T‰m‰ on turvallinen tapa k‰ytt‰‰ Critical Section:ia, koska se
 automaattisesti kutsuu LeaveCriticalSection() funktiota instanssin
 poistuessa scope:sta.
 */

__declspec(dllexport) class CCriticalSection {
  public:
    CCriticalSection() = delete;

    /// Contructor - Kutsuu EnterCriticalSection(...)
    explicit CCriticalSection(CRITICAL_SECTION* section);

    CCriticalSection(const CCriticalSection&) = delete;
    CCriticalSection& operator=(const CCriticalSection&) const = delete;

    /// Destructor - Kutsuu LeaveCriticalSection()
    ~CCriticalSection();

  private:
    CRITICAL_SECTION* m_Section {};
};

}

#endif
