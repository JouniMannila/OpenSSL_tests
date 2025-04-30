//----------------------------------------------------------------------------
//
// Module: TcpResult
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

#ifndef TlsResultH
#define TlsResultH

//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------

namespace ztls {

enum LTlsResult : int {
  tlse_Ok,

//  tlsrError,

  tlse_TCP_Port,
  tlse_TCP_Address,

  tlse_TCP_WSAStartup,
  tlse_TCP_socket,
  tlse_TCP_connect,

  tlse_SSL_CTX_new,
  tlse_SSL_CTX_set_min_proto_version,
  tlse_SSL_CTX_set_max_proto_version,
  tlse_SSL_new,
  tlse_SSL_set_fd,
  tlse_SSL_get_peer_certificate,
  tlse_X509_get_subject_name,
  tlse_X509_get_issuer_name,
  tlse_SSL_CTX_load_verify_locations,
  tlse_SSL_get_verify_result,
  tlse_SSL_connect,
  tlse_SSL_write,
  tlse_SSL_shutdown,
};


//***************************************************************************
//
// class CTlsResult
// ----- ----------
//***************************************************************************

/*!
 */

class CTlsResult {
  public:
    explicit CTlsResult(int errCode=tlse_Ok) : m_ErrCode(errCode) {}

    CTlsResult(int errCode, const std::string& message)
      : m_ErrCode(errCode), m_Message(message) {}

//    CTlsResult(int errCode, const std::string& caption, const std::string& msg)
//      : m_ErrCode(errCode), m_Caption(caption), m_Message(msg) {}

    ///
    explicit operator bool() const
        { return m_ErrCode == tlse_Ok; }

    ///
    bool operator!() const
        { return m_ErrCode != tlse_Ok; }

    ///
    int ErrCode() const
        { return m_ErrCode; }

    ///
//    std::string Caption() const
//        { return m_Caption; }

    ///
    std::string Message() const
        { return m_Message; }

  private:
    int m_ErrCode {};
//    std::string m_Caption {};
    std::string m_Message {};
};


extern std::string TlsResultText(int errCode);

}

#endif
