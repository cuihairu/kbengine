// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
//
// OpenSSL 3.x compatible implementation

#include "rsa.h"
#include "common.h"
#include "strutil.h"
#include "helper/debug_helper.h"

#include <iostream>
#include <fstream>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>

// Fix Windows GUI application OpenSSL compatibility
#ifdef _WIN32
	#ifndef OPENSSL_UPLINK
		#include <openssl/applink.c>
	#endif
#endif

namespace KBEngine
{

namespace
{
void log_rsa_error(const std::string& message)
{
	if(DebugHelper::isInit())
	{
		ERROR_MSG(message);
	}
}

void log_rsa_info(const std::string& message)
{
	if(DebugHelper::isInit())
	{
		INFO_MSG(message);
	}
}

// Helper to get RSA from EVP_PKEY
RSA* EVP_PKEY_get_RSA(EVP_PKEY* pkey)
{
	if (!pkey) return NULL;
	#if OPENSSL_VERSION_NUMBER >= 0x30000000L
		// OpenSSL 3.x: Use EVP_PKEY_get0_RSA with correct signature
		return (RSA*)EVP_PKEY_get0_RSA(pkey);
	#else
		return EVP_PKEY_get0_RSA(pkey);
	#endif
}
}

//-------------------------------------------------------------------------------------
KBE_RSA::KBE_RSA(const std::string& pubkeyname, const std::string& prikeyname):
rsa_public(0),
rsa_private(0)
{
	if(pubkeyname.size() > 0 || prikeyname.size() > 0)
	{
		KBE_ASSERT(pubkeyname.size() > 0);
		KBE_ASSERT(prikeyname.size() > 0);

		bool key = loadPrivate(prikeyname) && loadPublic(pubkeyname);
		KBE_ASSERT(key);
	}
}

//-------------------------------------------------------------------------------------
KBE_RSA::KBE_RSA():
rsa_public(0),
rsa_private(0)
{
}

//-------------------------------------------------------------------------------------
KBE_RSA::~KBE_RSA()
{
	if(rsa_public != NULL)
	{
		EVP_PKEY_free(static_cast<EVP_PKEY*>(rsa_public));
		rsa_public = NULL;
	}

	if(rsa_private != NULL)
	{
		EVP_PKEY_free(static_cast<EVP_PKEY*>(rsa_private));
		rsa_private = NULL;
	}
}

//-------------------------------------------------------------------------------------
bool KBE_RSA::loadPublic(const std::string& keyname)
{
    if(rsa_public == NULL)
    {
		FILE *fp = fopen(keyname.c_str(), "rb");
		if (!fp) {
			return false;
		}

		#if OPENSSL_VERSION_NUMBER >= 0x30000000L
		// OpenSSL 3.x: Use EVP_PKEY API
		EVP_PKEY* pkey = PEM_read_PUBKEY(fp, NULL, NULL, NULL);
		#else
		// OpenSSL 1.x: Fall back to RSA API
		EVP_PKEY* pkey = PEM_read_PUBKEY(fp, NULL, NULL, NULL);
		#endif

		if(NULL == pkey)
		{
			char err[1024];
			char* errret = ERR_error_string(ERR_get_error(), err);
			log_rsa_error(fmt::format("KBE_RSA::loadPublic: PEM_read_PUBKEY error({} : {})\n",
				errret, err));

			fclose(fp);
			return false;
		}

		rsa_public = pkey;
		fclose(fp);
	}

	return rsa_public != NULL;
}

//-------------------------------------------------------------------------------------
bool KBE_RSA::loadPrivate(const std::string& keyname)
{
    if(rsa_private == NULL)
    {
		FILE *fp = fopen(keyname.c_str(), "rb");
		if (!fp) {
			return false;
		}

		#if OPENSSL_VERSION_NUMBER >= 0x30000000L
		// OpenSSL 3.x: Use EVP_PKEY API
		EVP_PKEY* pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
		#else
		// OpenSSL 1.x: Use PEM_read_RSAPrivateKey
		EVP_PKEY* pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
		#endif

		if(NULL == pkey)
		{
			char err[1024];
			char* errret = ERR_error_string(ERR_get_error(), err);
			log_rsa_error(fmt::format("KBE_RSA::loadPrivate: PEM_read_PrivateKey error({} : {})\n",
				errret, err));

			fclose(fp);
			return false;
		}

		rsa_private = pkey;
		fclose(fp);
	}

	return rsa_private != NULL;
}

//-------------------------------------------------------------------------------------
bool KBE_RSA::generateKey(const std::string& pubkeyname,
						  const std::string& prikeyname, int keySize, int e)
{
	KBE_ASSERT(rsa_public == NULL && rsa_private == NULL);

	EVP_PKEY* pkey = NULL;
    FILE *fp = NULL;

	#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	// OpenSSL 3.x: Use EVP_PKEY key generation
	EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);

	if (!ctx || EVP_PKEY_keygen_init(ctx) <= 0)
	{
		char err[1024];
		char* errret = ERR_error_string(ERR_get_error(), err);
		log_rsa_error(fmt::format("KBE_RSA::generateKey: EVP_PKEY_keygen_init error({} : {})\n",
			errret, err));

		if(ctx) EVP_PKEY_CTX_free(ctx);
		return false;
	}

	if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, keySize) <= 0)
	{
		char err[1024];
		char* errret = ERR_error_string(ERR_get_error(), err);
		log_rsa_error(fmt::format("KBE_RSA::generateKey: EVP_PKEY_CTX_set_rsa_keygen_bits error({} : {})\n",
			errret, err));

		EVP_PKEY_CTX_free(ctx);
		return false;
	}

	if (EVP_PKEY_keygen(ctx, &pkey) <= 0)
	{
		char err[1024];
		char* errret = ERR_error_string(ERR_get_error(), err);
		log_rsa_error(fmt::format("KBE_RSA::generateKey: EVP_PKEY_keygen error({} : {})\n",
			errret, err));

		EVP_PKEY_CTX_free(ctx);
		return false;
	}

	EVP_PKEY_CTX_free(ctx);

	#else
	// OpenSSL 1.x: Use RSA_generate_key_ex
	BIGNUM *bne = BN_new();
	if (bne && BN_set_word(bne, e) == 1)
	{
		RSA* rsa = RSA_new();
		if (rsa && RSA_generate_key_ex(rsa, keySize, bne, NULL) == 1)
		{
			pkey = EVP_PKEY_new();
			EVP_PKEY_assign_RSA(pkey, rsa);
		}
		else if (rsa)
		{
			RSA_free(rsa);
		}
	}
	BN_free(bne);

	if (!pkey)
	{
		char err[1024];
		char* errret = ERR_error_string(ERR_get_error(), err);
		log_rsa_error(fmt::format("KBE_RSA::generateKey: RSA_generate_key_ex error({} : {})\n",
			errret, err));
		return false;
	}
	#endif

	// Write private key
	fp = fopen(prikeyname.c_str(), "w");
	if (!fp) {
		EVP_PKEY_free(pkey);
		return false;
	}

	if (!PEM_write_PrivateKey(fp, pkey, NULL, NULL, 0, 0, NULL))
	{
		char err[1024];
		char* errret = ERR_error_string(ERR_get_error(), err);
		log_rsa_error(fmt::format("KBE_RSA::generateKey: PEM_write_PrivateKey error({} : {})\n",
			errret, err));

		fclose(fp);
		EVP_PKEY_free(pkey);
		return false;
	}

	fclose(fp);

	// Write public key
	fp = fopen(pubkeyname.c_str(), "w");
	if (!fp) {
		EVP_PKEY_free(pkey);
		return false;
	}

	if (!PEM_write_PUBKEY(fp, pkey))
	{
		char err[1024];
		char* errret = ERR_error_string(ERR_get_error(), err);
		log_rsa_error(fmt::format("KBE_RSA::generateKey: PEM_write_PUBKEY error({} : {})\n",
			errret, err));

		fclose(fp);
		EVP_PKEY_free(pkey);
		return false;
	}

	log_rsa_info(fmt::format("KBE_RSA::generateKey: RSA key generated. keysize({}) bits.\n", keySize));

	EVP_PKEY_free(pkey);
	fclose(fp);

	return loadPrivate(prikeyname) && loadPublic(pubkeyname);
}

//-------------------------------------------------------------------------------------
std::string KBE_RSA::encrypt(const std::string& instr)
{
	std::string encrypted;
	if(encrypt(instr, encrypted) < 0)
		return "";

	// Calculate required buffer size (2 bytes per byte for hex encoding)
	size_t required_size = encrypted.size() * 2 + 1;
	char* strencrypted = new char[required_size];
	memset(strencrypted, 0, required_size);
	strutil::bytes2string((unsigned char *)encrypted.data(), encrypted.size(), (unsigned char *)strencrypted, required_size);
	std::string result(strencrypted);
	delete[] strencrypted;
	return result;
}

//-------------------------------------------------------------------------------------
int KBE_RSA::encrypt(const std::string& instr, std::string& outCertifdata)
{
	KBE_ASSERT(rsa_public != NULL);

	EVP_PKEY* pkey = static_cast<EVP_PKEY*>(rsa_public);
	EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, NULL);

	if (!ctx || EVP_PKEY_encrypt_init(ctx) <= 0)
	{
		char err[1024];
		char* errret = ERR_error_string(ERR_get_error(), err);
		log_rsa_error(fmt::format("KBE_RSA::encrypt: EVP_PKEY_encrypt_init error({} : {})\n",
			errret, err));

		if(ctx) EVP_PKEY_CTX_free(ctx);
		return -1;
	}

	if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
	{
		char err[1024];
		char* errret = ERR_error_string(ERR_get_error(), err);
		log_rsa_error(fmt::format("KBE_RSA::encrypt: EVP_PKEY_CTX_set_rsa_padding error({} : {})\n",
			errret, err));

		EVP_PKEY_CTX_free(ctx);
		return -1;
	}

	size_t outlen;
	if (EVP_PKEY_encrypt(ctx, NULL, &outlen,
		(const unsigned char*)instr.c_str(), instr.size()) <= 0)
	{
		char err[1024];
		char* errret = ERR_error_string(ERR_get_error(), err);
		log_rsa_error(fmt::format("KBE_RSA::encrypt: EVP_PKEY_encrypt (determine size) error({} : {})\n",
			errret, err));

		EVP_PKEY_CTX_free(ctx);
		return -1;
	}

	unsigned char* out = (unsigned char*)OPENSSL_malloc(outlen);
	if (!out)
	{
		EVP_PKEY_CTX_free(ctx);
		return -1;
	}

	if (EVP_PKEY_encrypt(ctx, out, &outlen,
		(const unsigned char*)instr.c_str(), instr.size()) <= 0)
	{
		char err[1024];
		char* errret = ERR_error_string(ERR_get_error(), err);
		log_rsa_error(fmt::format("KBE_RSA::encrypt: EVP_PKEY_encrypt error({} : {})\n",
			errret, err));

		OPENSSL_free(out);
		EVP_PKEY_CTX_free(ctx);
		return -1;
	}

	outCertifdata.assign((const char*)out, outlen);
	OPENSSL_free(out);
	EVP_PKEY_CTX_free(ctx);

	return outlen;
}

//-------------------------------------------------------------------------------------
void KBE_RSA::hexCertifData(const std::string& inCertifdata)
{
	std::string s = "KBE_RSA::encrypt: encrypted string = \n";

	for (int i=0; i<(int)inCertifdata.size(); ++i) {
		s += fmt::format("{:x}{:x}", ((inCertifdata.data()[i] >> 4) & 0xf),
			(inCertifdata.data()[i] & 0xf));
	}

	s += "\n";

	log_rsa_info(s.c_str());
}

//-------------------------------------------------------------------------------------
int KBE_RSA::decrypt(const std::string& inCertifdata, std::string& outstr)
{
	KBE_ASSERT(rsa_private != NULL);

	EVP_PKEY* pkey = static_cast<EVP_PKEY*>(rsa_private);
	EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, NULL);

	if (!ctx || EVP_PKEY_decrypt_init(ctx) <= 0)
	{
		char err[1024];
		char* errret = ERR_error_string(ERR_get_error(), err);
		log_rsa_error(fmt::format("KBE_RSA::decrypt: EVP_PKEY_decrypt_init error({} : {})\n",
			errret, err));

		if(ctx) EVP_PKEY_CTX_free(ctx);
		return -1;
	}

	if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
	{
		char err[1024];
		char* errret = ERR_error_string(ERR_get_error(), err);
		log_rsa_error(fmt::format("KBE_RSA::decrypt: EVP_PKEY_CTX_set_rsa_padding error({} : {})\n",
			errret, err));

		EVP_PKEY_CTX_free(ctx);
		return -1;
	}

	size_t outlen;
	if (EVP_PKEY_decrypt(ctx, NULL, &outlen,
		(const unsigned char*)inCertifdata.data(), inCertifdata.size()) <= 0)
	{
		char err[1024];
		char* errret = ERR_error_string(ERR_get_error(), err);
		log_rsa_error(fmt::format("KBE_RSA::decrypt: EVP_PKEY_decrypt (determine size) error({} : {})\n",
			errret, err));

		EVP_PKEY_CTX_free(ctx);
		return -1;
	}

	unsigned char* out = (unsigned char*)OPENSSL_malloc(outlen);
	if (!out)
	{
		EVP_PKEY_CTX_free(ctx);
		return -1;
	}

	if (EVP_PKEY_decrypt(ctx, out, &outlen,
		(const unsigned char*)inCertifdata.data(), inCertifdata.size()) <= 0)
	{
		char err[1024];
		char* errret = ERR_error_string(ERR_get_error(), err);
		log_rsa_error(fmt::format("KBE_RSA::decrypt: EVP_PKEY_decrypt error({} : {})\n",
			errret, err));

		OPENSSL_free(out);
		EVP_PKEY_CTX_free(ctx);
		return -1;
	}

	outstr.assign((const char*)out, outlen);
	OPENSSL_free(out);
	EVP_PKEY_CTX_free(ctx);

	return outlen;
}

//-------------------------------------------------------------------------------------
std::string KBE_RSA::decrypt(const std::string& instr)
{
	// Calculate required buffer size (hex string is 2x the binary size)
	size_t binary_size = instr.length() / 2;
	unsigned char* strencrypted = new unsigned char[binary_size];
	memset(strencrypted, 0, binary_size);
	strutil::string2bytes((unsigned char *)instr.data(), strencrypted, binary_size);
	std::string encrypted;
	encrypted.assign((char*)strencrypted, binary_size);
	delete[] strencrypted;

	std::string out;
	if(decrypt(encrypted, out) < 0)
		return "";

	return out;
}

//-------------------------------------------------------------------------------------
}
