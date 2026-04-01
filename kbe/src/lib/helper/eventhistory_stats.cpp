// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "eventhistory_stats.h"
#include "debug_helper.h"

namespace
{
constexpr unsigned int KBE_EVENT_HISTORY_PACKET_MAX_SIZE_TCP = 1460u;
constexpr unsigned int KBE_EVENT_HISTORY_NETWORK_MESSAGE_MAX_SIZE = 65535u;

KBEngine::EventHistoryStatsTrigger g_eventHistoryStatsTrigger = nullptr;

#define KBE_EVENT_HISTORY_WARNING_MSG(m) do { if (KBEngine::DebugHelper::isInit()) { WARNING_MSG(m); } } while (0)
#define KBE_EVENT_HISTORY_ERROR_MSG(m) do { if (KBEngine::DebugHelper::isInit()) { ERROR_MSG(m); } } while (0)
}

namespace KBEngine { 

void setEventHistoryStatsTrigger(EventHistoryStatsTrigger trigger)
{
	g_eventHistoryStatsTrigger = trigger;
}

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
	
	if(size >= KBE_EVENT_HISTORY_PACKET_MAX_SIZE_TCP)
	{
		if(size < KBE_EVENT_HISTORY_NETWORK_MESSAGE_MAX_SIZE)
		{
			KBE_EVENT_HISTORY_WARNING_MSG(fmt::format("EventHistoryStats::trackEvent[{}]: message size({}) >= PACKET_MAX_SIZE_TCP({}).\n",
				fullname, size, KBE_EVENT_HISTORY_PACKET_MAX_SIZE_TCP));
		}
		else
		{
			KBE_EVENT_HISTORY_ERROR_MSG(fmt::format("EventHistoryStats::trackEvent[{}]: message size({}) > NETWORK_MESSAGE_MAX_SIZE({}).\n",
				fullname, size, KBE_EVENT_HISTORY_NETWORK_MESSAGE_MAX_SIZE));
		}
	}

	STATS::iterator iter = stats_.find(fullname);
	if(iter == stats_.end())
	{
		stats_[fullname].name = fullname;
		stats_[fullname].size += size;
		stats_[fullname].count++;

		if(g_eventHistoryStatsTrigger != nullptr)
		{
			g_eventHistoryStatsTrigger(*this, stats_[fullname], size);
		}

		return;
	}

	iter->second.size += size;
	iter->second.count++;

	if(g_eventHistoryStatsTrigger != nullptr)
	{
		g_eventHistoryStatsTrigger(*this, iter->second, size);
	}
}

//-------------------------------------------------------------------------------------
}
