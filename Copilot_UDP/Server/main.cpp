//----------------------------------------------------------------------------
//
// Module: main
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

//----------------------------------------------------------------------------
#include <iostream>
#include <winsock2.h>
#include <conio.h>
#include <stdio.h>

#include "ConsoleColors.h"
#include "OpenSSL_Server.h"
//----------------------------------------------------------------------------

const int PORTNO = 12350;

//const char* PRIVATEKEY = "certs/jounin-lantronix-server.key";
//const char* CERTIFICATE = "certs/jounin-lantronix-server.crt";

const char* PRIVATEKEY = "certs/hedsam.key";
const char* CERTIFICATE = "certs/hedsam.crt";

const bool g_WaitAfter = false;

//***************************************************************************
//
// functions
// ---------
//***************************************************************************

int showError(const ztls::CError& error)
{
    using namespace std;

    CConsoleColors colors;
    if (!colors.SetColors(FOREGROUND_RED|FOREGROUND_INTENSITY))
        return 0;

    cout << endl << "-- ERROR " << string(41, '-') << endl << endl
         << error.Caption << endl;

    if (!error.Message.empty())
        cout << endl << error.Message << endl;

    cout << endl << string(50, '-') << endl << endl;

    if (g_WaitAfter)
        getch();

    return 0;
}
//----------------------------------------------------------------------------


//
int createTcpServer()
{
    ztls::CTcpServer tcpServer(PORTNO);
    tcpServer.Initialize();

    ztls::COpenSSL_Server sslServer;
    sslServer.SetPrivateKey(PRIVATEKEY);
    sslServer.SetCertificate(CERTIFICATE);
    sslServer.Initialize();

    if (!sslServer.CreateContext())
        return showError(sslServer.GetLastError());

    if (!sslServer.ConfigureContext())
        return showError(sslServer.GetLastError());

    if (!tcpServer.Connect())
        return showError(tcpServer.GetLastError());

    while (1)
    {
        if (!tcpServer.Accept())
            return showError(tcpServer.GetLastError());

        if (!sslServer.CreateSSL(tcpServer.ClientSocket()))
            return showError(sslServer.GetLastError());

        if (!sslServer.Accept())
            return showError(sslServer.GetLastError());
        else
        {
            sslServer.Write("Hello, SSL/TLS world!");
        }
    }

    return 0;
}
//----------------------------------------------------------------------------


//
int createUdpServer()
{
    ztls::CUdpServer udpServer(PORTNO);
    udpServer.Initialize();

    ztls::COpenSSL_Server sslServer;
    sslServer.SetPrivateKey(PRIVATEKEY);
    sslServer.SetCertificate(CERTIFICATE);
    sslServer.Initialize();

    if (!sslServer.CreateContext())
        return showError(sslServer.GetLastError());

    if (!sslServer.ConfigureContext())
        return showError(sslServer.GetLastError());

    if (!udpServer.Connect())
        return showError(udpServer.GetLastError());

    while (1)
    {
        if (!sslServer.CreateSSL(udpServer.UdpSocket()))
            return showError(sslServer.GetLastError());

        if (!sslServer.Accept())
            return showError(sslServer.GetLastError());

        sslServer.Write("Hello, SSL/TLS/UDP world!");
    }

    return 0;
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// main
// ----
//***************************************************************************

int main()
{
    return createUdpServer();
//    return createTcpServer();
}

