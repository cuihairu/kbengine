// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBENGINE_PROFILE_H
#define KBENGINE_PROFILE_H

#include "debug_helper.h"
#include "common/common.h"
#include "common/timer.h"
#include "common/timestamp.h"

namespace KBEngine
{

#if ENABLE_WATCHERS

class ProfileVal;

class ProfileGroup
{
public:
	ProfileGroup(std::string name = "default");
	~ProfileGroup();

	typedef std::vector<ProfileVal*> PROFILEVALS;
	typedef PROFILEVALS::iterator iterator;

	static ProfileGroup & defaultGroup();

	PROFILEVALS & stack() { return stack_; }
	void add(ProfileVal * pVal);

	iterator begin() { return profiles_.begin(); }
	iterator end() { return profiles_.end(); }

	ProfileVal * pRunningTime() { return profiles_[0]; }
	const ProfileVal * pRunningTime() const { return profiles_[0]; }
	TimeStamp runningTime() const;

	INLINE const char* name() const;

	bool initializeWatcher();

	static void finalise(void);

	INLINE const ProfileGroup::PROFILEVALS& profiles() const;

private:
	PROFILEVALS profiles_;
	PROFILEVALS stack_;
	std::string name_;
};

class ProfileVal
{
public:
	ProfileVal(std::string name, ProfileGroup * pGroup = NULL);
	~ProfileVal();

	bool initializeWatcher();

	void start()
	{
		if(!initWatcher_ && count_ > 10)
			initializeWatcher();

		TimeStamp now = timestamp();

		// ¼ÇÂ¼µÚ¼¸´Î´¦Àí
		if (inProgress_++ == 0)
			lastTime_ = now;

		ProfileGroup::PROFILEVALS & stack = pProfileGroup_->stack();

		// Èç¹ûÕ»ÖÐÓÐ¶ÔÏóÔò×Ô¼ºÊÇ´ÓÉÏÒ»¸öProfileValº¯Êý½øÈëµ÷ÓÃµÄ
		// ÎÒÃÇ¿ÉÒÔÔÚ´ËµÃµ½ÉÏÒ»¸öº¯Êý½øÈëµ½±¾º¯ÊýÖ®Ç°µÄÒ»¶ÎÊ±¼äÆ¬
		// È»ºó½«Æä¼ÓÈëµ½sumIntTime_
		if (!stack.empty()){
			ProfileVal & profile = *stack.back();
			profile.lastIntTime_ = now - profile.lastIntTime_;
			profile.sumIntTime_ += profile.lastIntTime_;
		}

		// ½«×Ô¼ºÑ¹Õ»
		stack.push_back(this);

		// ¼ÇÂ¼¿ªÊ¼Ê±¼ä
		lastIntTime_ = now;
	}

	void stop(uint32 qty = 0)
	{
		TimeStamp now = timestamp();

		// Èç¹ûÎª0Ôò±íÃ÷×Ô¼ºÊÇµ÷ÓÃÕ»µÄ²úÉú×Å
		// ÔÚ´ËÎÒÃÇ¿ÉÒÔµÃµ½Õâ¸öº¯Êý×Ü¹²ºÄ·ÑµÄÊ±¼ä
		if (--inProgress_ == 0){
			lastTime_ = now - lastTime_;
			sumTime_ += lastTime_;
		}

		lastQuantity_ = qty;
		sumQuantity_ += qty;
		++count_;

		ProfileGroup::PROFILEVALS & stack = pProfileGroup_->stack();
		KBE_ASSERT( stack.back() == this );

		stack.pop_back();

		// µÃµ½±¾º¯ÊýËùºÄ·ÑµÄÊ±¼ä
		lastIntTime_ = now - lastIntTime_;
		sumIntTime_ += lastIntTime_;

		// ÎÒÃÇÐèÒªÔÚ´ËÖØÉèÉÏÒ»¸öº¯ÊýÖÐµÄprofile¶ÔÏóµÄ×îºóÒ»´ÎÄÚ²¿Ê±¼ä
		// Ê¹ÆäÄÜ¹»ÔÚstartÊ±ÕýÈ·µÃµ½×Ôµ÷ÓÃÍê±¾º¯ÊýÖ®ºó½øÈëÏÂÒ»¸öprofileº¯ÊýÖÐÊ±ËùÏûºÄ
		// µÄÊ±¼äÆ¬¶Î
		if (!stack.empty())
			stack.back()->lastIntTime_ = now;
	}

	INLINE bool stop( const char * filename, int lineNum, uint32 qty = 0);

	INLINE bool running() const;

	INLINE const char * c_str() const;

	INLINE TimeStamp sumTime() const;
	INLINE TimeStamp lastIntTime() const ;
	INLINE TimeStamp sumIntTime() const ;
	INLINE TimeStamp lastTime() const;

	INLINE double lastTimeInSeconds() const;
	INLINE double sumTimeInSeconds() const ;
	INLINE double lastIntTimeInSeconds() const ;
	INLINE double sumIntTimeInSeconds() const;

	INLINE const char* name() const;

	INLINE uint32 count() const;

	INLINE bool isTooLong() const;

	static void setWarningPeriod(TimeStamp warningPeriod) { warningPeriod_ = warningPeriod; }

	// Ãû³Æ
	std::string		name_;

	// ProfileGroupÖ¸Õë
	ProfileGroup * pProfileGroup_;

	// startdºóµÄÊ±¼ä.
	TimeStamp		lastTime_;

	// count_´ÎµÄ×ÜÊ±¼ä
	TimeStamp		sumTime_;

	// ¼ÇÂ¼×îºóÒ»´ÎÄÚ²¿Ê±¼äÆ¬
	TimeStamp		lastIntTime_;

	// count_´ÎÄÚ²¿×ÜÊ±¼ä
	TimeStamp		sumIntTime_;

	uint32			lastQuantity_;	///< The last value passed into stop.
	uint32			sumQuantity_;	///< The total of all values passed into stop.
	uint32			count_;			///< The number of times stop has been called.

	// ¼ÇÂ¼µÚ¼¸´Î´¦Àí, ÈçµÝ¹éµÈ
	int				inProgress_;

	bool			initWatcher_;

private:
	static TimeStamp warningPeriod_;

};

class ScopedProfile
{
public:
	ScopedProfile(ProfileVal & profile, const char * filename, int lineNum) :
		profile_(profile),
		filename_(filename),
		lineNum_(lineNum)
	{
		profile_.start();
	}

	~ScopedProfile()
	{
		profile_.stop(filename_, lineNum_);
	}

private:
	ProfileVal& profile_;
	const char* filename_;
	int lineNum_;

};

#define START_PROFILE( PROFILE ) PROFILE.start();

#define STOP_PROFILE( PROFILE )	PROFILE.stop( __FILE__, __LINE__ );

#define AUTO_SCOPED_PROFILE( NAME )												\
	static ProfileVal _localProfile( NAME );									\
	ScopedProfile _autoScopedProfile( _localProfile, __FILE__, __LINE__ );

#define SCOPED_PROFILE(PROFILE)													\
	ScopedProfile PROFILE##_scopedProfile(PROFILE, __FILE__, __LINE__);

#define STOP_PROFILE_WITH_CHECK( PROFILE )										\
	if (PROFILE.stop( __FILE__, __LINE__ ))

#define STOP_PROFILE_WITH_DATA( PROFILE, DATA )									\
	PROFILE.stop( __FILE__, __LINE__ , DATA );

// ÓÉ´Ë¿ÉµÃµ½ÏµÍ³profileÊ±¼ä
uint64 runningTime();

#else

#define AUTO_SCOPED_PROFILE( NAME )
#define STOP_PROFILE_WITH_DATA( PROFILE, DATA )
#define STOP_PROFILE_WITH_CHECK( PROFILE )
#define SCOPED_PROFILE(PROFILE)
#define STOP_PROFILE( PROFILE )
#define START_PROFILE( PROFILE )

uint64 runningTime(){
	return 0;
}

#endif //ENABLE_WATCHERS


}

#ifdef CODE_INLINE
#include "profile.inl"
#endif

#endif // KBENGINE_PROFILE_H

