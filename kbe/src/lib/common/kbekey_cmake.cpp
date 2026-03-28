// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "common/kbekey.h"

#include <cassert>

#include "common/common.h"

namespace KBEngine
{

KBE_SINGLETON_INIT(KBEKey);

KBEKey::KBEKey(const std::string& pubkeyname, const std::string& prikeyname) :
  KBE_RSA()
{
  if (pubkeyname.empty() && prikeyname.empty())
  {
    return;
  }

  assert(!pubkeyname.empty());

  if (g_componentType != CLIENT_TYPE)
  {
    assert(!prikeyname.empty());

    bool has_private = loadPrivate(prikeyname);
    bool has_public = loadPublic(pubkeyname);
    assert(has_private == has_public);

    if (!has_private && !has_public)
    {
      const bool generated = generateKey(pubkeyname, prikeyname);
      assert(generated);

      has_private = loadPrivate(prikeyname);
      has_public = loadPublic(pubkeyname);
      assert(has_private && has_public);
    }
  }
  else
  {
    const bool has_public = loadPublic(pubkeyname);
    assert(has_public);
  }
}

KBEKey::KBEKey() :
  KBE_RSA()
{
}

KBEKey::~KBEKey()
{
}

bool KBEKey::isGood() const
{
  if (g_componentType == CLIENT_TYPE)
  {
    return rsa_public != NULL;
  }

  return rsa_public != NULL && rsa_private != NULL;
}

} // namespace KBEngine
