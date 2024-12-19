//----------------------------------------------------------------------------
//
// Module: Client
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

#ifndef ClientH
#define ClientH

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include <string>
//---------------------------------------------------------------------------

#pragma comment(lib, "Ws2_32.lib")

extern TMemo* g_Memo;

//***************************************************************************
//
// class CTcpClient
// ----- ----------
//***************************************************************************

/*!
 */

class CTcpClient {
  public:
    ~CTcpClient();

    ///
    bool Initialize();

    ///
    bool Shutdown();

    ///
    bool Connect();

    ///
    bool Send(std::string_view msg);

    ///
    bool Receive();

    ///
    bool HasError() const
        { return m_HasError; }

    ///
    std::string GetLastError() const
        { return m_LastError; }

  private:
    struct addrinfo* m_AddrInfo = nullptr;

    SOCKET m_Socket { INVALID_SOCKET };

    bool m_HasError {};
    std::string m_LastError {};

    bool startup();
    bool getAddrInfo();

    bool SetLastError(std::string_view error);
};


#endif
