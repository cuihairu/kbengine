#ifndef KBE_PACKET_FILTER_CMAKE_H
#define KBE_PACKET_FILTER_CMAKE_H

#include "network/packet_filter.h"

namespace KBEngine {
namespace Network {

class Channel
{
};

class Packet
{
};

class PacketSender
{
public:
  virtual ~PacketSender() = default;
  virtual Reason processFilterPacket(Channel* pChannel, Packet* pPacket, int userarg) = 0;
};

class PacketReceiver
{
public:
  virtual ~PacketReceiver() = default;
  virtual Reason processFilteredPacket(Channel* pChannel, Packet* pPacket) = 0;
};

}  // namespace Network
}  // namespace KBEngine

#endif  // KBE_PACKET_FILTER_CMAKE_H
