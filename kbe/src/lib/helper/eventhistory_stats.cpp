// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "eventhistory_stats.h"

#ifdef KBE_CMAKE_BOOTSTRAP_EVENT_STATS
#include <cassert>
#define DEBUG_MSG(m) do { } while (0)
#define INFO_MSG(m) do { } while (0)
#define WARNING_MSG(m) do { } while (0)
#define ERROR_MSG(m) do { } while (0)
#define CRITICAL_MSG(m) do { } while (0)
#define KBE_ASSERT(exp) assert((exp))
namespace {
constexpr unsigned int PACKET_MAX_SIZE_TCP = 1460u;
constexpr unsigned int NETWORK_MESSAGE_MAX_SIZE = 65535u;
}
#else
#include "profile_handler.h"
#endif

namespace KBEngine { 

//-------------------------------------------------------------------------------------
EventHistoryStats::EventHistoryStats(std::string name):
stats_(),
name_(name)
{
}

//-------------------------------------------------------------------------------------
EventHistoryStats::~EventHistoryStats()
{
}

//-------------------------------------------------------------------------------------
void EventHistoryStats::trackEvent(const std::string& type, const std::string& name, uint32 size, const char* flags)
{
	std::string fullname = type + flags + name;
	
	if(size >= PACKET_MAX_SIZE_TCP)
	{
		if(size < NETWORK_MESSAGE_MAX_SIZE)
		{
			WARNING_MSG(fmt::format("EventHistoryStats::trackEvent[{}]: message size({}) >= PACKET_MAX_SIZE_TCP({}).\n",
				fullname, size, PACKET_MAX_SIZE_TCP));
		}
		else
		{
			ERROR_MSG(fmt::format("EventHistoryStats::trackEvent[{}]: message size({}) > NETWORK_MESSAGE_MAX_SIZE({}).\n",
				fullname, size, NETWORK_MESSAGE_MAX_SIZE));
		}
	}

	STATS::iterator iter = stats_.find(fullname);
	if(iter == stats_.end())
	{
		stats_[fullname].name = fullname;
		stats_[fullname].size += size;
		stats_[fullname].count++;
#ifndef KBE_CMAKE_BOOTSTRAP_EVENT_STATS
		EventProfileHandler::triggerEvent(*this, stats_[fullname], size);
#endif
		return;
	}

	iter->second.size += size;
	iter->second.count++;
	
#ifndef KBE_CMAKE_BOOTSTRAP_EVENT_STATS
	EventProfileHandler::triggerEvent(*this, iter->second, size);
#endif
}

//-------------------------------------------------------------------------------------
}
