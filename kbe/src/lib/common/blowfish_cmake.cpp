// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "common/blowfish.h"

#include <cstdio>
#include <cstdint>
#include <cstring>

#include <openssl/rand.h>

namespace KBEngine
{

KBEBlowfish::KBEBlowfish(const Key& key) :
  key_(key),
  keySize_(static_cast<int>(key.size())),
  isGood_(false),
  pBlowFishKey_(NULL)
{
  init();
}

KBEBlowfish::KBEBlowfish(int keySize) :
  key_(keySize, 0),
  keySize_(keySize),
  isGood_(false),
  pBlowFishKey_(NULL)
{
  RAND_bytes(reinterpret_cast<unsigned char*>(&key_[0]), static_cast<int>(key_.size()));
  init();
}

KBEBlowfish::~KBEBlowfish()
{
  delete pBlowFishKey();
  pBlowFishKey_ = NULL;
}

bool KBEBlowfish::init()
{
  pBlowFishKey_ = new BF_KEY;

  if ((MIN_KEY_SIZE <= keySize_) && (keySize_ <= MAX_KEY_SIZE))
  {
    BF_set_key(this->pBlowFishKey(), static_cast<int>(key_.size()),
      reinterpret_cast<const unsigned char*>(key_.data()));
    isGood_ = true;
  }
  else
  {
    isGood_ = false;
  }

  return isGood_;
}

const char* KBEBlowfish::strBlowFishKey() const
{
  static char buf[1024];
  char* c = buf;

  for (int i = 0; i < keySize_; ++i)
  {
    c += std::snprintf(c, static_cast<size_t>(buf + sizeof(buf) - c), "%02hhX ",
      static_cast<unsigned char>(key_[i]));
  }

  if (c != buf)
  {
    c[-1] = '\0';
  }
  else
  {
    *c = '\0';
  }

  return buf;
}

int KBEBlowfish::encrypt(const unsigned char* src, unsigned char* dest, int length)
{
  if (length % BLOCK_SIZE != 0)
  {
    return -1;
  }

  std::uint64_t prev_block = 0;
  bool has_prev = false;
  for (int i = 0; i < length; i += BLOCK_SIZE)
  {
    std::uint64_t current_block = 0;
    std::memcpy(&current_block, src + i, BLOCK_SIZE);

    if (has_prev)
    {
      current_block ^= prev_block;
    }

    std::memcpy(dest + i, &current_block, BLOCK_SIZE);

    BF_ecb_encrypt(dest + i, dest + i, this->pBlowFishKey(), BF_ENCRYPT);
    std::memcpy(&prev_block, dest + i, BLOCK_SIZE);
    has_prev = true;
  }

  return length;
}

int KBEBlowfish::decrypt(const unsigned char* src, unsigned char* dest, int length)
{
  if (length % BLOCK_SIZE != 0)
  {
    return -1;
  }

  std::uint64_t prev_block = 0;
  bool has_prev = false;

  for (int i = 0; i < length; i += BLOCK_SIZE)
  {
    BF_ecb_encrypt(src + i, dest + i, this->pBlowFishKey(), BF_DECRYPT);

    if (has_prev)
    {
      std::uint64_t plain_block = 0;
      std::memcpy(&plain_block, dest + i, BLOCK_SIZE);
      plain_block ^= prev_block;
      std::memcpy(dest + i, &plain_block, BLOCK_SIZE);
    }

    std::memcpy(&prev_block, src + i, BLOCK_SIZE);
    has_prev = true;
  }

  return length;
}

} // namespace KBEngine
