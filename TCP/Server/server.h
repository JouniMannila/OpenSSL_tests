//----------------------------------------------------------------------------
//
// Module: server
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

#ifndef serverH
#define serverH

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
//---------------------------------------------------------------------------

#define DEFAULT_BUFLEN 512
#define IP_ADDR "127.0.0.1"

extern TMemo* g_Memo;


using FThreadCallback = void(*)(void* This);


//***************************************************************************
//
// class CThread
// ----- -------
//***************************************************************************

/*!
 Kantaluokka thredille. Ei suorita mit‰‰n j‰rkev‰‰ teht‰v‰‰. Se j‰‰ perivien
 luokkien teht‰v‰ksi.
 */

class CThread {
  public:
    ~CThread();

    void Terminate()
        { m_Terminated = true; }

    bool Terminated() const
        { return m_Terminated; }

    /// K‰ynnist‰‰ thread:in funktionaan Run().
    void Execute();

  private:
    std::thread* m_Thread {};

    bool m_Terminated { true };

    /// Virtuaalinen thread-funktio, joka perivien luokkien t‰ytyy ajaa yli.
    virtual void Run() {}
};


//***************************************************************************
//
// class CListenerThread
// ----- ---------------
//***************************************************************************

/*!
 */

class CListenerThread : public CThread {
  public:
    CListenerThread(SOCKET* socket) : m_Socket(socket) {}

    void SetReceivedCallback(
        void* this_,
        FThreadCallback received,
        FThreadCallback terminated=nullptr,
        FThreadCallback error=nullptr)
    {
        m_OwnerThis = this_;
        m_ReceivedCallback = received;
        m_TerminatedCallback = terminated;
        m_ErrorCallback = error;
    }

    ///
    bool DataReceived()
        { return std::exchange(m_DataReceived, false); }

    ///
    std::vector<char> Data() const
        { return m_Data; }

  private:
    SOCKET* m_Socket {};

    std::vector<char> m_Data {};
    bool m_DataReceived {};

    void* m_OwnerThis {};
    FThreadCallback m_ReceivedCallback {};
    FThreadCallback m_TerminatedCallback {};
    FThreadCallback m_ErrorCallback {};

    ///
    void Run() override;
};


//***************************************************************************
//
// class CTcpServer
// ----- ----------
//***************************************************************************

/*!
 */

class CTcpServer {
  public:
    CTcpServer() = default;

    ~CTcpServer();

    CTcpServer(const CTcpServer&) = delete;
    CTcpServer(CTcpServer&&) = delete;

    void SetPort(int portNo)
        { m_PortNo = portNo; }

    /// WSAStartup + getaddrinfo
    bool Initialize();

    /// freeaddrinfo + closesocket + WSACleanup
    void Cleanup();

    /// socket + bind + listen
    bool Listen();

    /// accept
    bool Accept();

    /// luo listening thread:in
    bool Receive();

    /// Initialize() + Listen() + Accept()
    bool Connect();

    ///
    bool Connected() const
        { return m_Connected; }

    ///
    bool DataReceived()
        { return std::exchange(m_DataReceived, false); }

    ///
    std::vector<char> GetReceivedData() const
        { return m_ReceivedData; }

    ///
    bool HasError() const
        { return m_HasError; }

    ///
    std::string GetLastError() const
        { return m_LastError; }

  private:
    int m_PortNo {};

    struct addrinfo* m_AddrInfo = nullptr;

    SOCKET m_ListenSocket { INVALID_SOCKET };
    SOCKET m_ClientSocket { INVALID_SOCKET };

    CListenerThread* m_ListenerThread {};

    bool m_Connected {};

    bool m_DataReceived {};
    std::vector<char> m_ReceivedData {};

    bool m_HasError {};
    std::string m_LastError {};

    bool startup();
    bool getAddrInfo(int port);

    bool SetLastError(std::string_view error);

    void SetDataReceived()
        { m_DataReceived = true; }

    void SetReceivedData(const std::vector<char>& data)
        { m_ReceivedData = data; }

    void Terminate();

    CListenerThread* ListenerThread() const
        { return m_ListenerThread; }

    static void OnDataReceived(void* this_);
    static void OnTerminated(void* this_);
    static void OnError(void* this_);
};

#endif

