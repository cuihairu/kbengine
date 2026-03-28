#include "network/packet_filter_cmake.h"

namespace KBEngine {
namespace Network {

Reason PacketFilter::send(Channel* pChannel, PacketSender& sender, Packet* pPacket, int userarg)
{
  return sender.processFilterPacket(pChannel, pPacket, userarg);
}

Reason PacketFilter::recv(Channel* pChannel, PacketReceiver& receiver, Packet* pPacket)
{
  return receiver.processFilteredPacket(pChannel, pPacket);
}

}  // namespace Network
}  // namespace KBEngine
