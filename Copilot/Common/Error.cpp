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

}

