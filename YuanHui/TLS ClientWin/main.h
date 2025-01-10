//----------------------------------------------------------------------------
//
// Module: main
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

#ifndef mainH
#define mainH

//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
//---------------------------------------------------------------------------

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

    void SetPortNo(int portNo)
        { m_PortNo = portNo; }

    SOCKET Socket() const
        { return m_Socket; }

    ///
    bool Initialize();

    ///
    bool Connect();

  private:
    int m_PortNo {};

    struct addrinfo* m_AddrInfo = nullptr;

    SOCKET m_Socket { INVALID_SOCKET };

    bool startup();
    bool getAddrInfo();
};


//***************************************************************************
//
// class CSSLClient
// ----- ----------
//***************************************************************************

/*!
 */

class CSSLClient {
  public:
    ~CSSLClient();

    void Initialize();

    bool CreateCTX();

    void CreateSSL(SOCKET socket);

    bool Connect();

  private:
    SSL_CTX* m_CTX {};
    SSL* m_SSL {};
};


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
  TMemo *memo;
  TButton *Button1;
  void __fastcall Button1Click(TObject *Sender);

private:	// User declarations
  CTcpClient m_TCPClient {};
  CSSLClient m_SSLClient {};

public:		// User declarations
  __fastcall TformMain(TComponent* Owner);
};

//---------------------------------------------------------------------------
extern PACKAGE TformMain *formMain;
//---------------------------------------------------------------------------

#endif
