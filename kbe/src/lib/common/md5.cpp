// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "md5.h"
#include "common.h"

namespace KBEngine
{

//-------------------------------------------------------------------------------------
KBE_MD5::KBE_MD5()
{
#if defined(KBE_USE_EVP_MD5)
	state_ = EVP_MD_CTX_new();
	EVP_DigestInit_ex(state_, EVP_md5(), NULL);
#else
	MD5_Init(&state_);
#endif
	isFinal_ = false;
}

//-------------------------------------------------------------------------------------
KBE_MD5::KBE_MD5(const void * data, int numBytes)
{
#if defined(KBE_USE_EVP_MD5)
	state_ = EVP_MD_CTX_new();
	EVP_DigestInit_ex(state_, EVP_md5(), NULL);
#else
	MD5_Init(&state_);
#endif
	isFinal_ = false;

	append(data, numBytes);
}

//-------------------------------------------------------------------------------------
KBE_MD5::~KBE_MD5()
{
#if defined(KBE_USE_EVP_MD5)
	if (state_ != NULL)
	{
		EVP_MD_CTX_free(state_);
		state_ = NULL;
	}
#endif
}

//-------------------------------------------------------------------------------------
void KBE_MD5::append(const void * data, int numBytes)
{
#if defined(KBE_USE_EVP_MD5)
	EVP_DigestUpdate(state_, data, static_cast<size_t>(numBytes));
#else
	MD5_Update(&state_, (const unsigned char*)data, numBytes);
#endif
}

//-------------------------------------------------------------------------------------
const unsigned char* KBE_MD5::getDigest()
{
	final();
	return bytes_;
}

//-------------------------------------------------------------------------------------
std::string KBE_MD5::getDigestStr()
{
	const unsigned char* md = getDigest();

	char tmp[3]={'\0'}, md5str[33] = {'\0'};
	for (int i = 0; i < 16; ++i)
	{
		snprintf(tmp, sizeof(tmp), "%2.2X", md[i]);
		strcat(md5str, tmp);
	}

	return md5str;
}

//-------------------------------------------------------------------------------------
void KBE_MD5::final()
{
	if(!isFinal_)
	{
#if defined(KBE_USE_EVP_MD5)
		unsigned int outLen = sizeof(bytes_);
		EVP_DigestFinal_ex(state_, bytes_, &outLen);
#else
		MD5_Final(bytes_, &state_);
#endif
		isFinal_ = true;
	}
}

//-------------------------------------------------------------------------------------
void KBE_MD5::clear()
{
#if defined(KBE_USE_EVP_MD5)
	memset(bytes_, 0, sizeof(bytes_));
	isFinal_ = false;
	EVP_MD_CTX_reset(state_);
	EVP_DigestInit_ex(state_, EVP_md5(), NULL);
#else
	memset(this, 0, sizeof(*this));
	MD5_Init(&state_);
	isFinal_ = false;
#endif
}

//-------------------------------------------------------------------------------------
bool KBE_MD5::operator==(const KBE_MD5 & other) const
{
	return memcmp(this->bytes_, other.bytes_, sizeof(bytes_)) == 0;
}

//-------------------------------------------------------------------------------------
bool KBE_MD5::operator<(const KBE_MD5 & other) const
{
	return memcmp(this->bytes_, other.bytes_, sizeof(bytes_)) < 0;
}

//-------------------------------------------------------------------------------------
std::string KBE_MD5::getDigest(const void * data, int numBytes)
{
	KBE_MD5 md5 = KBE_MD5(data, numBytes);
	return md5.getDigestStr();
}

//-------------------------------------------------------------------------------------
} 
