// TLS_UDP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <conio.h>

#include "CommandLineArgs.h"
#include "ConsoleColors.h"
#include "OpenSSL_Client.h"

const int PORTNO = 12350;
const char* ADDRESS = "127.0.0.1";

//const char* CERTIFICATE = "certs/jounin-ca.crt";
const char* CERTIFICATE = "certs/hedsam.crt";

const bool g_WaitAfter = true;


//***************************************************************************
//
// functions
// ---------
//***************************************************************************

int showError(const ztls::CError& error)
{
    using namespace std;

    CConsoleColors colors;
    if (!colors.SetColors(FOREGROUND_RED | FOREGROUND_INTENSITY))
        return 0;

    cout << endl << "-- ERROR " << string(41, '-') << endl << endl
        << error.Caption << endl;

    if (!error.Message.empty())
        cout << endl << error.Message << endl;

    cout << endl << string(50, '-') << endl << endl;

    if (g_WaitAfter)
    {
        std::cout << "Hit anyt key!" << std::endl;
        _getch();
    }

    return 0;
}
//----------------------------------------------------------------------------


int showMessage(std::string_view msg)
{
    using namespace std;

    CConsoleColors colors;
    if (!colors.SetColors(FOREGROUND_BLUE | FOREGROUND_INTENSITY))
        return 0;

    cout << endl << "-- Message from server " << string(27, '-') << endl << endl
        << msg << endl
        << endl << string(50, '-') << endl << endl;

    if (g_WaitAfter)
        _getch();

    return 0;
}
//----------------------------------------------------------------------------


int main(int argc, char* argv[])
{
    std::string host = ADDRESS;
    std::string cert = CERTIFICATE;
    int portNo = PORTNO;

    zutl::CArgs args(argc, argv);

    if (argc > 1)
    {
        args.FindArg("--host", host);

        std::string port;
        if (args.FindArg("--port", port))
        {
            try {
                portNo = std::stoi(port);
            }
            catch (const std::invalid_argument& e) {
            }
        }

        args.FindArg("--cert", cert);
    }

    ztls::CTcpClient tcpClient(host, portNo);

    if (!tcpClient.Connect())
        return showError(tcpClient.GetLastError());

    //    if (!tcpClient.Initialize())
    //        return showError(tcpClient.GetLastError());
    //
    //    if (!tcpClient.Connect())
    //        return showError(tcpClient.GetLastError());

    ztls::COpenSSL_Client sslClient(cert);

    //    if (!sslClient.MakeConnection(tcpClient))
    //        return showError(sslClient.GetLastError());

    sslClient.Initialize();

    if (!sslClient.CreateContext())
        return showError(sslClient.GetLastError());

    if (!sslClient.SetVersions())
        return showError(sslClient.GetLastError());

    if (!sslClient.CreateSSL(tcpClient.ServerSocket()))
        return showError(sslClient.GetLastError());

    if (!sslClient.Connect())
        return showError(sslClient.GetLastError());

    if (!sslClient.LoadVerifyLocations())
        return showError(sslClient.GetLastError());

    if (!sslClient.DisplayCerts())
        return 0;

    //    if (!sslClient.VerifyCertification())
    //        return showError(sslClient.GetLastError());

    if (!sslClient.Write("Hello from the client!"))
        return showError(sslClient.GetLastError());

    std::string message;
    if (sslClient.Read(message))
    {
        showMessage(message);
    }

    if (g_WaitAfter)
    {
        std::cout << "Hit anyt key!" << std::endl;
        _getch();
    }

    return 0;
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
