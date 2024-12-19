//----------------------------------------------------------------------------
//
// Module: OpenSSL_AES
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#pragma hdrstop

#include "OpenSSL_AES.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace zutl {

//***************************************************************************
//
// class COpenSSL_AES
// ----- ------------
//***************************************************************************

COpenSSL_AES::~COpenSSL_AES()
{
    if (m_CTX)
        EVP_CIPHER_CTX_free(m_CTX);
}
//----------------------------------------------------------------------------

bool COpenSSL_AES::Encrypt(const QByteVector& plainText)
{
    m_PlainText = plainText;
    return Encrypt();
}
//----------------------------------------------------------------------------

bool COpenSSL_AES::Encrypt(const QByteVector&& plainText)
{
    m_PlainText = std::move(plainText);
    return Encrypt();
}
//----------------------------------------------------------------------------

bool COpenSSL_AES::Encrypt()
{
    if (!CreateCTX())
        return false;

    if (!EncryptInit())
        return false;

    // asettaa puskurin koon m_PlainText-vektorin koon mukaan
    m_Buffer.resize(m_PlainText.size() + 16, 0);

    std::optional<int> length;

    if (length = EncryptUpdate(); !length)
        return false;

    int ciphertext_len = *length;

    if (length = EncryptFinal(*length); !length)
        return false;

    ciphertext_len += *length;

    m_Buffer.resize(ciphertext_len);
    m_CipherText = m_Buffer;

    return true;
}
//----------------------------------------------------------------------------

bool COpenSSL_AES::Decrypt(const QByteVector& cipherText)
{
    m_CipherText = cipherText;
    return Decrypt();
}
//----------------------------------------------------------------------------

bool COpenSSL_AES::Decrypt(const QByteVector&& cipherText)
{
    m_CipherText = std::move(cipherText);
    return Decrypt();
}
//----------------------------------------------------------------------------

bool COpenSSL_AES::Decrypt()
{
    if (!CreateCTX())
        return false;

    if (!DecryptInit())
        return false;

    // asettaa puskurin koon m_CipherText-vektorin koon mukaan
    m_Buffer.resize(m_CipherText.size() + 16, 0);

    std::optional<int> length;

    if (length = DecryptUpdate(); !length)
        return false;

    int plaintext_len = *length;

    if (length = DecryptFinal(*length); !length)
        return false;

    plaintext_len += *length;

    m_Buffer.resize(plaintext_len);
    m_PlainText = m_Buffer;

    return true;
}
//----------------------------------------------------------------------------

EVP_CIPHER_CTX* COpenSSL_AES::CreateCTX()
{
    if (m_CTX)
        return m_CTX;

    if (m_CTX = EVP_CIPHER_CTX_new())
        return m_CTX;

    HandleError(LOpenSSL_AES_Error::create_ctx_failed);
    return nullptr;
}
//----------------------------------------------------------------------------

bool COpenSSL_AES::EncryptInit()
{
    /*
     * Initialise the decryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if (auto r = EVP_EncryptInit_ex(
            m_CTX, EVP_aes_256_cbc(), nullptr, m_Key.data(), m_IV.data());
            r == 1)
    {
        return true;
    }

    return HandleError(LOpenSSL_AES_Error::encrypt_init_failed);
}
//----------------------------------------------------------------------------

bool COpenSSL_AES::DecryptInit()
{
    /*
     * Initialise the decryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if (auto r = EVP_DecryptInit_ex(
            m_CTX, EVP_aes_256_cbc(), nullptr, m_Key.data(), m_IV.data());
            r == 1)
    {
        return true;
    }

    return HandleError(LOpenSSL_AES_Error::decrypt_init_failed);
}
//----------------------------------------------------------------------------

std::optional<int> COpenSSL_AES::EncryptUpdate()
{
    int length = 0;

    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if (auto r = EVP_EncryptUpdate(
            m_CTX, m_Buffer.data(), &length, m_PlainText.data(),
            m_PlainText.size());
            r == 1)
    {
        return std::optional<int>(length);
    }

    HandleError(LOpenSSL_AES_Error::encrypt_update_failed);
    return std::optional<int>();
}
//----------------------------------------------------------------------------

std::optional<int> COpenSSL_AES::DecryptUpdate()
{
    int length = 0;

    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary.
     */
    if (auto r = EVP_DecryptUpdate(
            m_CTX, m_Buffer.data(), &length, m_CipherText.data(),
            m_CipherText.size());
            r == 1)
    {
        return std::optional<int>(length);
    }

    HandleError(LOpenSSL_AES_Error::decrypt_update_failed);
    return std::optional<int>();
}
//----------------------------------------------------------------------------

std::optional<int> COpenSSL_AES::EncryptFinal(int length)
{
    int finalLength = 0;

    /*
     * Finalise the encryption. Further plaintext bytes may be written at
     * this stage.
     */
    if (auto r = EVP_EncryptFinal_ex(
            m_CTX, &m_Buffer[length], &finalLength); r == 1)
    {
        return std::optional<int>(finalLength);
    }

    HandleError(LOpenSSL_AES_Error::encrypt_final_failed);
    return std::optional<int>();
}
//----------------------------------------------------------------------------

std::optional<int> COpenSSL_AES::DecryptFinal(int length)
{
    int finalLength = 0;

    /*
     * Finalise the decryption. Further plaintext bytes may be written at
     * this stage.
     */
    if (auto r = EVP_DecryptFinal_ex(
            m_CTX, &m_Buffer[length], &finalLength); r == 1)
    {
        return std::optional<int>(finalLength);
    }

    HandleError(LOpenSSL_AES_Error::decrypt_final_failed);
    return std::optional<int>();
}
//----------------------------------------------------------------------------

bool COpenSSL_AES::HandleError(LOpenSSL_AES_Error error)
{
    m_LastError = error;

    if (error == LOpenSSL_AES_Error::ok)
    {
        m_ErrorText = "No errors.";
        return true;
    }

    switch (error)
    {
    case LOpenSSL_AES_Error::ok:
        break;
    case LOpenSSL_AES_Error::create_ctx_failed:
        m_ErrorText = "Create CTX failed.";
        break;
    case LOpenSSL_AES_Error::encrypt_init_failed:
        m_ErrorText = "EncryptInit failed.";
        break;
    case LOpenSSL_AES_Error::decrypt_init_failed:
        m_ErrorText = "DecryptInit failed.";
        break;
    case LOpenSSL_AES_Error::encrypt_update_failed:
        m_ErrorText = "EncryptUpdate failed.";
        break;
    case LOpenSSL_AES_Error::decrypt_update_failed:
        m_ErrorText = "DecryptUpdate failed.";
        break;
    case LOpenSSL_AES_Error::encrypt_final_failed:
        m_ErrorText = "EncryptFinal failed.";
        break;
    case LOpenSSL_AES_Error::decrypt_final_failed:
        m_ErrorText = "DecryptFinal failed.";
        break;
    }

    m_HasErrors = true;
    return false;
}
//----------------------------------------------------------------------------

}
