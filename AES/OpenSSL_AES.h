//----------------------------------------------------------------------------
//
// Module: OpenSSL_AES
// Author: J.Mannila
//
// -----
//
//---------------------------------------------------------------------------

#ifndef OpenSSL_AESH
#define OpenSSL_AESH

//---------------------------------------------------------------------------
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include <vector>
#include <string>
#include <optional>
//----------------------------------------------------------------------------

namespace zutl {

using QByteVector = std::vector<uint8_t>;

//
enum class LOpenSSL_AES_Error {
  ok,
  create_ctx_failed,
  encrypt_init_failed,
  decrypt_init_failed,
  encrypt_update_failed,
  decrypt_update_failed,
  encrypt_final_failed,
  decrypt_final_failed,
};


//***************************************************************************
//
// class COpenSSL_AES
// ----- ------------
//***************************************************************************

/*!
 */

class COpenSSL_AES {
  public:
    COpenSSL_AES() = default;

    COpenSSL_AES(const QByteVector& key, const QByteVector& iv)
      : m_Key(key), m_IV(iv) {}

    ~COpenSSL_AES();

    // deleted copy constructor
    COpenSSL_AES(const COpenSSL_AES&) = delete;

    // deleted copy assignment
    COpenSSL_AES& operator=(const COpenSSL_AES&) = delete;

    /// Salaa plainText vektorin sisällön. Kopioi parametrin m_PlainText-
    /// vektoriin ja kutsuu Encrypt()-metodia.
    bool Encrypt(const QByteVector& plainText);

    // move-versio edellisestä
    bool Encrypt(const QByteVector&& plainText);

    /// Salaa m_PlainText vektorin sisällön käyttäen avainta m_Key ja
    /// alustusvektoria m_IV käyttäen. Salattu vetori luettavissa CiperText()
    /// metodilla.
    bool Encrypt();

    /// Purkaa cipherText vektorin sisällön. Kopioi parametrin m_CipherText-
    /// vektoriin ja kutsuu Decrypt()-metodia.
    bool Decrypt(const QByteVector& cipherText);

    // move-versio edellisestä
    bool Decrypt(const QByteVector&& cipherText);

    /// Salaa m_CipherText vektorin sisällön käyttäen avainta m_Key ja
    /// alustusvektoria m_IV käyttäen. Salattu vetori luettavissa PlainText()
    /// metodilla.
    bool Decrypt();

    /// Asettaa salausavaimen m_Key.
    void SetKey(const QByteVector& key)
        { m_Key = key; }

    /// Asettaa alustusvektorin m_IV.
    void SetIV(const QByteVector& iv)
        { m_IV = iv; }

    /// Asettaa salattavan vektorin m_PlayText.
    void SetPlainText(const QByteVector& bytes)
        { m_PlainText = bytes; }

    /// Asettaa purettavan vektorin m_CipherText.
    void SetCipherText(const QByteVector& bytes)
        { m_CipherText = bytes; }

    /// Palauttaa puretun vektorin m_PlainText.
    QByteVector PlainText() const
        { return m_PlainText; }

    /// Palauttaa salatun vektorin m_CipherText.
    QByteVector CipherText() const
        { return m_CipherText; }

    /// Jos salauksessa tai salauksen purussa on tapahtunut virhe, HasErrors()-
    /// metodi kertoo tämän.
    bool HasErrors() const
        { return m_HasErrors; }

    /// Palauttaa edellisen virheen.
    LOpenSSL_AES_Error LastError() const
        { return m_LastError; }

    /// Palauttaa edelliseen virheeseen liittyvän tekstin.
    std::string ErrorText() const
        { return m_ErrorText; }

  private:
    QByteVector m_Key {};
    QByteVector m_IV {};
    QByteVector m_PlainText {};
    QByteVector m_CipherText {};

    bool m_HasErrors {};
    LOpenSSL_AES_Error m_LastError { LOpenSSL_AES_Error::ok };
    std::string m_ErrorText {};

    QByteVector m_Buffer {};

    EVP_CIPHER_CTX* m_CTX {};

    //
    EVP_CIPHER_CTX* CreateCTX();

    // Kutsuu funktiota EVP_EncryptInit_ex, joka ..
    bool EncryptInit();

    // Kutsuu funktiota EVP_DecryptInit_ex, joka ...
    bool DecryptInit();

    // Kutsuu funktiota EVP_EncryptUpdate, joka ..
    std::optional<int> EncryptUpdate();

    // Kutsuu funktiota EVP_DecryptUpdate, joka ..
    std::optional<int> DecryptUpdate();

    // Kutsuu funktiota EVP_EncryptFinal_ex, joka
    std::optional<int> EncryptFinal(int length);

    // Kutsuu funktiota EVP_DecryptFinal_ex, joka
    std::optional<int> DecryptFinal(int length);

    //
    bool HandleError(LOpenSSL_AES_Error);
};

}

#endif
