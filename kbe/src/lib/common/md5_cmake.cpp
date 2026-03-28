// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "common/md5.h"

#include <cstdio>
#include <cstring>

namespace KBEngine
{

KBE_MD5::KBE_MD5() :
	state_(EVP_MD_CTX_new()),
	bytes_{0},
	isFinal_(false)
{
	EVP_DigestInit_ex(state_, EVP_md5(), NULL);
}

KBE_MD5::KBE_MD5(const void * data, int numBytes) :
	KBE_MD5()
{
	append(data, numBytes);
}

KBE_MD5::~KBE_MD5()
{
	if (state_ != NULL)
	{
		EVP_MD_CTX_free(state_);
		state_ = NULL;
	}
}

void KBE_MD5::append(const void * data, int numBytes)
{
	EVP_DigestUpdate(state_, data, static_cast<size_t>(numBytes));
}

const unsigned char* KBE_MD5::getDigest()
{
	final();
	return bytes_;
}

std::string KBE_MD5::getDigestStr()
{
	const unsigned char* md = getDigest();

	char md5str[33] = {'\0'};
	for (int i = 0; i < 16; ++i)
	{
		std::snprintf(md5str + (i * 2), 3, "%02X", md[i]);
	}

	return md5str;
}

void KBE_MD5::final()
{
	if (!isFinal_)
	{
		unsigned int out_len = sizeof(bytes_);
		EVP_DigestFinal_ex(state_, bytes_, &out_len);
		isFinal_ = true;
	}
}

void KBE_MD5::clear()
{
	std::memset(bytes_, 0, sizeof(bytes_));
	isFinal_ = false;

	EVP_MD_CTX_reset(state_);
	EVP_DigestInit_ex(state_, EVP_md5(), NULL);
}

bool KBE_MD5::operator==(const KBE_MD5 & other) const
{
	return std::memcmp(this->bytes_, other.bytes_, sizeof(bytes_)) == 0;
}

bool KBE_MD5::operator<(const KBE_MD5 & other) const
{
	return std::memcmp(this->bytes_, other.bytes_, sizeof(bytes_)) < 0;
}

std::string KBE_MD5::getDigest(const void * data, int numBytes)
{
	KBE_MD5 md5(data, numBytes);
	return md5.getDigestStr();
}

}
