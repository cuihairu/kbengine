// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "event_poller.h"
#include "poller_select.h"
#include "helper/profile.h"

#if KBE_PLATFORM == PLATFORM_UNIX
#define HAS_EPOLL
#endif

#if KBE_PLATFORM == PLATFORM_WIN32
#define HAS_IOCP
#endif

#ifdef HAS_EPOLL
#include "poller_epoll.h"
#endif

#ifdef HAS_IOCP
#include "poller_iocp.h"
#endif

namespace KBEngine { 
namespace Network
{

//-------------------------------------------------------------------------------------
EventPoller::EventPoller() : 
	fdReadHandlers_(), 
	fdWriteHandlers_(), 
	spareTime_(0)
{
}

//-------------------------------------------------------------------------------------
EventPoller::~EventPoller()
{
}

//-------------------------------------------------------------------------------------
bool EventPoller::registerForRead(int fd,
		InputNotificationHandler * handler)
{
	if (!this->doRegisterForRead(fd))
	{
		return false;
	}

	fdReadHandlers_[ fd ] = handler;

	return true;
}

//-------------------------------------------------------------------------------------
bool EventPoller::registerForWrite(int fd,
		OutputNotificationHandler * handler)
{
	if (!this->doRegisterForWrite(fd))
	{
		return false;
	}

	fdWriteHandlers_[ fd ] = handler;

	return true;
}

//-------------------------------------------------------------------------------------
bool EventPoller::deregisterForRead(int fd)
{
	fdReadHandlers_.erase(fd);

	return this->doDeregisterForRead(fd);
}

//-------------------------------------------------------------------------------------
bool EventPoller::deregisterForWrite(int fd)
{
	fdWriteHandlers_.erase(fd);

	return this->doDeregisterForWrite(fd);
}

//-------------------------------------------------------------------------------------
bool EventPoller::triggerRead(int fd)	
{
	FDReadHandlers::iterator iter = fdReadHandlers_.find(fd);

	if (iter == fdReadHandlers_.end())
	{
		return false;
	}

	iter->second->handleInputNotification(fd);

	return true;
}

//-------------------------------------------------------------------------------------
bool EventPoller::triggerWrite(int fd)	
{
	FDWriteHandlers::iterator iter = fdWriteHandlers_.find(fd);

	if (iter == fdWriteHandlers_.end())
	{
		return false;
	}

	iter->second->handleOutputNotification(fd);

	return true;
}

//-------------------------------------------------------------------------------------
bool EventPoller::triggerError(int fd)
{
	if (!this->triggerRead(fd))
	{
		return this->triggerWrite(fd);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EventPoller::isRegistered(int fd, bool isForRead) const
{
	return isForRead ? (fdReadHandlers_.find(fd) != fdReadHandlers_.end()) : 
		(fdWriteHandlers_.find(fd) != fdWriteHandlers_.end());
}

//-------------------------------------------------------------------------------------
InputNotificationHandler* EventPoller::findForRead(int fd)
{
	FDReadHandlers::iterator iter = fdReadHandlers_.find(fd);
	
	if(iter == fdReadHandlers_.end())
		return NULL;

	return iter->second;
}

//-------------------------------------------------------------------------------------
OutputNotificationHandler* EventPoller::findForWrite(int fd)
{
	FDWriteHandlers::iterator iter = fdWriteHandlers_.find(fd);
	
	if(iter == fdWriteHandlers_.end())
		return NULL;

	return iter->second;
}

//-------------------------------------------------------------------------------------
int EventPoller::getFileDescriptor() const
{
	return -1;
}

//-------------------------------------------------------------------------------------
int EventPoller::maxFD() const
{
	int readMaxFD = -1;

	FDReadHandlers::const_iterator iFDReadHandler = fdReadHandlers_.begin();
	while (iFDReadHandler != fdReadHandlers_.end())
	{
		if (iFDReadHandler->first > readMaxFD)
		{
			readMaxFD = iFDReadHandler->first;
		}

		++iFDReadHandler;
	}

	int writeMaxFD = -1;

	FDWriteHandlers::const_iterator iFDWriteHandler = fdWriteHandlers_.begin();
	while (iFDWriteHandler != fdWriteHandlers_.end())
	{
		if (iFDWriteHandler->first > writeMaxFD)
		{
			writeMaxFD = iFDWriteHandler->first;
		}

		++iFDWriteHandler;
	}

	return std::max(readMaxFD, writeMaxFD);
}

//-------------------------------------------------------------------------------------
EventPoller * EventPoller::create()
{
#if defined(HAS_IOCP)
	// Windows 平台优先使用 IOCP
	return new IocpPoller();
#elif defined(HAS_EPOLL)
	// Unix/Linux 平台使用 epoll
	return new EpollPoller();
#else
	// 其他平台使用 select
	return new SelectPoller();
#endif
}

}
}
