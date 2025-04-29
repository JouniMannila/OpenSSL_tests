//----------------------------------------------------------------------------
//
// Module: Error
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#pragma hdrstop

#include "Error.h"

#include <openssl/err.h>
#include <string>
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace ztls {

//***************************************************************************
//
// functions
// ---------
//***************************************************************************

std::string getOpenSSLError()
{
    BIO *bio = BIO_new(BIO_s_mem());
    ERR_print_errors(bio);

    char *buf;
    size_t len = BIO_get_mem_data(bio, &buf);
    std::string s(buf, len);

    BIO_free(bio);
    return s;
}
//----------------------------------------------------------------------------


//***************************************************************************
//
// class CSSL_GetError
// ----- -------------
//***************************************************************************

std::string CSSL_GetError::Reason(SSL* ssl, int functionRetValue)
{
    int ret = SSL_get_error(ssl, functionRetValue);

    switch (ret)
    {
    case SSL_ERROR_NONE:
        return "SSL_ERROR_NONE";
    case SSL_ERROR_ZERO_RETURN:
        return "SSL_ERROR_ZERO_RETURN";
    case SSL_ERROR_WANT_READ:
        return "SSL_ERROR_WANT_READ";
    case SSL_ERROR_WANT_WRITE:
        return "SSL_ERROR_WANT_WRITE";
    case SSL_ERROR_WANT_CONNECT:
        return "SSL_ERROR_WANT_CONNECT";
    case SSL_ERROR_WANT_ACCEPT:
        return "SSL_ERROR_WANT_ACCEPT";
    case SSL_ERROR_WANT_X509_LOOKUP:
        return "SSL_ERROR_WANT_X509_LOOKUP";
    case SSL_ERROR_WANT_ASYNC:
        return "SSL_ERROR_WANT_ASYNC";
    case SSL_ERROR_WANT_ASYNC_JOB:
        return "SSL_ERROR_WANT_ASYNC_JOB";
    case SSL_ERROR_WANT_CLIENT_HELLO_CB:
        return "SSL_ERROR_WANT_CLIENT_HELLO_CB";
    case SSL_ERROR_SYSCALL:
        return "SSL_ERROR_SYSCALL";
    case SSL_ERROR_SSL:
        return "SSL_ERROR_SSL";
    default:
        return "";
    }

    return "";
}
//----------------------------------------------------------------------------

}

