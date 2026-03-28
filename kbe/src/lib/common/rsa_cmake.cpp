// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "common/rsa.h"

#include "common/strutil.h"

#include <cstdio>
#include <cstring>

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

namespace KBEngine
{

namespace
{

EVP_PKEY* as_pkey(void* key)
{
  return static_cast<EVP_PKEY*>(key);
}

std::string last_error()
{
  char err[256] = {0};
  ERR_error_string_n(ERR_get_error(), err, sizeof(err));
  return err;
}

void log_error(const char* where)
{
  const auto error = last_error();
  if (!error.empty())
  {
    std::fprintf(stderr, "%s: %s\n", where, error.c_str());
  }
}

bool write_private_key(EVP_PKEY* key, const std::string& path)
{
  FILE* fp = std::fopen(path.c_str(), "wb");
  if (fp == NULL)
  {
    return false;
  }

  const bool ok = PEM_write_PrivateKey(fp, key, NULL, NULL, 0, NULL, NULL) == 1;
  std::fclose(fp);
  return ok;
}

bool write_public_key(EVP_PKEY* key, const std::string& path)
{
  FILE* fp = std::fopen(path.c_str(), "wb");
  if (fp == NULL)
  {
    return false;
  }

  const bool ok = PEM_write_PUBKEY(fp, key) == 1;
  std::fclose(fp);
  return ok;
}

EVP_PKEY* load_key(const std::string& path, bool private_key)
{
  FILE* fp = std::fopen(path.c_str(), "rb");
  if (fp == NULL)
  {
    return NULL;
  }

  EVP_PKEY* key = private_key
    ? PEM_read_PrivateKey(fp, NULL, NULL, NULL)
    : PEM_read_PUBKEY(fp, NULL, NULL, NULL);

  std::fclose(fp);
  return key;
}

int process_rsa_oaep(EVP_PKEY* key, const std::string& input, std::string& output, bool encrypt)
{
  EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(key, NULL);
  if (ctx == NULL)
  {
    log_error("EVP_PKEY_CTX_new");
    return -1;
  }

  int ok = encrypt ? EVP_PKEY_encrypt_init(ctx) : EVP_PKEY_decrypt_init(ctx);
  if (ok <= 0)
  {
    log_error(encrypt ? "EVP_PKEY_encrypt_init" : "EVP_PKEY_decrypt_init");
    EVP_PKEY_CTX_free(ctx);
    return -1;
  }

  if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
  {
    log_error("EVP_PKEY_CTX_set_rsa_padding");
    EVP_PKEY_CTX_free(ctx);
    return -1;
  }

  size_t out_len = 0;
  ok = encrypt
    ? EVP_PKEY_encrypt(ctx, NULL, &out_len,
        reinterpret_cast<const unsigned char*>(input.data()), input.size())
    : EVP_PKEY_decrypt(ctx, NULL, &out_len,
        reinterpret_cast<const unsigned char*>(input.data()), input.size());

  if (ok <= 0)
  {
    log_error(encrypt ? "EVP_PKEY_encrypt(size)" : "EVP_PKEY_decrypt(size)");
    EVP_PKEY_CTX_free(ctx);
    return -1;
  }

  output.resize(out_len);
  ok = encrypt
    ? EVP_PKEY_encrypt(ctx, reinterpret_cast<unsigned char*>(&output[0]), &out_len,
        reinterpret_cast<const unsigned char*>(input.data()), input.size())
    : EVP_PKEY_decrypt(ctx, reinterpret_cast<unsigned char*>(&output[0]), &out_len,
        reinterpret_cast<const unsigned char*>(input.data()), input.size());

  EVP_PKEY_CTX_free(ctx);

  if (ok <= 0)
  {
    log_error(encrypt ? "EVP_PKEY_encrypt" : "EVP_PKEY_decrypt");
    output.clear();
    return -1;
  }

  output.resize(out_len);
  return static_cast<int>(out_len);
}

} // namespace

KBE_RSA::KBE_RSA(const std::string& pubkeyname, const std::string& prikeyname) :
  rsa_public(NULL),
  rsa_private(NULL)
{
  if (!pubkeyname.empty() || !prikeyname.empty())
  {
    loadPrivate(prikeyname);
    loadPublic(pubkeyname);
  }
}

KBE_RSA::KBE_RSA() :
  rsa_public(NULL),
  rsa_private(NULL)
{
}

KBE_RSA::~KBE_RSA()
{
  if (rsa_public != NULL)
  {
    EVP_PKEY_free(as_pkey(rsa_public));
    rsa_public = NULL;
  }

  if (rsa_private != NULL)
  {
    EVP_PKEY_free(as_pkey(rsa_private));
    rsa_private = NULL;
  }
}

bool KBE_RSA::loadPublic(const std::string& keyname)
{
  if (rsa_public != NULL)
  {
    return true;
  }

  rsa_public = load_key(keyname, false);
  return rsa_public != NULL;
}

bool KBE_RSA::loadPrivate(const std::string& keyname)
{
  if (rsa_private != NULL)
  {
    return true;
  }

  rsa_private = load_key(keyname, true);
  return rsa_private != NULL;
}

bool KBE_RSA::generateKey(const std::string& pubkeyname,
                          const std::string& prikeyname,
                          int keySize,
                          int e)
{
  EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
  if (ctx == NULL)
  {
    log_error("EVP_PKEY_CTX_new_id");
    return false;
  }

  EVP_PKEY* generated = NULL;
  BIGNUM* exponent = BN_new();

  bool ok = exponent != NULL &&
    BN_set_word(exponent, static_cast<unsigned long>(e)) == 1 &&
    EVP_PKEY_keygen_init(ctx) == 1 &&
    EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, keySize) == 1 &&
    EVP_PKEY_CTX_set1_rsa_keygen_pubexp(ctx, exponent) == 1 &&
    EVP_PKEY_keygen(ctx, &generated) == 1 &&
    write_private_key(generated, prikeyname) &&
    write_public_key(generated, pubkeyname);

  BN_free(exponent);
  EVP_PKEY_CTX_free(ctx);

  if (!ok)
  {
    if (generated != NULL)
    {
      EVP_PKEY_free(generated);
    }
    log_error("KBE_RSA::generateKey");
    return false;
  }

  EVP_PKEY_free(generated);
  return loadPrivate(prikeyname) && loadPublic(pubkeyname);
}

std::string KBE_RSA::encrypt(const std::string& instr)
{
  std::string encrypted;
  if (encrypt(instr, encrypted) < 0)
  {
    return "";
  }

  std::string encoded(encrypted.size() * 2 + 1, '\0');
  strutil::bytes2string(reinterpret_cast<unsigned char*>(&encrypted[0]),
                        static_cast<int>(encrypted.size()),
                        reinterpret_cast<unsigned char*>(&encoded[0]),
                        static_cast<int>(encoded.size()));
  encoded.resize(std::strlen(encoded.c_str()));
  return encoded;
}

int KBE_RSA::encrypt(const std::string& instr, std::string& outCertifdata)
{
  if (rsa_public == NULL)
  {
    return -1;
  }

  return process_rsa_oaep(as_pkey(rsa_public), instr, outCertifdata, true);
}

int KBE_RSA::decrypt(const std::string& inCertifdata, std::string& outstr)
{
  if (rsa_private == NULL)
  {
    return -1;
  }

  return process_rsa_oaep(as_pkey(rsa_private), inCertifdata, outstr, false);
}

std::string KBE_RSA::decrypt(const std::string& instr)
{
  std::string encrypted(instr.size() / 2, '\0');
  const int bytes = strutil::string2bytes(
    reinterpret_cast<unsigned char*>(const_cast<char*>(instr.data())),
    reinterpret_cast<unsigned char*>(&encrypted[0]),
    static_cast<int>(encrypted.size()));

  if (bytes <= 0)
  {
    return "";
  }

  encrypted.resize(bytes);
  std::string out;
  return decrypt(encrypted, out) < 0 ? "" : out;
}

void KBE_RSA::hexCertifData(const std::string& inCertifdata)
{
  for (unsigned char ch : inCertifdata)
  {
    std::fprintf(stderr, "%02x", ch);
  }
  std::fprintf(stderr, "\n");
}

} // namespace KBEngine
