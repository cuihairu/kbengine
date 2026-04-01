// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_THREADPOOL_H
#define KBE_THREADPOOL_H

#include "common/common.h"
#include "common/tasks.h"
#include <cstdio>
#include "helper/debug_helper.h"
#include "thread/threadtask.h"

#define KBE_THREADPOOL_DEBUG_MSG(m) do { if (KBEngine::DebugHelper::isInit()) { DEBUG_MSG(m); } } while (0)
#define KBE_THREADPOOL_INFO_MSG(m) do { if (KBEngine::DebugHelper::isInit()) { INFO_MSG(m); } } while (0)
#define KBE_THREADPOOL_WARNING_MSG(m) do { if (KBEngine::DebugHelper::isInit()) { WARNING_MSG(m); } } while (0)
#define KBE_THREADPOOL_ERROR_MSG(m) do { if (KBEngine::DebugHelper::isInit()) { ERROR_MSG(m); } } while (0)
#define KBE_THREADPOOL_CRITICAL_MSG(m) do { if (KBEngine::DebugHelper::isInit()) { CRITICAL_MSG(m); } } while (0)
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#include <windows.h>          // for HANDLE
#include <process.h>          // for _beginthread()	
#include "helper/crashhandler.h"
#else
// linux include
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <fcntl.h>
#if defined(__linux__)
#include <sys/epoll.h>
#endif
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>	
#endif
	
namespace KBEngine{ namespace thread{

// Ïß³Ì³Ø»î¶¯Ïß³Ì´óÓÚÕâ¸öÊýÄ¿Ôò´¦ÓÚ·±Ã¦×´Ì¬
#define THREAD_BUSY_SIZE 32

/*
	Ïß³Ì³ØµÄÏß³Ì»ùÀà
*/
class ThreadPool;
class TPThread
{
public:
	friend class ThreadPool;

	// Ïß³Ì×´Ì¬ -1»¹Î´Æô¶¯, 0Ë¯Ãß£¬ 1·±Ã¦ÖÐ
	enum THREAD_STATE
	{
		THREAD_STATE_STOP = -1,
		THREAD_STATE_SLEEP = 0,
		THREAD_STATE_BUSY = 1,
		THREAD_STATE_END = 2,
		THREAD_STATE_PENDING = 3
	};

public:
	TPThread(ThreadPool* threadPool, int threadWaitSecond = 0):
	threadWaitSecond_(threadWaitSecond), 
	currTask_(NULL), 
	threadPool_(threadPool)
	{
		state_ = THREAD_STATE_SLEEP;
		initCond();
		initMutex();
	}
		
	virtual ~TPThread()
	{
		deleteCond();
		deleteMutex();

		KBE_THREADPOOL_DEBUG_MSG(fmt::format("TPThread::~TPThread(): {}\n", (void*)this));
	}
	
	virtual void onStart(){}
	virtual void onEnd(){}

	virtual void onProcessTaskStart(TPTask* pTask) {}
	virtual void processTask(TPTask* pTask){ pTask->process(); }
	virtual void onProcessTaskEnd(TPTask* pTask) {}

	INLINE THREAD_ID id(void) const;
	
	INLINE void id(THREAD_ID tidp);
	
	/**
		´´½¨Ò»¸öÏß³Ì£¬ ²¢½«×Ô¼ºÓë¸ÃÏß³Ì°ó¶¨
	*/
	THREAD_ID createThread(void);
	
	virtual void initCond(void)
	{
		THREAD_SINGNAL_INIT(cond_);
	}

	virtual void initMutex(void)
	{
		THREAD_MUTEX_INIT(mutex_);	
	}

	virtual void deleteCond(void)
	{
		THREAD_SINGNAL_DELETE(cond_);
	}
	
	virtual void deleteMutex(void)
	{
		THREAD_MUTEX_DELETE(mutex_);
	}

	virtual void lock(void)
	{
		THREAD_MUTEX_LOCK(mutex_); 
	}
	
	virtual void unlock(void)
	{
		THREAD_MUTEX_UNLOCK(mutex_); 
	}	

	virtual TPTask* tryGetTask(void);
	
	/**
		·¢ËÍÌõ¼þÐÅºÅ
	*/
	int sendCondSignal(void)
	{
#if KBE_PLATFORM == PLATFORM_WIN32
		return THREAD_SINGNAL_SET(cond_);
#else
REATTEMPT:

		lock();

		if (state_ == THREAD_STATE_PENDING)
		{       
			unlock();
			goto REATTEMPT;
		}

		int ret = THREAD_SINGNAL_SET(cond_);
		unlock();
		return ret;
#endif
	}
	
	/**
		Ïß³ÌÍ¨Öª µÈ´ýÌõ¼þÐÅºÅ
	*/
	bool onWaitCondSignal(void);
	
	bool join(void);

	/**
		»ñÈ¡±¾Ïß³ÌÒª´¦ÀíµÄÈÎÎñ
	*/
	INLINE TPTask* task(void) const;

	/**
		ÉèÖÃ±¾Ïß³ÌÒª´¦ÀíµÄÈÎÎñ
	*/
	INLINE void task(TPTask* tpt);

	INLINE int state(void) const;
	
	/**
		±¾Ïß³ÌÒª´¦ÀíµÄÈÎÎñÒÑ¾­´¦ÀíÍê±Ï ÎÒÃÇ¾ö¶¨É¾³ýÕâ¸ö·ÏÆúµÄÈÎÎñ
	*/
	void onTaskCompleted(void);

#if KBE_PLATFORM == PLATFORM_WIN32
	static unsigned __stdcall threadFunc(void *arg);
#else	
	static void* threadFunc(void* arg);
#endif

	/**
		ÉèÖÃ±¾Ïß³ÌÒª´¦ÀíµÄÈÎÎñ
	*/
	INLINE ThreadPool* threadPool();

	/**
		Êä³öÏß³Ì¹¤×÷×´Ì¬
		Ö÷ÒªÌá¹©¸øwatcherÊ¹ÓÃ
	*/
	virtual std::string printWorkState()
	{
		char buf[128];
		lock();
		std::snprintf(buf, sizeof(buf), "%p,%u", currTask_, done_tasks_);
		unlock();
		return buf;
	}

	/**
		Ïß³ÌÆô¶¯Ò»´ÎÔÚÎ´¸Ä±äµ½ÏÐÖÃ×´Ì¬ÏÂÁ¬ÐøÖ´ÐÐµÄÈÎÎñ¼ÆÊý
	*/
	void reset_done_tasks(){ done_tasks_ = 0; }
	void inc_done_tasks(){ ++done_tasks_; }

protected:
	THREAD_SINGNAL cond_;			// Ïß³ÌÐÅºÅÁ¿
	THREAD_MUTEX mutex_;			// Ïß³Ì»¥ËßÌå
	int threadWaitSecond_;			// Ïß³Ì¿ÕÏÐ×´Ì¬³¬¹ýÕâ¸öÃëÊýÔòÏß³ÌÍË³ö, Ð¡ÓÚ0ÎªÓÀ¾ÃÏß³Ì(Ãëµ¥Î»)
	TPTask * currTask_;				// ¸ÃÏß³ÌµÄµ±Ç°Ö´ÐÐµÄÈÎÎñ
	THREAD_ID tidp_;				// ±¾Ïß³ÌµÄID
	ThreadPool* threadPool_;		// Ïß³Ì³ØÖ¸Õë
	THREAD_STATE state_;			// Ïß³Ì×´Ì¬: -1»¹Î´Æô¶¯, 0Ë¯Ãß, 1·±Ã¦ÖÐ
	uint32 done_tasks_;				// Ïß³ÌÆô¶¯Ò»´ÎÔÚÎ´¸Ä±äµ½ÏÐÖÃ×´Ì¬ÏÂÁ¬ÐøÖ´ÐÐµÄÈÎÎñ¼ÆÊý
};


class ThreadPool
{
public:		
	
	ThreadPool();
	virtual ~ThreadPool();
	
	void finalise();

	virtual void onMainThreadTick();
	
	bool hasThread(TPThread* pTPThread);

	/**
		»ñÈ¡µ±Ç°Ïß³Ì³ØËùÓÐÏß³Ì×´Ì¬(Ìá¹©¸øwatchÓÃ)
	*/
	std::string printThreadWorks();

	/**
		»ñÈ¡µ±Ç°Ïß³Ì×ÜÊý
	*/	
	INLINE uint32 currentThreadCount(void) const;
	
	/**
		»ñÈ¡µ±Ç°¿ÕÏÐÏß³Ì×ÜÊý
	*/		
	INLINE uint32 currentFreeThreadCount(void) const;
	
	/**
		´´½¨Ïß³Ì³Ø
		@param inewThreadCount			: µ±ÏµÍ³·±Ã¦Ê±Ïß³Ì³Ø»áÐÂÔö¼ÓÕâÃ´¶àÏß³Ì£¨ÁÙÊ±£©
		@param inormalMaxThreadCount	: Ïß³Ì³Ø»áÒ»Ö±±£³ÖÕâÃ´¶à¸öÊýµÄÏß³Ì
		@param imaxThreadCount			: Ïß³Ì³Ø×î¶àÖ»ÄÜÓÐÕâÃ´¶à¸öÏß³Ì
	*/
	bool createThreadPool(uint32 inewThreadCount, 
			uint32 inormalMaxThreadCount, uint32 imaxThreadCount);
	
	/**
		ÏòÏß³Ì³ØÌí¼ÓÒ»¸öÈÎÎñ
	*/		
	bool addTask(TPTask* tptask);
	bool _addTask(TPTask* tptask);
	INLINE bool addBackgroundTask(TPTask* tptask){ return addTask(tptask); }
	INLINE bool pushTask(TPTask* tptask){ return addTask(tptask); }

	/**
		Ïß³ÌÊýÁ¿ÊÇ·ñµ½´ï×î´ó¸öÊý
	*/
	INLINE bool isThreadCountMax(void) const;
	
	/**
		Ïß³Ì³ØÊÇ·ñ´¦ÓÚ·±Ã¦×´Ì¬
		Î´´¦ÀíÈÎÎñÊÇ·ñ·Ç³£¶à   ËµÃ÷Ïß³ÌºÜ·±Ã¦
	*/
	INLINE bool isBusy(void) const;
	
	/** 
		Ïß³Ì³ØÊÇ·ñÒÑ¾­±»³õÊ¼»¯ 
	*/
	INLINE bool isInitialize(void) const;

	/**
		·µ»ØÊÇ·ñÒÑ¾­Ïú»Ù
	*/
	INLINE bool isDestroyed() const;

	/**
		·µ»ØÊÇ·ñÒÑ¾­Ïú»Ù
	*/
	INLINE void destroy();

	/** 
		»ñµÃ»º´æµÄÈÎÎñÊýÁ¿
	*/
	INLINE uint32 bufferTaskSize() const;

	/** 
		»ñµÃ»º´æµÄÈÎÎñ
	*/
	INLINE std::queue<thread::TPTask*>& bufferedTaskList();

	/** 
		²Ù×÷»º´æµÄÈÎÎñËø
	*/
	INLINE void lockBufferedTaskList();
	INLINE void unlockBufferedTaskList();

	/** 
		»ñµÃÒÑ¾­Íê³ÉµÄÈÎÎñÊýÁ¿
	*/
	INLINE uint32 finiTaskSize() const;

	virtual std::string name() const { return "ThreadPool"; }

public:
	static int timeout;

	/**
		´´½¨Ò»¸öÏß³Ì³ØÏß³Ì
	*/
	virtual TPThread* createThread(int threadWaitSecond = ThreadPool::timeout, bool threadStartsImmediately = true);

	/**
		½«Ä³¸öÈÎÎñ±£´æµ½Î´´¦ÀíÁÐ±í
	*/
	void bufferTask(TPTask* tptask);

	/**
		´ÓÎ´´¦ÀíÁÐ±íÈ¡³öÒ»¸öÈÎÎñ ²¢´ÓÁÐ±íÖÐÉ¾³ý
	*/
	TPTask* popbufferTask(void);

	/**
		ÒÆ¶¯Ò»¸öÏß³Ìµ½¿ÕÏÐÁÐ±í
	*/
	bool addFreeThread(TPThread* tptd);
	
	/**
		ÒÆ¶¯Ò»¸öÏß³Ìµ½·±Ã¦ÁÐ±í
	*/	
	bool addBusyThread(TPThread* tptd);
	
	/**
		Ìí¼ÓÒ»¸öÒÑ¾­Íê³ÉµÄÈÎÎñµ½ÁÐ±í
	*/	
	void addFiniTask(TPTask* tptask);
	
	/**
		É¾³ýÒ»¸ö¹ÒÆð(³¬Ê±)Ïß³Ì
	*/	
	bool removeHangThread(TPThread* tptd);

	bool initializeWatcher();

protected:
	bool isInitialize_;												// Ïß³Ì³ØÊÇ·ñ±»³õÊ¼»¯¹ý
	
	std::queue<TPTask*> bufferedTaskList_;							// ÏµÍ³´¦ÓÚ·±Ã¦Ê±»¹Î´´¦ÀíµÄÈÎÎñÁÐ±í
	std::list<TPTask*> finiTaskList_;								// ÒÑ¾­Íê³ÉµÄÈÎÎñÁÐ±í
	size_t finiTaskList_count_;

	THREAD_MUTEX bufferedTaskList_mutex_;							// ´¦ÀíbufferTaskList»¥³âËø
	THREAD_MUTEX threadStateList_mutex_;							// ´¦ÀíbufferTaskList and freeThreadList_»¥³âËø
	THREAD_MUTEX finiTaskList_mutex_;								// ´¦ÀífiniTaskList»¥³âËø
	
	std::list<TPThread*> busyThreadList_;							// ·±Ã¦µÄÏß³ÌÁÐ±í
	std::list<TPThread*> freeThreadList_;							// ÏÐÖÃµÄÏß³ÌÁÐ±í
	std::list<TPThread*> allThreadList_;							// ËùÓÐµÄÏß³ÌÁÐ±í

	uint32 maxThreadCount_;											// ×î´óÏß³Ì×ÜÊý
	uint32 extraNewAddThreadCount_;									// Èç¹ûnormalThreadCount_²»×ã¹»Ê¹ÓÃÔò»áÐÂ´´½¨ÕâÃ´¶àÏß³Ì
	uint32 currentThreadCount_;										// µ±Ç°Ïß³ÌÊý
	uint32 currentFreeThreadCount_;									// µ±Ç°ÏÐÖÃµÄÏß³ÌÊý
	uint32 normalThreadCount_;										// ±ê×¼×´Ì¬ÏÂµÄÏß³Ì×ÜÊý ¼´£ºÄ¬ÈÏÇé¿öÏÂÒ»Æô¶¯·þÎñÆ÷¾Í¿ªÆôÕâÃ´¶àÏß³Ì
																	// Èç¹ûÏß³Ì²»×ã¹»£¬Ôò»áÐÂ´´½¨Ò»Ð©Ïß³Ì£¬ ×î´óÄÜ¹»µ½maxThreadNum.

	bool isDestroyed_;
};

}


}

#ifdef CODE_INLINE
#include "threadpool.inl"
#endif
#endif // KBE_THREADPOOL_H
