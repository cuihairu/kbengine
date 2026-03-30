// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


/*
	引用计数实现类

	使用方法:
		class AA:public RefCountable
		{
		public:
			AA(){}
			~AA(){ printf("析构"); }
		};
		
		--------------------------------------------
		AA* a = new AA();
		RefCountedPtr<AA>* s = new RefCountedPtr<AA>(a);
		RefCountedPtr<AA>* s1 = new RefCountedPtr<AA>(a);
		
		int i = (*s)->getRefCount();
		
		delete s;
		delete s1;
		
		执行结果:
			析构
*/
#ifndef KBE_REFCOUNTABLE_H
#define KBE_REFCOUNTABLE_H
	
#include "common.h"
	
namespace KBEngine{

class RefCountable 
{
public:
	inline void incRef(void) const
	{
		++refCount_;
	}

	inline void decRef(void) const
	{
		
		int currRef = --refCount_;
		assert(currRef >= 0 && "RefCountable:currRef maybe a error!");
		if (0 >= currRef)
			onRefOver();											// 引用结束了
	}

	virtual void onRefOver(void) const
	{
		delete const_cast<RefCountable*>(this);
	}

	void setRefCount(int n)
	{
		refCount_ = n;
	}

	int getRefCount(void) const 
	{ 
		return refCount_; 
	}

protected:
	RefCountable(void) : refCount_(0) 
	{
	}

	virtual ~RefCountable(void) 
	{ 
		assert(0 == refCount_ && "RefCountable:currRef maybe a error!"); 
	}

protected:
	volatile mutable long refCount_;
};

#if KBE_PLATFORM == PLATFORM_WIN32
class SafeRefCountable 
{
public:
	inline void incRef(void) const
	{
		::InterlockedIncrement(&refCount_);
	}

	inline void decRef(void) const
	{
		
		long currRef =::InterlockedDecrement(&refCount_);
		assert(currRef >= 0 && "RefCountable:currRef maybe a error!");
		if (0 >= currRef)
			onRefOver();											// 引用结束了
	}

	virtual void onRefOver(void) const
	{
		delete const_cast<SafeRefCountable*>(this);
	}

	void setRefCount(long n)
	{
		InterlockedExchange((long *)&refCount_, n);
	}

	int getRefCount(void) const 
	{ 
		return InterlockedExchange((long *)&refCount_, refCount_);
	}

protected:
	SafeRefCountable(void) : refCount_(0) 
	{
	}

	virtual ~SafeRefCountable(void) 
	{ 
		assert(0 == refCount_ && "SafeRefCountable:currRef maybe a error!"); 
	}

protected:
	volatile mutable long refCount_;
};
#else
class SafeRefCountable 
{
public:
	inline void incRef(void) const
	{
		__sync_add_and_fetch(&refCount_, 1);
	}

	inline void decRef(void) const
	{
		
		long currRef = intDecRef();
		assert(currRef >= 0 && "RefCountable:currRef maybe a error!");
		if (0 >= currRef)
			onRefOver();											// 引用结束了
	}

	virtual void onRefOver(void) const
	{
		delete const_cast<SafeRefCountable*>(this);
	}

	void setRefCount(long n)
	{
		__sync_lock_test_and_set(&refCount_, n);
	}

	int getRefCount(void) const 
	{ 
		return static_cast<int>(__sync_add_and_fetch(&refCount_, 0));
	}

protected:
	SafeRefCountable(void) : refCount_(0) 
	{
	}

	virtual ~SafeRefCountable(void) 
	{ 
		assert(0 == refCount_ && "SafeRefCountable:currRef maybe a error!"); 
	}

protected:
	volatile mutable long refCount_;
private:
	/**
	 *	This private method decreases the reference count by 1.
	 */
	inline int intDecRef() const
	{
		return static_cast<int>(__sync_sub_and_fetch(&refCount_, 1));
	}
};
#endif

template<class T>
class RefCountedPtr 
{
public:
	RefCountedPtr(T* ptr):ptr_(ptr) 
	{
		if (ptr_)
			ptr_->addRef();
	}

	RefCountedPtr(RefCountedPtr<T>* refptr):ptr_(refptr->getObject()) 
	{
		if (ptr_)
			ptr_->addRef();
	}
	
	~RefCountedPtr(void) 
	{
		if (0 != ptr_)
			ptr_->decRef();
	}

	T& operator*() const 
	{ 
		return *ptr_; 
	}

	T* operator->() const 
	{ 
		return (&**this); 
	}

	T* getObject(void) const 
	{ 
		return ptr_; 
	}

private:
	T* ptr_;
};

}
#endif // KBE_REFCOUNTABLE_H
