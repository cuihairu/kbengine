// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_EVENT_DISPATCHER_CMAKE_H
#define KBE_EVENT_DISPATCHER_CMAKE_H

#include "common/timer.h"

namespace KBEngine {
namespace Network {

class EventDispatcher
{
public:
  EventDispatcher() = default;

  TimerHandle addTimer(int64, TimerHandler*, void* = NULL)
  {
    return TimerHandle();
  }
};

} // namespace Network
} // namespace KBEngine

#endif // KBE_EVENT_DISPATCHER_CMAKE_H
