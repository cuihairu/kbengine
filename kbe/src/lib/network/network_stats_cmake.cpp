// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "network/network_stats.h"

#include "network/message_handler.h"

namespace KBEngine {

KBE_SINGLETON_INIT(Network::NetworkStats);

namespace Network {

NetworkStats g_networkStats;

NetworkStats::NetworkStats() :
  stats_(),
  handlers_()
{
}

NetworkStats::~NetworkStats()
{
}

void NetworkStats::addHandler(NetworkStatsHandler* pHandler)
{
  handlers_.push_back(pHandler);
}

void NetworkStats::removeHandler(NetworkStatsHandler* pHandler)
{
  std::vector<NetworkStatsHandler*>::iterator iter = handlers_.begin();
  for (; iter != handlers_.end(); ++iter)
  {
    if (*iter == pHandler)
    {
      handlers_.erase(iter);
      break;
    }
  }
}

void NetworkStats::trackMessage(S_OP op, const MessageHandler& msgHandler, uint32 size)
{
  MessageHandler* pMsgHandler = const_cast<MessageHandler*>(&msgHandler);
  Stats& stats = stats_[msgHandler.name];
  stats.name = msgHandler.name;

  if (op == SEND)
  {
    pMsgHandler->send_size += size;
    pMsgHandler->send_count++;
    stats.send_size += size;
    stats.send_count++;
  }
  else
  {
    pMsgHandler->recv_size += size;
    pMsgHandler->recv_count++;
    stats.recv_size += size;
    stats.recv_count++;
  }

  std::vector<NetworkStatsHandler*>::iterator iter = handlers_.begin();
  for (; iter != handlers_.end(); ++iter)
  {
    if (op == SEND)
    {
      (*iter)->onSendMessage(msgHandler, static_cast<int>(size));
    }
    else
    {
      (*iter)->onRecvMessage(msgHandler, static_cast<int>(size));
    }
  }
}

} // namespace Network
} // namespace KBEngine
