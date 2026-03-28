// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "network/message_handler.h"

namespace KBEngine {
namespace Network {

MessageHandler::MessageHandler() :
  name(),
  msgID(0),
  pArgs(NULL),
  msgLen(0),
  exposed(false),
  pMessageHandlers(NULL),
  send_size(0),
  send_count(0),
  recv_size(0),
  recv_count(0)
{
}

MessageHandler::~MessageHandler()
{
  SAFE_RELEASE(pArgs);
}

const char* MessageHandler::c_str()
{
  static char buf[MAX_BUF];
  kbe_snprintf(buf, MAX_BUF, "id:%u, len:%d", msgID, msgLen);
  return buf;
}

} // namespace Network
} // namespace KBEngine
