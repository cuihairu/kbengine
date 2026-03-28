// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "network/address.h"

#include <cstdio>

namespace KBEngine {
namespace Network {

char Address::s_stringBuf[2][32] = {{0}, {0}};
int Address::s_currStringBuf = 0;
const Address Address::NONE(0, 0);

static ObjectPool<Address> g_objPool("Address");

ObjectPool<Address>& Address::ObjPool()
{
  return g_objPool;
}

Address* Address::createPoolObject(const std::string& logPoint)
{
  return g_objPool.createObject(logPoint);
}

void Address::reclaimPoolObject(Address* obj)
{
  g_objPool.reclaimObject(obj);
}

void Address::destroyObjPool()
{
  g_objPool.destroy();
}

Address::SmartPoolObjectPtr Address::createSmartPoolObj(const std::string& logPoint)
{
  return SmartPoolObjectPtr(new SmartPoolObject<Address>(ObjPool().createObject(logPoint), g_objPool));
}

void Address::onReclaimObject()
{
  ip = 0;
  port = 0;
}

Address::Address(std::string ipArg, uint16 portArg) :
  ip(0),
  port(htons(portArg))
{
  if (ipArg == "localhost")
  {
    ipArg = "127.0.0.1";
  }

  u_int32_t addr = 0;
  Network::Address::string2ip(ipArg.c_str(), addr);
  ip = static_cast<uint32>(addr);
}

Address::Address(const Address& addr) :
  ip(addr.ip),
  port(addr.port)
{
}

Address::~Address()
{
}

int Address::writeToString(char* str, int length) const
{
  const uint32 hip = ntohl(ip);
  const uint16 hport = ntohs(port);

  return kbe_snprintf(
    str,
    length,
    "%d.%d.%d.%d:%d",
    static_cast<int>(static_cast<uchar>(hip >> 24)),
    static_cast<int>(static_cast<uchar>(hip >> 16)),
    static_cast<int>(static_cast<uchar>(hip >> 8)),
    static_cast<int>(static_cast<uchar>(hip)),
    static_cast<int>(hport));
}

char* Address::c_str() const
{
  char* buf = Address::nextStringBuf();
  this->writeToString(buf, 32);
  return buf;
}

const char* Address::ipAsString() const
{
  const uint32 hip = ntohl(ip);
  char* buf = Address::nextStringBuf();

  kbe_snprintf(
    buf,
    32,
    "%d.%d.%d.%d",
    static_cast<int>(static_cast<uchar>(hip >> 24)),
    static_cast<int>(static_cast<uchar>(hip >> 16)),
    static_cast<int>(static_cast<uchar>(hip >> 8)),
    static_cast<int>(static_cast<uchar>(hip)));

  return buf;
}

char* Address::nextStringBuf()
{
  s_currStringBuf = (s_currStringBuf + 1) % 2;
  return s_stringBuf[s_currStringBuf];
}

int Address::string2ip(const char* string, u_int32_t& address)
{
  u_int32_t trial = 0;

#if KBE_PLATFORM == PLATFORM_UNIX
  if (inet_aton(string, reinterpret_cast<struct in_addr*>(&trial)) != 0)
#else
  if ((trial = inet_addr(string)) != INADDR_NONE)
#endif
  {
    address = trial;
    return 0;
  }

  hostent* hosts = gethostbyname(string);
  if (hosts != NULL)
  {
    address = *reinterpret_cast<u_int32_t*>(hosts->h_addr_list[0]);
    return 0;
  }

  return -1;
}

int Address::ip2string(u_int32_t address, char* string)
{
  address = ntohl(address);

  const int p1 = address >> 24;
  const int p2 = (address & 0xffffff) >> 16;
  const int p3 = (address & 0xffff) >> 8;
  const int p4 = (address & 0xff);

  return std::snprintf(string, 32, "%d.%d.%d.%d", p1, p2, p3, p4);
}

} // namespace Network
} // namespace KBEngine
