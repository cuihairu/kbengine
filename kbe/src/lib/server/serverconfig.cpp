// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "network/common.h"
#include "serverconfig.h"
#include "network/address.h"
#include "resmgr/resmgr.h"
#include "common/kbekey.h"
#include "common/kbeversion.h"
#include <tinyxml2.h>

#ifndef CODE_INLINE
#include "serverconfig.inl"
#endif

namespace KBEngine{
KBE_SINGLETON_INIT(ServerConfig);

static bool g_dbmgr_addDefaultAddress = true;

namespace
{
std::string textValue(const tinyxml2::XMLNode* node)
{
	if (node == nullptr)
	{
		return "";
	}

	const tinyxml2::XMLText* text = node->ToText();
	if (text != nullptr && text->Value() != nullptr)
	{
		return text->Value();
	}

	const tinyxml2::XMLElement* element = node->ToElement();
	const char* value = element ? element->GetText() : nullptr;
	return value ? value : "";
}

std::string trimmedText(const tinyxml2::XMLNode* node)
{
	return strutil::kbe_trim(textValue(node));
}

int intText(const tinyxml2::XMLNode* node)
{
	const tinyxml2::XMLElement* element = node ? node->ToElement() : nullptr;
	if (element != nullptr)
	{
		return element->IntText();
	}

	const std::string value = trimmedText(node);
	return value.empty() ? 0 : atoi(value.c_str());
}

double doubleText(const tinyxml2::XMLNode* node)
{
	const tinyxml2::XMLElement* element = node ? node->ToElement() : nullptr;
	if (element != nullptr)
	{
		return element->DoubleText();
	}

	const std::string value = trimmedText(node);
	return value.empty() ? 0.0 : atof(value.c_str());
}

}

//-------------------------------------------------------------------------------------
ServerConfig::ServerConfig():
	gameUpdateHertz_(10),
	tick_max_buffered_logs_(4096),
	tick_max_sync_logs_(32),
	channelCommon_(),
	bitsPerSecondToClient_(0),
	interfacesAddress_(),
	interfacesPort_min_(0),
	interfacesPort_max_(0),
	interfacesAddrs_(),
	interfaces_orders_timeout_(0),
	shutdown_time_(1.f),
	shutdown_waitTickTime_(1.f),
	callback_timeout_(180.f),
	thread_timeout_(300.f),
	thread_init_create_(1),
	thread_pre_create_(2),
	thread_max_create_(8),
	emailServerInfo_(),
	emailAtivationInfo_(),
	emailResetPasswordInfo_(),
	emailBindInfo_()
{
}

//-------------------------------------------------------------------------------------
ServerConfig::~ServerConfig()
{
}

//-------------------------------------------------------------------------------------
bool ServerConfig::loadConfig(std::string fileName)
{
	tinyxml2::XMLNode* node = NULL, *rootNode = NULL;
	tinyxml2::XMLDocument document;
	const std::string path = Resmgr::getSingleton().matchRes(fileName);

	if(document.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS)
	{
		ERROR_MSG(fmt::format("ServerConfig::loadConfig: load {} is failed!\n",
			fileName.c_str()));

		return false;
	}
	
	tinyxml2::XMLElement* root = document.RootElement();
	if(root == NULL || root->FirstChildElement() == NULL)
	{
		// root½ÚµãÏÂÃ»ÓÐ×Ó½ÚµãÁË
		return true;
	}

	std::string email_service_config;
	rootNode = root->FirstChildElement("email_service_config");
	if(rootNode != NULL)
	{
		email_service_config = trimmedText(rootNode);
	}

	rootNode = root->FirstChildElement("packetAlwaysContainLength");
	if(rootNode != NULL){
		Network::g_packetAlwaysContainLength = intText(rootNode) != 0;
	}

	rootNode = root->FirstChildElement("trace_packet");
	if(rootNode != NULL)
	{
		tinyxml2::XMLNode* childnode = rootNode->FirstChildElement("debug_type");
		if(childnode)
			Network::g_trace_packet = intText(childnode);

		if(Network::g_trace_packet > 3)
			Network::g_trace_packet = 0;

		childnode = rootNode->FirstChildElement("use_logfile");
		if(childnode)
			Network::g_trace_packet_use_logfile = (trimmedText(childnode) == "true");

		tinyxml2::XMLElement* disablesNode = rootNode->FirstChildElement("disables");
		childnode = disablesNode ? disablesNode->FirstChildElement() : NULL;
		if(childnode)
		{
			do
			{
				std::string c = trimmedText(childnode);
				if(c.size() > 0)
				{
					Network::g_trace_packet_disables.push_back(c);
					
					// ²»debug¼ÓÃÜ°ü
					if(c == "Encrypted::packets")
						Network::g_trace_encrypted_packet = false;
				}
			}while((childnode = childnode->NextSiblingElement()));
		}
	}

	rootNode = root->FirstChildElement("debugEntity");
	if(rootNode != NULL)
	{
		g_debugEntity = intText(rootNode) > 0;
	}

	rootNode = root->FirstChildElement("publish");
	if(rootNode != NULL)
	{
		tinyxml2::XMLNode* childnode = rootNode->FirstChildElement("state");
		if(childnode)
		{
			g_appPublish = intText(childnode);
		}

		childnode = rootNode->FirstChildElement("script_version");
		if(childnode)
		{
			KBEVersion::setScriptVersion(trimmedText(childnode));
		}
	}

	rootNode = root->FirstChildElement("shutdown_time");
	if(rootNode != NULL)
	{
		shutdown_time_ = float(doubleText(rootNode));
	}
	
	rootNode = root->FirstChildElement("shutdown_waittick");
	if(rootNode != NULL)
	{
		shutdown_waitTickTime_ = float(doubleText(rootNode));
	}

	rootNode = root->FirstChildElement("callback_timeout");
	if(rootNode != NULL)
	{
		callback_timeout_ = float(doubleText(rootNode));
		if(callback_timeout_ < 5.f)
			callback_timeout_ = 5.f;
	}
	
	rootNode = root->FirstChildElement("thread_pool");
	if(rootNode != NULL)
	{
		tinyxml2::XMLNode* childnode = rootNode->FirstChildElement("timeout");
		if(childnode)
		{
			thread_timeout_ = float(KBE_MAX(1.0, doubleText(childnode)));
		}

		childnode = rootNode->FirstChildElement("init_create");
		if(childnode)
		{
			thread_init_create_ = KBE_MAX(1, intText(childnode));
		}

		childnode = rootNode->FirstChildElement("pre_create");
		if(childnode)
		{
			thread_pre_create_ = KBE_MAX(1, intText(childnode));
		}

		childnode = rootNode->FirstChildElement("max_create");
		if(childnode)
		{
			thread_max_create_ = KBE_MAX(1, intText(childnode));
		}
	}

	rootNode = root->FirstChildElement("channelCommon");
	if(rootNode != NULL)
	{
		tinyxml2::XMLNode* childnode = rootNode->FirstChildElement("timeout");
		if(childnode)
		{
			tinyxml2::XMLNode* childnode1 = childnode->FirstChildElement("internal");
			if(childnode1)
			{
				channelCommon_.channelInternalTimeout = KBE_MAX(0.f, float(doubleText(childnode1)));
				Network::g_channelInternalTimeout = channelCommon_.channelInternalTimeout;
			}

			childnode1 = childnode->FirstChildElement("external");
			if(childnode1)
			{
				channelCommon_.channelExternalTimeout = KBE_MAX(0.f, float(doubleText(childnode1)));
				Network::g_channelExternalTimeout = channelCommon_.channelExternalTimeout;
			}
		}

		childnode = rootNode->FirstChildElement("resend");
		if(childnode)
		{
			tinyxml2::XMLNode* childnode1 = childnode->FirstChildElement("internal");
			if(childnode1)
			{
				tinyxml2::XMLNode* childnode2 = childnode1->FirstChildElement("interval");
				if(childnode2)
				{
					Network::g_intReSendInterval = uint32(intText(childnode2));
				}

				childnode2 = childnode1->FirstChildElement("retries");
				if(childnode2)
				{
					Network::g_intReSendRetries = uint32(intText(childnode2));
				}
			}

			childnode1 = childnode->FirstChildElement("external");
			if(childnode1)
			{
				tinyxml2::XMLNode* childnode2 = childnode1->FirstChildElement("interval");
				if(childnode2)
				{
					Network::g_extReSendInterval = uint32(intText(childnode2));
				}

				childnode2 = childnode1->FirstChildElement("retries");
				if(childnode2)
				{
					Network::g_extReSendRetries = uint32(intText(childnode2));
				}
			}
		}

		childnode = rootNode->FirstChildElement("readBufferSize");
		if(childnode)
		{
			tinyxml2::XMLNode* childnode1 = childnode->FirstChildElement("internal");
			if(childnode1)
				channelCommon_.intReadBufferSize = KBE_MAX(0, intText(childnode1));

			childnode1 = childnode->FirstChildElement("external");
			if(childnode1)
				channelCommon_.extReadBufferSize = KBE_MAX(0, intText(childnode1));
		}

		childnode = rootNode->FirstChildElement("writeBufferSize");
		if(childnode)
		{
			tinyxml2::XMLNode* childnode1 = childnode->FirstChildElement("internal");
			if(childnode1)
				channelCommon_.intWriteBufferSize = KBE_MAX(0, intText(childnode1));

			childnode1 = childnode->FirstChildElement("external");
			if(childnode1)
				channelCommon_.extWriteBufferSize = KBE_MAX(0, intText(childnode1));
		}

		childnode = rootNode->FirstChildElement("windowOverflow");
		if(childnode)
		{
			tinyxml2::XMLNode* sendNode = childnode->FirstChildElement("send");
			if(sendNode)
			{
				tinyxml2::XMLNode* childnode1 = sendNode->FirstChildElement("messages");
				if(childnode1)
				{
					tinyxml2::XMLNode* childnode2 = childnode1->FirstChildElement("internal");
					if(childnode2)
						Network::g_intSendWindowMessagesOverflow = KBE_MAX(0, intText(childnode2));

					childnode2 = childnode1->FirstChildElement("external");
					if(childnode2)
						Network::g_extSendWindowMessagesOverflow = KBE_MAX(0, intText(childnode2));

					childnode2 = childnode1->FirstChildElement("critical");
					if(childnode2)
						Network::g_sendWindowMessagesOverflowCritical = KBE_MAX(0, intText(childnode2));
				}

				childnode1 = sendNode->FirstChildElement("bytes");
				if(childnode1)
				{
					tinyxml2::XMLNode* childnode2 = childnode1->FirstChildElement("internal");
					if(childnode2)
						Network::g_intSendWindowBytesOverflow = KBE_MAX(0, intText(childnode2));
				
					childnode2 = childnode1->FirstChildElement("external");
					if(childnode2)
						Network::g_extSendWindowBytesOverflow = KBE_MAX(0, intText(childnode2));
				}

				childnode1 = sendNode->FirstChildElement("tickSentBytes");
				if (childnode1)
				{
					tinyxml2::XMLNode* childnode2 = childnode1->FirstChildElement("internal");
					if (childnode2)
						Network::g_intSentWindowBytesOverflow = KBE_MAX(0, intText(childnode2));

					childnode2 = childnode1->FirstChildElement("external");
					if (childnode2)
						Network::g_extSentWindowBytesOverflow = KBE_MAX(0, intText(childnode2));
				}
			}

			tinyxml2::XMLNode* recvNode = childnode->FirstChildElement("receive");
			if(recvNode)
			{
				tinyxml2::XMLNode* childnode1 = recvNode->FirstChildElement("messages");
				if(childnode1)
				{
					tinyxml2::XMLNode* childnode2 = childnode1->FirstChildElement("internal");
					if(childnode2)
						Network::g_intReceiveWindowMessagesOverflow = KBE_MAX(0, intText(childnode2));

					childnode2 = childnode1->FirstChildElement("external");
					if(childnode2)
						Network::g_extReceiveWindowMessagesOverflow = KBE_MAX(0, intText(childnode2));

					childnode2 = childnode1->FirstChildElement("critical");
					if(childnode2)
						Network::g_receiveWindowMessagesOverflowCritical = KBE_MAX(0, intText(childnode2));
				}

				childnode1 = recvNode->FirstChildElement("bytes");
				if(childnode1)
				{
					tinyxml2::XMLNode* childnode2 = childnode1->FirstChildElement("internal");
					if(childnode2)
						Network::g_intReceiveWindowBytesOverflow = KBE_MAX(0, intText(childnode2));
				
					childnode2 = childnode1->FirstChildElement("external");
					if(childnode2)
						Network::g_extReceiveWindowBytesOverflow = KBE_MAX(0, intText(childnode2));
				}
			}
		}

		childnode = rootNode->FirstChildElement("encrypt_type");
		if(childnode)
		{
			Network::g_channelExternalEncryptType = intText(childnode);
		}

		childnode = rootNode->FirstChildElement("sslCertificate");
		if (childnode)
		{
			Network::g_sslCertificate = trimmedText(childnode);
		}

		childnode = rootNode->FirstChildElement("sslPrivateKey");
		if (childnode)
		{
			Network::g_sslPrivateKey = trimmedText(childnode);
		}

		tinyxml2::XMLNode* rudpChildnode = rootNode->FirstChildElement("reliableUDP");
		if(rudpChildnode)
		{
			childnode = rudpChildnode->FirstChildElement("readPacketsQueueSize");
			if (childnode)
			{
				tinyxml2::XMLNode* childnode1 = childnode->FirstChildElement("internal");
				if (childnode1)
					Network::g_rudp_intReadPacketsQueueSize = KBE_MAX(0, intText(childnode1));

				childnode1 = childnode->FirstChildElement("external");
				if (childnode1)
					Network::g_rudp_extReadPacketsQueueSize = KBE_MAX(0, intText(childnode1));
			}

			childnode = rudpChildnode->FirstChildElement("writePacketsQueueSize");
			if (childnode)
			{
				tinyxml2::XMLNode* childnode1 = childnode->FirstChildElement("internal");
				if (childnode1)
					Network::g_rudp_intWritePacketsQueueSize = KBE_MAX(0, intText(childnode1));

				childnode1 = childnode->FirstChildElement("external");
				if (childnode1)
					Network::g_rudp_extWritePacketsQueueSize = KBE_MAX(0, intText(childnode1));
			}

			childnode = rudpChildnode->FirstChildElement("tickInterval");
			if (childnode)
			{
				Network::g_rudp_tickInterval = KBE_MAX(0, intText(childnode));
			}

			childnode = rudpChildnode->FirstChildElement("minRTO");
			if (childnode)
			{
				Network::g_rudp_minRTO = KBE_MAX(0, intText(childnode));
			}

			childnode = rudpChildnode->FirstChildElement("mtu");
			if (childnode)
			{
				Network::g_rudp_mtu = KBE_MAX(0, intText(childnode));
			}

			childnode = rudpChildnode->FirstChildElement("missAcksResend");
			if (childnode)
			{
				Network::g_rudp_missAcksResend = KBE_MAX(0, intText(childnode));
			}

			childnode = rudpChildnode->FirstChildElement("congestionControl");
			if (childnode)
			{
				Network::g_rudp_congestionControl = (trimmedText(childnode) == "true");
			}

			childnode = rudpChildnode->FirstChildElement("nodelay");
			if (childnode)
			{
				Network::g_rudp_nodelay = (trimmedText(childnode) == "true");
			}
		}
	}

	rootNode = root->FirstChildElement("gameUpdateHertz");
	if(rootNode != NULL){
		gameUpdateHertz_ = intText(rootNode);
	}

	rootNode = root->FirstChildElement("bitsPerSecondToClient");
	if(rootNode != NULL){
		bitsPerSecondToClient_ = intText(rootNode);
	}

	rootNode = root->FirstChildElement("interfaces");
	if(rootNode != NULL)
	{
		tinyxml2::XMLNode* childnode = rootNode->FirstChildElement("entryScriptFile");	
		if(childnode != NULL)
			strncpy((char*)&_interfacesInfo.entryScriptFile, trimmedText(childnode).c_str(), MAX_NAME - 1);

		childnode = rootNode->FirstChildElement("host");
		if(childnode)
		{
			interfacesAddress_ = trimmedText(childnode);
		}

		childnode = rootNode->FirstChildElement("port_min");
		if(childnode)
		{
			interfacesPort_min_ = intText(childnode);

			if(interfacesPort_min_ <= 0)
				interfacesPort_min_ = KBE_INTERFACES_TCP_PORT;
		}

		childnode = rootNode->FirstChildElement("port_max");
		if (childnode)
		{
			interfacesPort_max_ = intText(childnode);

			if (interfacesPort_max_ <= 0)
				interfacesPort_max_ = interfacesPort_min_;
		}

		node = rootNode->FirstChildElement("SOMAXCONN");
		if(node != NULL){
			_interfacesInfo.tcp_SOMAXCONN = intText(node);
		}

		node = rootNode->FirstChildElement("orders_timeout");
		if(node != NULL){
			interfaces_orders_timeout_ = intText(node);
		}
	
		node = rootNode->FirstChildElement("telnet_service");
		if (node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("port");
			if (childnode)
			{
				_interfacesInfo.telnet_port = intText(childnode);
			}

			childnode = node->FirstChildElement("password");
			if (childnode)
			{
				_interfacesInfo.telnet_passwd = trimmedText(childnode);
			}

			childnode = node->FirstChildElement("default_layer");
			if (childnode)
			{
				_interfacesInfo.telnet_deflayer = trimmedText(childnode);
			}
		}
	}

	rootNode = root->FirstChildElement("cellapp");
	if(rootNode != NULL)
	{
		node = rootNode->FirstChildElement("internalInterface");	
		if(node != NULL)
			strncpy((char*)&_cellAppInfo.internalInterface, trimmedText(node).c_str(), MAX_NAME - 1);

		node = rootNode->FirstChildElement("entryScriptFile");	
		if(node != NULL)
			strncpy((char*)&_cellAppInfo.entryScriptFile, trimmedText(node).c_str(), MAX_NAME - 1);
		
		tinyxml2::XMLNode* viewNode = rootNode->FirstChildElement("defaultViewRadius");
		if(viewNode != NULL)
		{
			node = NULL;
			node = viewNode->FirstChildElement("radius");
			if(node != NULL)
				_cellAppInfo.defaultViewRadius = float(doubleText(node));
					
			node = viewNode->FirstChildElement("hysteresisArea");
			if(node != NULL)
				_cellAppInfo.defaultViewHysteresisArea = float(doubleText(node));
		}
			
		node = rootNode->FirstChildElement("ids");
		if(node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("criticallyLowSize");
			if(childnode)
			{
				_cellAppInfo.ids_criticallyLowSize = intText(childnode);
				if (_cellAppInfo.ids_criticallyLowSize < 100)
					_cellAppInfo.ids_criticallyLowSize = 100;
			}
		}
		
		node = rootNode->FirstChildElement("profiles");
		if(node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("cprofile");
			if(childnode)
			{
				_cellAppInfo.profiles.open_cprofile = (trimmedText(childnode) == "true");
			}

			childnode = node->FirstChildElement("pyprofile");
			if(childnode)
			{
				_cellAppInfo.profiles.open_pyprofile = (trimmedText(childnode) == "true");
			}

			childnode = node->FirstChildElement("eventprofile");
			if(childnode)
			{
				_cellAppInfo.profiles.open_eventprofile = (trimmedText(childnode) == "true");
			}

			childnode = node->FirstChildElement("networkprofile");
			if(childnode)
			{
				_cellAppInfo.profiles.open_networkprofile = (trimmedText(childnode) == "true");
			}
		}

		node = rootNode->FirstChildElement("SOMAXCONN");
		if(node != NULL){
			_cellAppInfo.tcp_SOMAXCONN = intText(node);
		}

		node = rootNode->FirstChildElement("aliasEntityID");
		if(node != NULL){
			_cellAppInfo.aliasEntityID = (trimmedText(node) == "true");
		}

		node = rootNode->FirstChildElement("entitydefAliasID");
		if(node != NULL){
			_cellAppInfo.entitydefAliasID = (trimmedText(node) == "true");
		}

		node = rootNode->FirstChildElement("loadSmoothingBias");
		if(node != NULL)
			_cellAppInfo.loadSmoothingBias = float(doubleText(node));

		node = rootNode->FirstChildElement("ghostDistance");
		if(node != NULL){
			_cellAppInfo.ghostDistance = (float)doubleText(node);
		}

		node = rootNode->FirstChildElement("ghostingMaxPerCheck");
		if(node != NULL){
			_cellAppInfo.ghostingMaxPerCheck = intText(node);
		}

		node = rootNode->FirstChildElement("ghostUpdateHertz");
		if(node != NULL){
			_cellAppInfo.ghostUpdateHertz = intText(node);
		}

		node = rootNode->FirstChildElement("coordinate_system");
		if(node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("enable");
			if(childnode)
			{
				_cellAppInfo.use_coordinate_system = (trimmedText(childnode) == "true");
			}
			
			childnode = node->FirstChildElement("rangemgr_y");
			if(childnode)
			{
				_cellAppInfo.coordinateSystem_hasY = (trimmedText(childnode) == "true");
			}

			childnode = node->FirstChildElement("entity_posdir_additional_updates");
			if(childnode)
			{
				_cellAppInfo.entity_posdir_additional_updates = intText(childnode);
			}

			childnode = node->FirstChildElement("entity_posdir_updates");
			if (childnode)
			{
				tinyxml2::XMLNode* node = childnode->FirstChildElement("type");
				if (node)
					_cellAppInfo.entity_posdir_updates_type = intText(node);

				node = childnode->FirstChildElement("smartThreshold");
				if (node)
					_cellAppInfo.entity_posdir_updates_smart_threshold = intText(node);
			}
		}

		node = rootNode->FirstChildElement("telnet_service");
		if(node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("port");
			if(childnode)
			{
				_cellAppInfo.telnet_port = intText(childnode);
			}

			childnode = node->FirstChildElement("password");
			if(childnode)
			{
				_cellAppInfo.telnet_passwd = trimmedText(childnode);
			}

			childnode = node->FirstChildElement("default_layer");
			if(childnode)
			{
				_cellAppInfo.telnet_deflayer = trimmedText(childnode);
			}
		}

		node = rootNode->FirstChildElement("shutdown");
		if(node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("perSecsDestroyEntitySize");
			if(childnode)
			{
				_cellAppInfo.perSecsDestroyEntitySize = uint32(intText(childnode));
			}
		}

		node = rootNode->FirstChildElement("witness");
		if(node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("timeout");
			if(childnode)
			{
				_cellAppInfo.witness_timeout = uint16(intText(childnode));
			}
		}
	}
	
	rootNode = root->FirstChildElement("baseapp");
	if(rootNode != NULL)
	{
		node = rootNode->FirstChildElement("entryScriptFile");	
		if(node != NULL)
			strncpy((char*)&_baseAppInfo.entryScriptFile, trimmedText(node).c_str(), MAX_NAME - 1);

		node = rootNode->FirstChildElement("internalInterface");	
		if(node != NULL)
			strncpy((char*)&_baseAppInfo.internalInterface, trimmedText(node).c_str(), MAX_NAME - 1);

		node = rootNode->FirstChildElement("externalInterface");	
		if(node != NULL)
			strncpy((char*)&_baseAppInfo.externalInterface, trimmedText(node).c_str(), MAX_NAME - 1);

		node = rootNode->FirstChildElement("externalAddress");	
		if(node != NULL)
			strncpy((char*)&_baseAppInfo.externalAddress, trimmedText(node).c_str(), MAX_NAME - 1);

		node = rootNode->FirstChildElement("externalTcpPorts_min");
		if(node != NULL)	
			_baseAppInfo.externalTcpPorts_min = intText(node);

		node = rootNode->FirstChildElement("externalTcpPorts_max");
		if(node != NULL)	
			_baseAppInfo.externalTcpPorts_max = intText(node);

		if(_baseAppInfo.externalTcpPorts_min < 0)
			_baseAppInfo.externalTcpPorts_min = -1;
		if(_baseAppInfo.externalTcpPorts_max < _baseAppInfo.externalTcpPorts_min)
			_baseAppInfo.externalTcpPorts_max = _baseAppInfo.externalTcpPorts_min;

		node = rootNode->FirstChildElement("externalUdpPorts_min");
		if (node != NULL)
			_baseAppInfo.externalUdpPorts_min = intText(node);

		node = rootNode->FirstChildElement("externalUdpPorts_max");
		if (node != NULL)
			_baseAppInfo.externalUdpPorts_max = intText(node);

		if (_baseAppInfo.externalUdpPorts_min < 0)
			_baseAppInfo.externalUdpPorts_min = -1;
		if (_baseAppInfo.externalUdpPorts_max < _baseAppInfo.externalUdpPorts_min)
			_baseAppInfo.externalUdpPorts_max = _baseAppInfo.externalUdpPorts_min;

		node = rootNode->FirstChildElement("archivePeriod");
		if(node != NULL)
			_baseAppInfo.archivePeriod = float(doubleText(node));
				
		node = rootNode->FirstChildElement("backupPeriod");
		if(node != NULL)
			_baseAppInfo.backupPeriod = float(doubleText(node));
		
		node = rootNode->FirstChildElement("backUpUndefinedProperties");
		if(node != NULL)
			_baseAppInfo.backUpUndefinedProperties = intText(node) > 0;
			
		node = rootNode->FirstChildElement("loadSmoothingBias");
		if(node != NULL)
			_baseAppInfo.loadSmoothingBias = float(doubleText(node));
		
		node = rootNode->FirstChildElement("ids");
		if(node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("criticallyLowSize");
			if(childnode)
			{
				_baseAppInfo.ids_criticallyLowSize = intText(childnode);
				if (_baseAppInfo.ids_criticallyLowSize < 100)
					_baseAppInfo.ids_criticallyLowSize = 100;
			}
		}
		
		node = rootNode->FirstChildElement("downloadStreaming");
		if(node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("bitsPerSecondTotal");
			if(childnode)
				_baseAppInfo.downloadBitsPerSecondTotal = intText(childnode);

			childnode = node->FirstChildElement("bitsPerSecondPerClient");
			if(childnode)
				_baseAppInfo.downloadBitsPerSecondPerClient = intText(childnode);
		}
	
		node = rootNode->FirstChildElement("profiles");
		if(node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("cprofile");
			if(childnode)
			{
				_baseAppInfo.profiles.open_cprofile = (trimmedText(childnode) == "true");
			}

			childnode = node->FirstChildElement("pyprofile");
			if(childnode)
			{
				_baseAppInfo.profiles.open_pyprofile = (trimmedText(childnode) == "true");
			}

			childnode = node->FirstChildElement("eventprofile");
			if(childnode)
			{
				_baseAppInfo.profiles.open_eventprofile = (trimmedText(childnode) == "true");
			}

			childnode = node->FirstChildElement("networkprofile");
			if(childnode)
			{
				_baseAppInfo.profiles.open_networkprofile = (trimmedText(childnode) == "true");
			}
		}

		node = rootNode->FirstChildElement("SOMAXCONN");
		if(node != NULL){
			_baseAppInfo.tcp_SOMAXCONN = intText(node);
		}

		node = rootNode->FirstChildElement("entityRestoreSize");
		if(node != NULL){
			_baseAppInfo.entityRestoreSize = intText(node);
		}
		
		if(_baseAppInfo.entityRestoreSize <= 0)
			_baseAppInfo.entityRestoreSize = 32;

		node = rootNode->FirstChildElement("telnet_service");
		if(node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("port");
			if(childnode)
			{
				_baseAppInfo.telnet_port = intText(childnode);
			}

			childnode = node->FirstChildElement("password");
			if(childnode)
			{
				_baseAppInfo.telnet_passwd = trimmedText(childnode);
			}

			childnode = node->FirstChildElement("default_layer");
			if(childnode)
			{
				_baseAppInfo.telnet_deflayer = trimmedText(childnode);
			}
		}

		node = rootNode->FirstChildElement("shutdown");
		if(node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("perSecsDestroyEntitySize");
			if(childnode)
			{
				_baseAppInfo.perSecsDestroyEntitySize = uint32(intText(childnode));
			}
		}

		node = rootNode->FirstChildElement("respool");
		if(node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("buffer_size");
			if(childnode)
				_baseAppInfo.respool_buffersize = intText(childnode);

			childnode = node->FirstChildElement("timeout");
			if(childnode)
				_baseAppInfo.respool_timeout = uint64(intText(childnode));

			childnode = node->FirstChildElement("checktick");
			if(childnode)
				Resmgr::respool_checktick = intText(childnode);

			Resmgr::respool_timeout = _baseAppInfo.respool_timeout;
			Resmgr::respool_buffersize = _baseAppInfo.respool_buffersize;
		}
	}

	rootNode = root->FirstChildElement("dbmgr");
	if(rootNode != NULL)
	{
		node = rootNode->FirstChildElement("entryScriptFile");
		if (node != NULL)
			strncpy((char*)&_dbmgrInfo.entryScriptFile, trimmedText(node).c_str(), MAX_NAME - 1);
		
		node = rootNode->FirstChildElement("telnet_service");
		if (node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("port");
			if (childnode)
			{
				_dbmgrInfo.telnet_port = intText(childnode);
			}

			childnode = node->FirstChildElement("password");
			if (childnode)
			{
				_dbmgrInfo.telnet_passwd = trimmedText(childnode);
			}

			childnode = node->FirstChildElement("default_layer");
			if (childnode)
			{
				_dbmgrInfo.telnet_deflayer = trimmedText(childnode);
			}
		}

		node = rootNode->FirstChildElement("ids");
		if (node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("increasing_range");
			if (childnode)
			{
				_dbmgrInfo.ids_increasing_range = intText(childnode);
			}
		}

		node = rootNode->FirstChildElement("InterfacesServiceAddr");
		if (node != NULL)
		{
			for (tinyxml2::XMLElement* loopNode = node->FirstChildElement("item");
				loopNode != NULL;
				loopNode = loopNode->NextSiblingElement("item"))
			{
				tinyxml2::XMLElement* hostNode = loopNode->FirstChildElement("host");
				tinyxml2::XMLElement* portNode = loopNode->FirstChildElement("port");
				if (hostNode && portNode)
				{
					std::string ip = trimmedText(hostNode);
					int port = intText(portNode);

					if (port <= 0)
						port = KBE_INTERFACES_TCP_PORT;

					Network::Address addr(ip, port);
					interfacesAddrs_.push_back(addr);
				}
			}

			tinyxml2::XMLNode* childnode = node->FirstChildElement("addDefaultAddress");
			if (childnode)
			{
				g_dbmgr_addDefaultAddress = trimmedText(childnode) == "true";
			}

			childnode = node->FirstChildElement("enable");
			if (childnode)
			{
				if (trimmedText(childnode) != "true")
				{
					interfacesAddrs_.clear();
					g_dbmgr_addDefaultAddress = false;
				}
			}
		}

		node = rootNode->FirstChildElement("internalInterface");	
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.internalInterface, trimmedText(node).c_str(), MAX_NAME - 1);

		tinyxml2::XMLElement* databaseInterfacesRoot = rootNode->FirstChildElement("databaseInterfaces");	
		if(databaseInterfacesRoot != NULL)
		{
			for (tinyxml2::XMLElement* databaseInterfacesNode = databaseInterfacesRoot->FirstChildElement();
				databaseInterfacesNode != NULL;
				databaseInterfacesNode = databaseInterfacesNode->NextSiblingElement())
				{
					std::vector<std::string> missingFields;
					missingFields.clear();

					std::string name = databaseInterfacesNode->Value();

					DBInterfaceInfo dbinfo;
					DBInterfaceInfo* pDBInfo = dbInterface(name);
					if (!pDBInfo)
						pDBInfo = &dbinfo;
					
					strncpy((char*)&pDBInfo->name, name.c_str(), MAX_NAME - 1);

					tinyxml2::XMLElement* interfaceNode = databaseInterfacesNode;
					
					node = interfaceNode->FirstChildElement("pure");
					if (node)
						pDBInfo->isPure = trimmedText(node) == "true";
					else
						missingFields.push_back("pure");

					// Ä¬ÈÏ¿â²»ÔÊÐíÊÇ´¿¾»¿â£¬ÒýÇæÐèÒª´´½¨ÊµÌå±í
					if (name == "default")
						pDBInfo->isPure = false;

					node = interfaceNode->FirstChildElement("type");
					if(node != NULL)
						strncpy((char*)&pDBInfo->db_type, trimmedText(node).c_str(), MAX_NAME - 1);
					else
						missingFields.push_back("type");

					
					node = interfaceNode->FirstChildElement("host");
					if(node != NULL)
						strncpy((char*)&pDBInfo->db_ip, trimmedText(node).c_str(), MAX_IP - 1);
					else
						missingFields.push_back("host");

					node = interfaceNode->FirstChildElement("port");
					if(node != NULL)
						pDBInfo->db_port = intText(node);
					else
						missingFields.push_back("port");

					node = interfaceNode->FirstChildElement("autoIncrementInit");
					if (node != NULL)
						strncpy((char*)&pDBInfo->db_autoIncrementInit, trimmedText(node).c_str(), MAX_BUF - 1);

					node = interfaceNode->FirstChildElement("auth");
					if(node != NULL)
					{
						tinyxml2::XMLNode* childnode = node->FirstChildElement("password");
						if(childnode)
						{
							strncpy((char*)&pDBInfo->db_password, trimmedText(childnode).c_str(), MAX_BUF * 10 - 1);
						}
						else
						{
							missingFields.push_back("auth->password");
						}

						childnode = node->FirstChildElement("username");
						if(childnode)
						{
							strncpy((char*)&pDBInfo->db_username, trimmedText(childnode).c_str(), MAX_NAME - 1);
						}
						else
						{
							missingFields.push_back("auth->username");
						}

						childnode = node->FirstChildElement("encrypt");
						if(childnode)
						{
							pDBInfo->db_passwordEncrypt = trimmedText(childnode) == "true";
						}
						else
						{
							missingFields.push_back("auth->encrypt");
						}
					}
					else
					{
						missingFields.push_back("auth");
					}
						
					node = interfaceNode->FirstChildElement("databaseName");
					if(node != NULL)
						strncpy((char*)&pDBInfo->db_name, trimmedText(node).c_str(), MAX_NAME - 1);
					else
						missingFields.push_back("databaseName");

					node = interfaceNode->FirstChildElement("numConnections");
					if(node != NULL)
						pDBInfo->db_numConnections = intText(node);
					else
						missingFields.push_back("numConnections");
						
					node = interfaceNode->FirstChildElement("unicodeString");
					if(node != NULL)
					{
						tinyxml2::XMLNode* childnode = node->FirstChildElement("characterSet");
						if(childnode)
						{
							pDBInfo->db_unicodeString_characterSet = trimmedText(childnode);
						}
						else
						{
							missingFields.push_back("unicodeString->characterSet");
						}

						childnode = node->FirstChildElement("collation");
						if(childnode)
						{
							pDBInfo->db_unicodeString_collation = trimmedText(childnode);
						}
						else
						{
							missingFields.push_back("unicodeString->collation");
						}
					}
					else
					{
						missingFields.push_back("unicodeString");
					}

					if (pDBInfo->db_unicodeString_characterSet.size() == 0)
						pDBInfo->db_unicodeString_characterSet = "utf8";

					if (pDBInfo->db_unicodeString_collation.size() == 0)
						pDBInfo->db_unicodeString_collation = "utf8_bin";
	
					if (pDBInfo == &dbinfo)
					{
						// ¼ì²é²»ÄÜÔÚ²»Í¬µÄ½Ó¿ÚÖÐÊ¹ÓÃÏàÍ¬µÄÊý¾Ý¿âÓëÏàÍ¬µÄ±í
						std::vector<DBInterfaceInfo>::iterator dbinfo_iter = _dbmgrInfo.dbInterfaceInfos.begin();
						for (; dbinfo_iter != _dbmgrInfo.dbInterfaceInfos.end(); ++dbinfo_iter)
						{
							if (kbe_stricmp((*dbinfo_iter).db_ip, dbinfo.db_ip) == 0 && 
								kbe_stricmp((*dbinfo_iter).db_type, dbinfo.db_type) == 0 &&
								(*dbinfo_iter).db_port == dbinfo.db_port &&
								strcmp(dbinfo.db_name, (*dbinfo_iter).db_name) == 0)
							{
								ERROR_MSG(fmt::format("ServerConfig::loadConfig: databaseInterfaces, Conflict between \"{}=(databaseName={})\" and \"{}=(databaseName={})\", file={}!\n",
									(*dbinfo_iter).name, (*dbinfo_iter).db_name, dbinfo.name, dbinfo.db_name, fileName.c_str()));

								return false;
							}
						}

						if (fileName == "server/kbengine_defaults.xml" && !missingFields.empty())
						{
							std::vector<std::string>::const_iterator iter = missingFields.begin();
							for (; iter != missingFields.end(); iter++)
							{
								ERROR_MSG(fmt::format("ServerConfig::loadConfig: kbengine_defaults.xml error, databaseInterface({}) missing filed:{}.\n", name, *iter));
							}

							return false;
						}

						_dbmgrInfo.dbInterfaceInfos.push_back(dbinfo);
					}

			}
		}

		node = rootNode->FirstChildElement("SOMAXCONN");
		if(node != NULL){
			_dbmgrInfo.tcp_SOMAXCONN = intText(node);
		}
		
		node = rootNode->FirstChildElement("debug");
		if(node != NULL){
			_dbmgrInfo.debugDBMgr = (trimmedText(node) == "true");
		}

		node = rootNode->FirstChildElement("allowEmptyDigest");
		if(node != NULL){
			_dbmgrInfo.allowEmptyDigest = (trimmedText(node) == "true");
		}

		node = rootNode->FirstChildElement("shareDB");
		if (node != NULL) {
			_dbmgrInfo.isShareDB = (trimmedText(node) == "true");
		}

		node = rootNode->FirstChildElement("account_system");
		if(node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("accountDefaultFlags");
			if(childnode)
			{
				_dbmgrInfo.accountDefaultFlags = intText(childnode);
			}

			childnode = node->FirstChildElement("accountDefaultDeadline");	
			if(childnode != NULL)
			{
				_dbmgrInfo.accountDefaultDeadline = intText(childnode);
			}

			childnode = node->FirstChildElement("accountEntityScriptType");	
			if(childnode != NULL)
			{
				strncpy((char*)&_dbmgrInfo.dbAccountEntityScriptType, trimmedText(childnode).c_str(), MAX_NAME - 1);
			}

			childnode = node->FirstChildElement("account_registration");	
			if(childnode != NULL)
			{
				tinyxml2::XMLNode* childchildnode = childnode->FirstChildElement("enable");
				if(childchildnode)
				{
					_dbmgrInfo.account_registration_enable = (trimmedText(childchildnode) == "true");
				}

				childchildnode = childnode->FirstChildElement("loginAutoCreate");
				if(childchildnode != NULL){
					_dbmgrInfo.notFoundAccountAutoCreate = (trimmedText(childchildnode) == "true");
				}
			} 

			childnode = node->FirstChildElement("account_resetPassword");
			if (childnode != NULL)
			{
				tinyxml2::XMLNode* childchildnode = childnode->FirstChildElement("enable");
				if (childchildnode)
				{
					_dbmgrInfo.account_reset_password_enable = (trimmedText(childchildnode) == "true");
				}
			}
		}
	}

	rootNode = root->FirstChildElement("loginapp");
	if(rootNode != NULL)
	{
		node = rootNode->FirstChildElement("entryScriptFile");
		if (node != NULL)
			strncpy((char*)&_loginAppInfo.entryScriptFile, trimmedText(node).c_str(), MAX_NAME - 1);
		
		node = rootNode->FirstChildElement("telnet_service");
		if (node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("port");
			if (childnode)
			{
				_loginAppInfo.telnet_port = intText(childnode);
			}

			childnode = node->FirstChildElement("password");
			if (childnode)
			{
				_loginAppInfo.telnet_passwd = trimmedText(childnode);
			}

			childnode = node->FirstChildElement("default_layer");
			if (childnode)
			{
				_loginAppInfo.telnet_deflayer = trimmedText(childnode);
			}
		}

		node = rootNode->FirstChildElement("internalInterface");	
		if(node != NULL)
			strncpy((char*)&_loginAppInfo.internalInterface, trimmedText(node).c_str(), MAX_NAME - 1);

		node = rootNode->FirstChildElement("externalInterface");	
		if(node != NULL)
			strncpy((char*)&_loginAppInfo.externalInterface, trimmedText(node).c_str(), MAX_NAME - 1);

		node = rootNode->FirstChildElement("externalAddress");	
		if(node != NULL)
			strncpy((char*)&_loginAppInfo.externalAddress, trimmedText(node).c_str(), MAX_NAME - 1);

		node = rootNode->FirstChildElement("externalTcpPorts_min");
		if(node != NULL)	
			_loginAppInfo.externalTcpPorts_min = intText(node);

		node = rootNode->FirstChildElement("externalTcpPorts_max");
		if(node != NULL)	
			_loginAppInfo.externalTcpPorts_max = intText(node);

		if(_loginAppInfo.externalTcpPorts_min < 0)
			_loginAppInfo.externalTcpPorts_min = -1;
		if(_loginAppInfo.externalTcpPorts_max < _loginAppInfo.externalTcpPorts_min)
			_loginAppInfo.externalTcpPorts_max = _loginAppInfo.externalTcpPorts_min;

		node = rootNode->FirstChildElement("externalUdpPorts_min");
		if (node != NULL)
			_loginAppInfo.externalUdpPorts_min = intText(node);

		node = rootNode->FirstChildElement("externalUdpPorts_max");
		if (node != NULL)
			_loginAppInfo.externalUdpPorts_max = intText(node);

		if (_loginAppInfo.externalUdpPorts_min < 0)
			_loginAppInfo.externalUdpPorts_min = -1;
		if (_loginAppInfo.externalUdpPorts_max < _loginAppInfo.externalUdpPorts_min)
			_loginAppInfo.externalUdpPorts_max = _loginAppInfo.externalUdpPorts_min;

		node = rootNode->FirstChildElement("SOMAXCONN");
		if(node != NULL){
			_loginAppInfo.tcp_SOMAXCONN = intText(node);
		}

		node = rootNode->FirstChildElement("encrypt_login");
		if(node != NULL){
			_loginAppInfo.encrypt_login = intText(node);
		}

		node = rootNode->FirstChildElement("account_type");
		if(node != NULL){
			_loginAppInfo.account_type = intText(node);
		}

		node = rootNode->FirstChildElement("http_cbhost");
		if(node)
			_loginAppInfo.http_cbhost = trimmedText(node);

		node = rootNode->FirstChildElement("http_cbport");
		if(node)
			_loginAppInfo.http_cbport = intText(node);
	}
	
	rootNode = root->FirstChildElement("cellappmgr");
	if(rootNode != NULL)
	{
		node = rootNode->FirstChildElement("internalInterface");	
		if(node != NULL)
			strncpy((char*)&_cellAppMgrInfo.internalInterface, trimmedText(node).c_str(), MAX_NAME - 1);

		node = rootNode->FirstChildElement("SOMAXCONN");
		if(node != NULL){
			_cellAppMgrInfo.tcp_SOMAXCONN = intText(node);
		}
	}
	
	rootNode = root->FirstChildElement("baseappmgr");
	if(rootNode != NULL)
	{
		node = rootNode->FirstChildElement("internalInterface");	
		if(node != NULL)
			strncpy((char*)&_baseAppMgrInfo.internalInterface, trimmedText(node).c_str(), MAX_NAME - 1);

		node = rootNode->FirstChildElement("SOMAXCONN");
		if(node != NULL){
			_baseAppMgrInfo.tcp_SOMAXCONN = intText(node);
		}
	}
	
	rootNode = root->FirstChildElement("machine");
	if(rootNode != NULL)
	{
		node = rootNode->FirstChildElement("internalInterface");	
		if(node != NULL)
			strncpy((char*)&_kbMachineInfo.internalInterface, trimmedText(node).c_str(), MAX_NAME - 1);

		node = rootNode->FirstChildElement("externalInterface");	
		if(node != NULL)
			strncpy((char*)&_kbMachineInfo.externalInterface, trimmedText(node).c_str(), MAX_NAME - 1);

		node = rootNode->FirstChildElement("externalTcpPorts_min");
		if(node != NULL)	
			_kbMachineInfo.externalTcpPorts_min = intText(node);

		node = rootNode->FirstChildElement("externalTcpPorts_max");
		if(node != NULL)	
			_kbMachineInfo.externalTcpPorts_max = intText(node);

		if(_kbMachineInfo.externalTcpPorts_min < 0)
			_kbMachineInfo.externalTcpPorts_min = 0;
		if(_kbMachineInfo.externalTcpPorts_max < _kbMachineInfo.externalTcpPorts_min)
			_kbMachineInfo.externalTcpPorts_max = _kbMachineInfo.externalTcpPorts_min;

		node = rootNode->FirstChildElement("externalUdpPorts_min");
		if (node != NULL)
			_kbMachineInfo.externalUdpPorts_min = intText(node);

		node = rootNode->FirstChildElement("externalUdpPorts_max");
		if (node != NULL)
			_kbMachineInfo.externalUdpPorts_max = intText(node);

		if (_kbMachineInfo.externalUdpPorts_min < 0)
			_kbMachineInfo.externalUdpPorts_min = 0;
		if (_kbMachineInfo.externalUdpPorts_max < _kbMachineInfo.externalUdpPorts_min)
			_kbMachineInfo.externalUdpPorts_max = _kbMachineInfo.externalUdpPorts_min;

		node = rootNode->FirstChildElement("SOMAXCONN");
		if(node != NULL){
			_kbMachineInfo.tcp_SOMAXCONN = intText(node);
		}
		
		node = rootNode->FirstChildElement("addresses");
		if(node)
		{
			for (tinyxml2::XMLElement* addressNode = node->FirstChildElement();
				addressNode != NULL;
				addressNode = addressNode->NextSiblingElement())
			{
				std::string c = trimmedText(addressNode);
				if(c.size() > 0)
				{
					_kbMachineInfo.machine_addresses.push_back(c);
				}
			}
		}
	}
	
	rootNode = root->FirstChildElement("bots");
	if(rootNode != NULL)
	{
		node = rootNode->FirstChildElement("entryScriptFile");	
		if(node != NULL)
			strncpy((char*)&_botsInfo.entryScriptFile, trimmedText(node).c_str(), MAX_NAME - 1);

		node = rootNode->FirstChildElement("internalInterface");	
		if(node != NULL)
			strncpy((char*)&_botsInfo.internalInterface, trimmedText(node).c_str(), MAX_NAME - 1);

		node = rootNode->FirstChildElement("host");	
		if(node != NULL)
			strncpy((char*)&_botsInfo.login_ip, trimmedText(node).c_str(), MAX_IP - 1);

		node = rootNode->FirstChildElement("port_min");	
		if(node != NULL)
			_botsInfo.login_port_min = intText(node);

		node = rootNode->FirstChildElement("port_max");
		if (node != NULL)
			_botsInfo.login_port_max = intText(node);

		if (_botsInfo.login_port_min < 0)
			_botsInfo.login_port_min = 0;
			
		if (_botsInfo.login_port_max < _botsInfo.login_port_min)
			_botsInfo.login_port_max = _botsInfo.login_port_min;
		
		_botsInfo.login_port = _botsInfo.login_port_min;

		node = rootNode->FirstChildElement("isOnInitCallPropertysSetMethods");
		if (node != NULL)
			_botsInfo.isOnInitCallPropertysSetMethods = intText(node);

		node = rootNode->FirstChildElement("defaultAddBots");
		if(node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("totalCount");
			if(childnode)
			{
				_botsInfo.defaultAddBots_totalCount = intText(childnode);
			}

			childnode = node->FirstChildElement("tickCount");
			if(childnode)
			{
				_botsInfo.defaultAddBots_tickCount = intText(childnode);
			}

			childnode = node->FirstChildElement("tickTime");
			if(childnode)
			{
				_botsInfo.defaultAddBots_tickTime = (float)doubleText(childnode);
			}
		}

		node = rootNode->FirstChildElement("account_infos");
		if(node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("account_name_prefix");
			if(childnode)
			{
				_botsInfo.bots_account_name_prefix = trimmedText(childnode);
			}

			childnode = node->FirstChildElement("account_name_suffix_inc");
			if(childnode)
			{
				_botsInfo.bots_account_name_suffix_inc = intText(childnode);
			}

			childnode = node->FirstChildElement("account_password");
			if (childnode)
			{
				_botsInfo.bots_account_passwd = trimmedText(childnode);
			}
		}

		node = rootNode->FirstChildElement("SOMAXCONN");
		if(node != NULL){
			_botsInfo.tcp_SOMAXCONN = intText(node);
		}

		node = rootNode->FirstChildElement("forceInternalLogin");
		if (node != NULL){
			_botsInfo.forceInternalLogin = (trimmedText(node) == "true");
		}

		node = rootNode->FirstChildElement("telnet_service");
		if(node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("port");
			if(childnode)
			{
				_botsInfo.telnet_port = intText(childnode);
			}

			childnode = node->FirstChildElement("password");
			if(childnode)
			{
				_botsInfo.telnet_passwd = trimmedText(childnode);
			}

			childnode = node->FirstChildElement("default_layer");
			if(childnode)
			{
				_botsInfo.telnet_deflayer = trimmedText(childnode);
			}
		}
	}

	rootNode = root->FirstChildElement("logger");
	if(rootNode != NULL)
	{
		node = rootNode->FirstChildElement("internalInterface");	
		if(node != NULL)
			strncpy((char*)&_loggerInfo.internalInterface, trimmedText(node).c_str(), MAX_NAME - 1);

		node = rootNode->FirstChildElement("entryScriptFile");
		if (node != NULL)
			strncpy((char*)&_loggerInfo.entryScriptFile, trimmedText(node).c_str(), MAX_NAME - 1);

		node = rootNode->FirstChildElement("SOMAXCONN");
		if(node != NULL){
			_loggerInfo.tcp_SOMAXCONN = intText(node);
		}

		node = rootNode->FirstChildElement("tick_max_buffered_logs");
		if(node != NULL){
			tick_max_buffered_logs_ = (uint32)intText(node);
		}

		node = rootNode->FirstChildElement("tick_sync_logs");
		if(node != NULL){
			tick_max_sync_logs_ = (uint32)intText(node);
		}
	
		node = rootNode->FirstChildElement("telnet_service");
		if (node != NULL)
		{
			tinyxml2::XMLNode* childnode = node->FirstChildElement("port");
			if (childnode)
			{
				_loggerInfo.telnet_port = intText(childnode);
			}

			childnode = node->FirstChildElement("password");
			if (childnode)
			{
				_loggerInfo.telnet_passwd = trimmedText(childnode);
			}

			childnode = node->FirstChildElement("default_layer");
			if (childnode)
			{
				_loggerInfo.telnet_deflayer = trimmedText(childnode);
			}
		}
	}

	if(email_service_config.size() > 0)
	{
		tinyxml2::XMLDocument emailDocument;
		const std::string emailPath = Resmgr::getSingleton().matchRes(email_service_config);

		if(emailDocument.LoadFile(emailPath.c_str()) != tinyxml2::XML_SUCCESS)
		{
			ERROR_MSG(fmt::format("ServerConfig::loadConfig: load {} is failed!\n",
				email_service_config.c_str()));

			return false;
		}

		tinyxml2::XMLElement* emailRoot = emailDocument.RootElement();
		if (emailRoot == NULL)
		{
			return true;
		}

		tinyxml2::XMLNode* childnode = emailRoot->FirstChildElement("smtp_server");
		if(childnode)
			emailServerInfo_.smtp_server = trimmedText(childnode);

		childnode = emailRoot->FirstChildElement("smtp_port");
		if(childnode)
			emailServerInfo_.smtp_port = intText(childnode);

		childnode = emailRoot->FirstChildElement("username");
		if(childnode)
			emailServerInfo_.username = trimmedText(childnode);

		childnode = emailRoot->FirstChildElement("password");
		if(childnode)
		{
			emailServerInfo_.password = trimmedText(childnode);
		}

		childnode = emailRoot->FirstChildElement("smtp_auth");
		if(childnode)
			emailServerInfo_.smtp_auth = intText(childnode);

		tinyxml2::XMLNode* rootNode1 = emailRoot->FirstChildElement("email_activation");
		if(rootNode1 != NULL)
		{
			tinyxml2::XMLNode* childnode1 = rootNode1->FirstChildElement("subject");
			if(childnode1)
				emailAtivationInfo_.subject = textValue(childnode1);

			childnode1 = rootNode1->FirstChildElement("message");
			if(childnode1)
				emailAtivationInfo_.message = textValue(childnode1);

			childnode1 = rootNode1->FirstChildElement("deadline");
			if(childnode1)
				emailAtivationInfo_.deadline = intText(childnode1);

			childnode1 = rootNode1->FirstChildElement("backlink_success_message");
			if(childnode1)
				emailAtivationInfo_.backlink_success_message = textValue(childnode1);

			childnode1 = rootNode1->FirstChildElement("backlink_fail_message");
			if(childnode1)
				emailAtivationInfo_.backlink_fail_message = textValue(childnode1);

			childnode1 = rootNode1->FirstChildElement("backlink_hello_message");
			if(childnode1)
				emailAtivationInfo_.backlink_hello_message = textValue(childnode1);
		}

		rootNode1 = emailRoot->FirstChildElement("email_resetpassword");
		if(rootNode1 != NULL)
		{
			tinyxml2::XMLNode* childnode1 = rootNode1->FirstChildElement("subject");
			if(childnode1)
				emailResetPasswordInfo_.subject = textValue(childnode1);

			childnode1 = rootNode1->FirstChildElement("message");
			if(childnode1)
				emailResetPasswordInfo_.message = textValue(childnode1);

			childnode1 = rootNode1->FirstChildElement("deadline");
			if(childnode1)
				emailResetPasswordInfo_.deadline = intText(childnode1);

			childnode1 = rootNode1->FirstChildElement("backlink_success_message");
			if(childnode1)
				emailResetPasswordInfo_.backlink_success_message = textValue(childnode1);

			childnode1 = rootNode1->FirstChildElement("backlink_fail_message");
			if(childnode1)
				emailResetPasswordInfo_.backlink_fail_message = textValue(childnode1);

			childnode1 = rootNode1->FirstChildElement("backlink_hello_message");
			if(childnode1)
				emailResetPasswordInfo_.backlink_hello_message = textValue(childnode1);
		}

		rootNode1 = emailRoot->FirstChildElement("email_bind");
		if(rootNode1 != NULL)
		{
			tinyxml2::XMLNode* childnode1 = rootNode1->FirstChildElement("subject");
			if(childnode1)
				emailBindInfo_.subject = textValue(childnode1);

			childnode1 = rootNode1->FirstChildElement("message");
			if(childnode1)
				emailBindInfo_.message = textValue(childnode1);

			childnode1 = rootNode1->FirstChildElement("deadline");
			if(childnode1)
				emailBindInfo_.deadline = intText(childnode1);

			childnode1 = rootNode1->FirstChildElement("backlink_success_message");
			if(childnode1)
				emailBindInfo_.backlink_success_message = textValue(childnode1);

			childnode1 = rootNode1->FirstChildElement("backlink_fail_message");
			if(childnode1)
				emailBindInfo_.backlink_fail_message = textValue(childnode1);

			childnode1 = rootNode1->FirstChildElement("backlink_hello_message");
			if(childnode1)
				emailBindInfo_.backlink_hello_message = textValue(childnode1);
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------	
uint32 ServerConfig::tcp_SOMAXCONN(COMPONENT_TYPE componentType)
{
	ENGINE_COMPONENT_INFO& cinfo = getComponent(componentType);
	return cinfo.tcp_SOMAXCONN;
}

//-------------------------------------------------------------------------------------	
void ServerConfig::_updateEmailInfos()
{
	// Èç¹ûÐ¡ÓÚ64Ôò±íÊ¾Ä¿Ç°»¹ÊÇÃ÷ÎÄÃÜÂë
	if(emailServerInfo_.password.size() < 64)
	{
		WARNING_MSG(fmt::format("ServerConfig::loadConfig: email password(email_service.xml) is not encrypted!\nplease use password(rsa):\n{}\n"
			, KBEKey::getSingleton().encrypt(emailServerInfo_.password)));
	}
	else
	{
		std::string out = KBEKey::getSingleton().decrypt(emailServerInfo_.password);
		if(out.size() == 0)
		{
			ERROR_MSG("ServerConfig::loadConfig: email password(email_service.xml) encrypt error!\n");
		}
		else
		{
			emailServerInfo_.password = out;
		}
	}
}

//-------------------------------------------------------------------------------------	
void ServerConfig::updateExternalAddress(char* buf)
{
	if(strlen(buf) > 0)
	{
		unsigned int inaddr = 0; 
		if((inaddr = inet_addr(buf)) == INADDR_NONE)  
		{
			struct hostent *host;
			host = gethostbyname(buf);
			if(host)
			{
				strncpy(buf, inet_ntoa(*(struct in_addr*)host->h_addr_list[0]), MAX_BUF - 1);
			}	
		}
	}
}

//-------------------------------------------------------------------------------------	
void ServerConfig::updateInfos(bool isPrint, COMPONENT_TYPE componentType, COMPONENT_ID componentID, 
							   const Network::Address& internalTcpAddr, const Network::Address& externalTcpAddr, const Network::Address& externalUdpAddr)
{
	std::string infostr = "";

	for (size_t i = 0; i < _dbmgrInfo.dbInterfaceInfos.size(); ++i)
		_dbmgrInfo.dbInterfaceInfos[i].index = i;

	if (g_dbmgr_addDefaultAddress)
	{
		Network::Address interfacesAddr(interfacesAddress_, interfacesPort_min_);
		interfacesAddrs_.insert(interfacesAddrs_.begin(), interfacesAddr);
	}

	//updateExternalAddress(getBaseApp().externalTcpAddr);
	//updateExternalAddress(getLoginApp().externalTcpAddr);

	if(componentType == CELLAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getCellApp();
		info.internalTcpAddr = &internalTcpAddr;
		info.externalTcpAddr = &externalTcpAddr;
		info.externalUdpAddr = &externalUdpAddr;
		info.componentID = componentID;

		if (info.ids_criticallyLowSize > getDBMgr().ids_increasing_range / 2)
		{
			info.ids_criticallyLowSize = getDBMgr().ids_increasing_range / 2;
			ERROR_MSG(fmt::format("kbengine[_defs].xml->cellapp->ids->criticallyLowSize > dbmgr->ids->increasing_range / 2, Force adjustment to criticallyLowSize({})\n", 
				info.ids_criticallyLowSize));
		}

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tgameUpdateHertz : {}\n", gameUpdateHertz()));
			INFO_MSG(fmt::format("\tdefaultViewRadius : {}\n", info.defaultViewRadius));
			INFO_MSG(fmt::format("\tdefaultViewHysteresisArea : {}\n", info.defaultViewHysteresisArea));
			INFO_MSG(fmt::format("\tentryScriptFile : {}\n", info.entryScriptFile));
			INFO_MSG(fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			//INFO_MSG(fmt::format("\texternalTcpAddr : {}\n", externalTcpAddr.c_str()));
			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tgameUpdateHertz : {}\n", gameUpdateHertz()));
			infostr += (fmt::format("\tdefaultViewRadius : {}\n", info.defaultViewRadius));
			infostr += (fmt::format("\tdefaultViewHysteresisArea : {}\n", info.defaultViewHysteresisArea));
			infostr += (fmt::format("\tentryScriptFile : {}\n", info.entryScriptFile));
			infostr += (fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			//infostr += (fmt::format("\texternalTcpAddr : {}\n", externalTcpAddr.c_str()));
			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}
	}
	else if (componentType == BASEAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getBaseApp();
		info.internalTcpAddr = const_cast<Network::Address*>(&internalTcpAddr);
		info.externalTcpAddr = const_cast<Network::Address*>(&externalTcpAddr);
		info.externalUdpAddr = const_cast<Network::Address*>(&externalUdpAddr);
		info.componentID = componentID;

		if (info.ids_criticallyLowSize > getDBMgr().ids_increasing_range / 2)
		{
			info.ids_criticallyLowSize = getDBMgr().ids_increasing_range / 2;
			ERROR_MSG(fmt::format("kbengine[_defs].xml->baseapp->ids->criticallyLowSize > dbmgr->ids->increasing_range / 2, Force adjustment to criticallyLowSize({})\n",
				info.ids_criticallyLowSize));
		}

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tgameUpdateHertz : {}\n", gameUpdateHertz()));
			INFO_MSG(fmt::format("\tentryScriptFile : {}\n", info.entryScriptFile));
			INFO_MSG(fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			INFO_MSG(fmt::format("\texternalTcpAddr : {}\n", externalTcpAddr.c_str()));
			INFO_MSG(fmt::format("\texternalUdpAddr : {}\n", externalUdpAddr.c_str()));

			if(strlen(info.externalAddress) > 0)
			{
				INFO_MSG(fmt::format("\texternalCustomAddr : {}\n", info.externalAddress));
			}

			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tgameUpdateHertz : {}\n", gameUpdateHertz()));
			infostr += (fmt::format("\tentryScriptFile : {}\n", info.entryScriptFile));
			infostr += (fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			infostr += (fmt::format("\texternalTcpAddr : {}\n", externalTcpAddr.c_str()));
			infostr += (fmt::format("\texternalUdpAddr : {}\n", externalUdpAddr.c_str()));

			if(strlen(info.externalAddress) > 0)
			{
				infostr +=  (fmt::format("\texternalCustomAddr : {}\n", info.externalAddress));
			}

			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}

		_updateEmailInfos();
	}
	else if (componentType == BASEAPPMGR_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getBaseAppMgr();
		info.internalTcpAddr = const_cast<Network::Address*>(&internalTcpAddr);
		info.externalTcpAddr = const_cast<Network::Address*>(&externalTcpAddr);
		info.componentID = componentID;

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			//INFO_MSG((fmt::format("\texternalTcpAddr : %s\n", externalTcpAddr.c_str())));
			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}
	}
	else if (componentType == CELLAPPMGR_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getCellAppMgr();
		info.internalTcpAddr = const_cast<Network::Address*>(&internalTcpAddr);
		info.externalTcpAddr = const_cast<Network::Address*>(&externalTcpAddr);
		info.componentID = componentID;

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			//INFO_MSG((fmt::format("\texternalTcpAddr : %s\n", externalTcpAddr.c_str())));
			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}
	}
	else if (componentType == DBMGR_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getDBMgr();
		info.internalTcpAddr = const_cast<Network::Address*>(&internalTcpAddr);
		info.externalTcpAddr = const_cast<Network::Address*>(&externalTcpAddr);
		info.componentID = componentID;

		if (info.ids_increasing_range < 500)
		{
			info.ids_increasing_range = 500;
			ERROR_MSG(fmt::format("kbengine[_defs].xml-> dbmgr->ids->increasing_range too small, Force adjustment to ids_increasing_range({})\n",
				info.ids_increasing_range));
		}

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			//INFO_MSG((fmt::format("\texternalTcpAddr : %s\n", externalTcpAddr.c_str())));
			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}
	}
	else if (componentType == LOGINAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getLoginApp();
		info.internalTcpAddr = const_cast<Network::Address*>(&internalTcpAddr);
		info.externalTcpAddr = const_cast<Network::Address*>(&externalTcpAddr);
		info.componentID = componentID;

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			INFO_MSG(fmt::format("\texternalTcpAddr : {}\n", externalTcpAddr.c_str()));
			if(strlen(info.externalAddress) > 0)
			{
				INFO_MSG(fmt::format("\texternalCustomAddr : {}\n", info.externalAddress));
			}

			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			infostr += (fmt::format("\texternalTcpAddr : {}\n", externalTcpAddr.c_str()));

			if(strlen(info.externalAddress) > 0)
			{
				infostr +=  (fmt::format("\texternalCustomAddr : {}\n", info.externalAddress));
			}

			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}

		_updateEmailInfos();
	}
	else if (componentType == MACHINE_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getKBMachine();
		info.internalTcpAddr = const_cast<Network::Address*>(&internalTcpAddr);
		info.externalTcpAddr = const_cast<Network::Address*>(&externalTcpAddr);
		info.componentID = componentID;
		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			//INFO_MSG((fmt::format("\texternalTcpAddr : %s\n", externalTcpAddr.c_str())));
			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}
	}
	else if (componentType == INTERFACES_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getInterfaces();
		info.internalTcpAddr = const_cast<Network::Address*>(&internalTcpAddr);
		info.externalTcpAddr = const_cast<Network::Address*>(&externalTcpAddr);
		info.componentID = componentID;
		if (isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			INFO_MSG((fmt::format("\texternalTcpAddr : %s\n", externalTcpAddr.c_str())));
			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}
	}

#if KBE_PLATFORM == PLATFORM_WIN32
	if(infostr.size() > 0)
	{
		infostr += "\n";
		printf("%s", infostr.c_str());
	}
#endif
}

//-------------------------------------------------------------------------------------		
}
