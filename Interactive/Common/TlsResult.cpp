//----------------------------------------------------------------------------
//
// Module: TlsResult
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma hdrstop

#include "TlsResult.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

namespace ztls {

std::string TlsResultText(int errCode)
{
    switch (errCode)
    {
    case tlse_Ok:
        return "";

    case tlse_TCP_Port:
        return "Port == 0.";
    case tlse_TCP_Address:
        return "Address == \"\".";

    case tlse_TCP_WSAStartup:
        return "WSAStartup failed.";
    case tlse_TCP_socket:
        return "Create TCP socket failed.";
    case tlse_TCP_connect:
        return "Unable to connect.";

    case tlse_SSL_CTX_new:
        return "Unable to create SSL context.";
    case tlse_SSL_CTX_set_min_proto_version:
        return "Unable to set min proto version.";
    case tlse_SSL_CTX_set_max_proto_version:
        return "Unable to set max proto version.";
    case tlse_SSL_new:
        return "SSL_new failed.";
    case tlse_SSL_set_fd:
        return "SSL_set_fd failed.";
    case tlse_SSL_get_peer_certificate:
        return "SSL_get_peer_certificate failed.";
    case tlse_X509_get_subject_name:
        return "X509_get_subject_name failed.";
    case tlse_X509_get_issuer_name:
        return "X509_get_issuer_name failed.";
    case tlse_SSL_CTX_load_verify_locations:
        return "SSL_CTX_load_verify_locations failed.";
    case tlse_SSL_get_verify_result:
        return "SSL_get_verify_result failed.";
    case tlse_SSL_connect:
        return "SSL_connect failed.";
    case tlse_SSL_write:
        return "SSL_write failed.";
    case tlse_SSL_shutdown:
        return "SSL_shutdown failed.";
    }
}
//----------------------------------------------------------------------------

}
