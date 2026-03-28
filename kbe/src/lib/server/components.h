// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_ENGINE_COMPONENT_MGR_H
#define KBE_ENGINE_COMPONENT_MGR_H
	
#include "common/timer.h"
#include "common/tasks.h"
#include "common/common.h"
#include "common/singleton.h"
#include "thread/threadmutex.h"
#include "thread/threadguard.h"
#include "server/common.h"
	
namespace KBEngine{

namespace Network
{
class Channel;
class Address;
class NetworkInterface;
class EventDispatcher;
}

// ComponentInfos.flags±êÖ¾
#define COMPONENT_FLAG_NORMAL 0x00000000
#define COMPONENT_FLAG_SHUTTINGDOWN 0x00000001

class Components : public Task, public Singleton<Components>
{
public:
	static int32 ANY_UID; 

	struct ComponentInfos
	{
		ComponentInfos()
		{
			uid = 0;
			flags = COMPONENT_FLAG_NORMAL;
			cid = 0;

			groupOrderid = 0;
			globalOrderid = 0;
			gus = 0;

			pChannel = NULL;
			state = COMPONENT_STATE_INIT;
			mem = cpu = 0.f;
			usedmem = 0;

			extradata = extradata1 = extradata2 = 0;

			pid = 0;
			externalAddressEx[0] = '\0';
			logTime = timestamp();
			appFlags = APP_FLAGS_NONE;
		}

		KBEShared_ptr<Network::Address> pIntAddr, pExtAddr;		// ÄÚ²¿ºÍÍâ²¿µØÖ·
		char externalAddressEx[MAX_NAME + 1];					// Ç¿ÖÆ±©Â¶¸øÍâ²¿µÄ¹«ÍøµØÖ·, Ïê¼ûÅäÖÃÖÐµÄexternalAddressEx

		int32 uid;
		COMPONENT_ID cid;
		COMPONENT_ORDER groupOrderid, globalOrderid;
		COMPONENT_GUS gus;
		char username[MAX_NAME + 1];
		Network::Channel* pChannel;
		COMPONENT_TYPE componentType;
		uint32 flags;

		// ½ø³Ì×´Ì¬
		COMPONENT_STATE state;

		float cpu;
		float mem;
		uint32 usedmem;
		uint64 extradata, extradata1, extradata2, extradata3;
		uint32 pid;
		uint64 logTime;
		uint32 appFlags;
	};

	typedef std::vector<ComponentInfos> COMPONENTS;

	/** ×é¼þÌí¼ÓÉ¾³ýhandler */
	class ComponentsNotificationHandler
	{
	public:
		virtual ~ComponentsNotificationHandler() {};
		virtual void onAddComponent(const Components::ComponentInfos*) = 0;
		virtual void onRemoveComponent(const Components::ComponentInfos*) = 0;
		virtual void onIdentityillegal(COMPONENT_TYPE componentType, COMPONENT_ID componentID, uint32 pid, const char* pAddr) = 0;
	};

public:
	Components();
	~Components();

	void initialize(Network::NetworkInterface * pNetworkInterface, COMPONENT_TYPE componentType, COMPONENT_ID componentID);
	void finalise();

	INLINE Network::NetworkInterface* pNetworkInterface()
	{ 
		return _pNetworkInterface;
	}

	void addComponent(int32 uid, const char* username, 
		COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_ORDER globalorderid, COMPONENT_ORDER grouporderid, COMPONENT_GUS gus,
		uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx, uint32 pid,
		float cpu, float mem, uint32 usedmem, uint64 extradata, uint64 extradata1, uint64 extradata2, uint64 extradata3,
		Network::Channel* pChannel = NULL);

	void delComponent(int32 uid, COMPONENT_TYPE componentType, 
		COMPONENT_ID componentID, bool ignoreComponentID = false, bool shouldShowLog = true);

	void removeComponentByChannel(Network::Channel * pChannel, bool isShutingdown = false);

	void clear(int32 uid = -1, bool shouldShowLog = true);

	Components::COMPONENTS& getComponents(COMPONENT_TYPE componentType);

	/** 
		²éÕÒ×é¼þ
	*/
	Components::ComponentInfos* findComponent(COMPONENT_TYPE componentType, int32 uid, COMPONENT_ID componentID);
	Components::ComponentInfos* findComponent(COMPONENT_TYPE componentType, COMPONENT_ID componentID);
	Components::ComponentInfos* findComponent(COMPONENT_ID componentID);
	Components::ComponentInfos* findComponent(Network::Channel * pChannel);
	Components::ComponentInfos* findComponent(Network::Address* pAddress);

	/** 
		Í¨¹ý½ø³ÌidÑ°ÕÒ±¾µØ×é¼þ
	*/
	Components::ComponentInfos* findLocalComponent(uint32 pid);

	int connectComponent(COMPONENT_TYPE componentType, int32 uid, COMPONENT_ID componentID, bool printlog = true);

	typedef std::map<int32/*uid*/, COMPONENT_ORDER/*lastorder*/> ORDER_LOG;
	ORDER_LOG& getGlobalOrderLog(){ return _globalOrderLog; }
	ORDER_LOG& getBaseappGroupOrderLog(){ return _baseappGrouplOrderLog; }
	ORDER_LOG& getCellappGroupOrderLog(){ return _cellappGrouplOrderLog; }
	ORDER_LOG& getLoginappGroupOrderLog(){ return _loginappGrouplOrderLog; }
	
	/** 
		¼ì²éËùÓÐµÄ×é¼þ£¬ ·ÀÖ¹ÓÐÖØ¸´µÄuuid£¬ ´ËÊ±Ó¦¸Ã±¨´í.
	*/
	bool checkComponents(int32 uid, COMPONENT_ID componentID, uint32 pid);

	/** 
		ÉèÖÃÓÃÓÚ½ÓÊÕ×é¼þÍ¨ÖªµÄ´¦ÀíÆ÷ÊµÀý
	*/
	void pHandler(ComponentsNotificationHandler* ph){ _pHandler = ph; };

	/** 
		¼ì²éÄ³¸ö×é¼þ¶Ë¿ÚÊÇ·ñÓÐÐ§.
	*/
	bool updateComponentInfos(const Components::ComponentInfos* info);

	/** 
		ÊÇ·ñÊÇ±¾µØ×é¼þ.
	*/
	bool isLocalComponent(const Components::ComponentInfos* info);

	/** 
		ÊÇ·ñ±¾µØ×é¼þÊÇ·ñÔÚÔËÐÐÖÐ.
	*/
	const Components::ComponentInfos* lookupLocalComponentRunning(uint32 pid);

	Components::ComponentInfos* getBaseappmgr();
	Components::ComponentInfos* getCellappmgr();
	Components::ComponentInfos* getDbmgr();
	Components::ComponentInfos* getLogger();
	Components::ComponentInfos* getInterfaceses();

	Network::Channel* getBaseappmgrChannel();
	Network::Channel* getCellappmgrChannel();
	Network::Channel* getDbmgrChannel();
	Network::Channel* getLoggerChannel();

	/**
		Í³¼ÆÄ³¸öUIDÏÂµÄËùÓÐ×é¼þÊýÁ¿
	*/
	size_t getGameSrvComponentsSize(int32 uid);

	/** 
		»ñÈ¡ÓÎÏ··þÎñ¶Ë±ØÒª×é¼þµÄ×¢²áÊýÁ¿¡£
	*/
	size_t getGameSrvComponentsSize();

	void componentID(COMPONENT_ID id){ componentID_ = id; }
	COMPONENT_ID componentID() const { return componentID_; }
	void componentType(COMPONENT_TYPE t){ componentType_ = t; }
	COMPONENT_TYPE componentType() const { return componentType_; }
	
	Network::EventDispatcher & dispatcher();

	void onChannelDeregister(Network::Channel * pChannel, bool isShutingdown);

	void extraData1(uint64 v){ extraData1_ = v; }
	void extraData2(uint64 v){ extraData2_ = v; }
	void extraData3(uint64 v){ extraData3_ = v; }
	void extraData4(uint64 v){ extraData4_ = v; }

	bool findLogger();
	
	void broadcastSelf();

private:
	virtual bool process();
	bool findComponents();

	void onFoundAllComponents();

private:
	COMPONENTS								_baseapps;
	COMPONENTS								_cellapps;
	COMPONENTS								_dbmgrs;
	COMPONENTS								_loginapps;
	COMPONENTS								_cellappmgrs;
	COMPONENTS								_baseappmgrs;
	COMPONENTS								_machines;
	COMPONENTS								_loggers;
	COMPONENTS								_interfaceses;
	COMPONENTS								_bots;
	COMPONENTS								_consoles;

	Network::NetworkInterface*				_pNetworkInterface;
	
	// ×é¼þµÄÈ«¾ÖÆô¶¯´ÎÐòlogºÍ×é(ÏàÍ¬µÄ×é¼þÎªÒ»×é£¬ Èç£ºËùÓÐbaseappÎªÒ»¸ö×é)Æô¶¯´ÎÐòlog
	// ×¢Òâ:ÖÐÍ¾ÓÐËÀµôµÄapp×é¼þÕâÀïlog²¢²»È¥×ö¼õ²Ù×÷, ´ÓÊ¹ÓÃÒâÍ¼À´¿´Ò²Ã»ÓÐ±ØÒª×öÕâ¸öÆ¥Åä¡£
	ORDER_LOG								_globalOrderLog;
	ORDER_LOG								_baseappGrouplOrderLog;
	ORDER_LOG								_cellappGrouplOrderLog;
	ORDER_LOG								_loginappGrouplOrderLog;

	ComponentsNotificationHandler*			_pHandler;

	// ±¾×é¼þµÄÀà±ð
	COMPONENT_TYPE							componentType_;

	// ±¾×é¼þµÄID
	COMPONENT_ID							componentID_;	

	uint8									state_;
	int16									findIdx_;
	int8									findComponentTypes_[8];

	uint64									extraData1_;
	uint64									extraData2_;
	uint64									extraData3_;
	uint64									extraData4_;
};

}

#endif // KBE_ENGINE_COMPONENT_MGR_H
