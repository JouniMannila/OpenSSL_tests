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
// class CTcpServer
// ----- ----------
//***************************************************************************

/*!
 */

class CTcpServer {
  public:
    ~CTcpServer();

    bool Connect(int portNo);
    SOCKET Accept();

    SOCKET ListenerSocket() const
        { return m_ListenerSocket; }

    SOCKET ClientSocket() const
        { return m_ClientSocket; }

  private:
    struct addrinfo* m_AddrInfo = nullptr;

    SOCKET m_ListenerSocket {};
    SOCKET m_ClientSocket {};

    bool startup();
    bool getAddrInfo(int port);
};


//***************************************************************************
//
// class CSSLServer
// ----- ----------
//***************************************************************************

/*!
 */

class CSSLServer {
  public:
    ~CSSLServer();

    bool CreateCTX();

    bool CreateSSL(SOCKET socket);

    bool HandleCertificate();

    bool Accept();

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
  TButton *Button1;
  TMemo *memo;
  void __fastcall FormShow(TObject *Sender);
  void __fastcall Button1Click(TObject *Sender);

private:	// User declarations
  CTcpServer m_TCPServer {};
  CSSLServer m_SSLServer {};

public:		// User declarations
  __fastcall TformMain(TComponent* Owner);
};

//---------------------------------------------------------------------------
extern PACKAGE TformMain *formMain;
//---------------------------------------------------------------------------

#endif
