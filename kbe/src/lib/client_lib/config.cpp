// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "config.h"
#include "network/common.h"
#include "network/address.h"
#include "resmgr/resmgr.h"
#include "entitydef/entitydef.h"
#include "server/serverconfig.h"
#include "common/kbeversion.h"
#include <tinyxml2.h>

namespace
{
std::string trimmedText(const tinyxml2::XMLElement* element)
{
	const char* text = element ? element->GetText() : nullptr;
	return text ? strutil::kbe_trim(text) : "";
}
}

namespace KBEngine{
KBE_SINGLETON_INIT(Config);

ServerConfig g_ServerConfig;

//-------------------------------------------------------------------------------------
Config::Config():
gameUpdateHertz_(10),
tcp_SOMAXCONN_(5),
port_(0),
channelInternalTimeout_(60.0f),
channelExternalTimeout_(60.0f),
encrypt_login_(0),
fileName_(),
useLastAccountName_(false),
telnet_port(0),
telnet_passwd(),
telnet_deflayer(),
isOnInitCallPropertysSetMethods_(true)
{
}

//-------------------------------------------------------------------------------------
Config::~Config()
{
}

//-------------------------------------------------------------------------------------
bool Config::loadConfig(std::string fileName)
{
	fileName_ = fileName;

	tinyxml2::XMLDocument document;
	const std::string path = Resmgr::getSingleton().matchRes(fileName_);

	if(document.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS)
	{
		ERROR_MSG(fmt::format("Config::loadConfig: load {} is failed!\n",
			fileName.c_str()));

		return false;
	}

	tinyxml2::XMLElement* root = document.RootElement();
	if(root == NULL || root->FirstChildElement() == NULL)
	{
		// root鑺傜偣涓嬫病鏈夊瓙鑺傜偣浜?
		return true;
	}

	tinyxml2::XMLElement* rootNode = root->FirstChildElement("packetAlwaysContainLength");
	if(rootNode != NULL){
		Network::g_packetAlwaysContainLength = rootNode->IntText() != 0;
	}

	rootNode = root->FirstChildElement("trace_packet");
	if(rootNode != NULL)
	{
		tinyxml2::XMLElement* childnode = rootNode->FirstChildElement("debug_type");
		if(childnode)
			Network::g_trace_packet = childnode->IntText();

		if(Network::g_trace_packet > 3)
			Network::g_trace_packet = 0;

		childnode = rootNode->FirstChildElement("use_logfile");
		if(childnode)
			Network::g_trace_packet_use_logfile = (trimmedText(childnode) == "true");

		tinyxml2::XMLElement* disablesNode = rootNode->FirstChildElement("disables");
		if(disablesNode)
		{
			for (tinyxml2::XMLElement* item = disablesNode->FirstChildElement();
				item != nullptr;
				item = item->NextSiblingElement())
			{
				std::string c = trimmedText(item);
				if(c.size() > 0)
				{
					Network::g_trace_packet_disables.push_back(c);

					// 涓峝ebug鍔犲瘑鍖?
					if(c == "Encrypted::packets")
						Network::g_trace_encrypted_packet = false;
				}
			}
		}
	}

	rootNode = root->FirstChildElement("debugEntity");
	if(rootNode != NULL)
	{
		g_debugEntity = rootNode->IntText() > 0;
	}

	rootNode = root->FirstChildElement("publish");
	if(rootNode != NULL)
	{
		tinyxml2::XMLElement* childnode = rootNode->FirstChildElement("state");
		if(childnode)
		{
			g_appPublish = childnode->IntText();
		}

		childnode = rootNode->FirstChildElement("script_version");
		if(childnode)
		{
			KBEVersion::setScriptVersion(trimmedText(childnode));
		}
	}

	rootNode = root->FirstChildElement("channelCommon");
	if(rootNode != NULL)
	{
		tinyxml2::XMLElement* childnode = rootNode->FirstChildElement("timeout");
		if(childnode)
		{
			tinyxml2::XMLElement* childnode1 = childnode->FirstChildElement("internal");
			if(childnode1)
			{
				channelInternalTimeout_ = KBE_MAX(0.f, float(childnode1->DoubleText()));
				Network::g_channelInternalTimeout = channelInternalTimeout_;
			}

			childnode1 = childnode->FirstChildElement("external");
			if(childnode1)
			{
				channelExternalTimeout_ = KBE_MAX(0.f, float(childnode1->DoubleText()));
				Network::g_channelExternalTimeout = channelExternalTimeout_;
			}
		}

		childnode = rootNode->FirstChildElement("resend");
		if(childnode)
		{
			tinyxml2::XMLElement* childnode1 = childnode->FirstChildElement("internal");
			if(childnode1)
			{
				tinyxml2::XMLElement* childnode2 = childnode1->FirstChildElement("interval");
				if(childnode2)
				{
					Network::g_intReSendInterval = uint32(childnode2->IntText());
				}

				childnode2 = childnode1->FirstChildElement("retries");
				if(childnode2)
				{
					Network::g_intReSendRetries = uint32(childnode2->IntText());
				}
			}

			childnode1 = childnode->FirstChildElement("external");
			if(childnode1)
			{
				tinyxml2::XMLElement* childnode2 = childnode1->FirstChildElement("interval");
				if(childnode2)
				{
					Network::g_extReSendInterval = uint32(childnode2->IntText());
				}

				childnode2 = childnode1->FirstChildElement("retries");
				if(childnode2)
				{
					Network::g_extReSendRetries = uint32(childnode2->IntText());
				}
			}
		}

		childnode = rootNode->FirstChildElement("windowOverflow");
		if(childnode)
		{
			tinyxml2::XMLElement* sendNode = childnode->FirstChildElement("send");
			if(sendNode)
			{
				tinyxml2::XMLElement* childnode1 = sendNode->FirstChildElement("messages");
				if(childnode1)
				{
					tinyxml2::XMLElement* childnode2 = childnode1->FirstChildElement("internal");
					if(childnode2)
						Network::g_intSendWindowMessagesOverflow = KBE_MAX(0, childnode2->IntText());

					childnode2 = childnode1->FirstChildElement("external");
					if(childnode2)
						Network::g_extSendWindowMessagesOverflow = KBE_MAX(0, childnode2->IntText());

					childnode2 = childnode1->FirstChildElement("critical");
					if(childnode2)
						Network::g_sendWindowMessagesOverflowCritical = KBE_MAX(0, childnode2->IntText());
				}

				childnode1 = sendNode->FirstChildElement("bytes");
				if(childnode1)
				{
					tinyxml2::XMLElement* childnode2 = childnode1->FirstChildElement("internal");
					if(childnode2)
						Network::g_intSendWindowBytesOverflow = KBE_MAX(0, childnode2->IntText());

					childnode2 = childnode1->FirstChildElement("external");
					if(childnode2)
						Network::g_extSendWindowBytesOverflow = KBE_MAX(0, childnode2->IntText());
				}

				childnode1 = sendNode->FirstChildElement("tickSentBytes");
				if (childnode1)
				{
					tinyxml2::XMLElement* childnode2 = childnode1->FirstChildElement("internal");
					if (childnode2)
						Network::g_intSentWindowBytesOverflow = KBE_MAX(0, childnode2->IntText());

					childnode2 = childnode1->FirstChildElement("external");
					if (childnode2)
						Network::g_extSentWindowBytesOverflow = KBE_MAX(0, childnode2->IntText());
				}
			}

			tinyxml2::XMLElement* recvNode = childnode->FirstChildElement("receive");
			if(recvNode)
			{
				tinyxml2::XMLElement* childnode1 = recvNode->FirstChildElement("messages");
				if(childnode1)
				{
					tinyxml2::XMLElement* childnode2 = childnode1->FirstChildElement("internal");
					if(childnode2)
						Network::g_intReceiveWindowMessagesOverflow = KBE_MAX(0, childnode2->IntText());

					childnode2 = childnode1->FirstChildElement("external");
					if(childnode2)
						Network::g_extReceiveWindowMessagesOverflow = KBE_MAX(0, childnode2->IntText());

					childnode2 = childnode1->FirstChildElement("critical");
					if(childnode2)
						Network::g_receiveWindowMessagesOverflowCritical = KBE_MAX(0, childnode2->IntText());
				}

				childnode1 = recvNode->FirstChildElement("bytes");
				if(childnode1)
				{
					tinyxml2::XMLElement* childnode2 = childnode1->FirstChildElement("internal");
					if(childnode2)
						Network::g_intReceiveWindowBytesOverflow = KBE_MAX(0, childnode2->IntText());

					childnode2 = childnode1->FirstChildElement("external");
					if(childnode2)
						Network::g_extReceiveWindowBytesOverflow = KBE_MAX(0, childnode2->IntText());
				}
			}
		}

		childnode = rootNode->FirstChildElement("encrypt_type");
		if(childnode)
		{
			Network::g_channelExternalEncryptType = childnode->IntText();
		}

		tinyxml2::XMLElement* rudpChildnode = rootNode->FirstChildElement("reliableUDP");
		if (rudpChildnode)
		{
			childnode = rudpChildnode->FirstChildElement("readPacketsQueueSize");
			if (childnode)
			{
				tinyxml2::XMLElement* childnode1 = childnode->FirstChildElement("internal");
				if (childnode1)
					Network::g_rudp_intReadPacketsQueueSize = KBE_MAX(0, childnode1->IntText());

				childnode1 = childnode->FirstChildElement("external");
				if (childnode1)
					Network::g_rudp_extReadPacketsQueueSize = KBE_MAX(0, childnode1->IntText());
			}

			childnode = rudpChildnode->FirstChildElement("writePacketsQueueSize");
			if (childnode)
			{
				tinyxml2::XMLElement* childnode1 = childnode->FirstChildElement("internal");
				if (childnode1)
					Network::g_rudp_intWritePacketsQueueSize = KBE_MAX(0, childnode1->IntText());

				childnode1 = childnode->FirstChildElement("external");
				if (childnode1)
					Network::g_rudp_extWritePacketsQueueSize = KBE_MAX(0, childnode1->IntText());
			}

			childnode = rudpChildnode->FirstChildElement("tickInterval");
			if (childnode)
			{
				Network::g_rudp_tickInterval = KBE_MAX(0, childnode->IntText());
			}

			childnode = rudpChildnode->FirstChildElement("minRTO");
			if (childnode)
			{
				Network::g_rudp_minRTO = KBE_MAX(0, childnode->IntText());
			}

			childnode = rudpChildnode->FirstChildElement("mtu");
			if (childnode)
			{
				Network::g_rudp_mtu = KBE_MAX(0, childnode->IntText());
			}

			childnode = rudpChildnode->FirstChildElement("missAcksResend");
			if (childnode)
			{
				Network::g_rudp_missAcksResend = KBE_MAX(0, childnode->IntText());
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

	rootNode = root->FirstChildElement("telnet_service");
	if(rootNode != NULL)
	{
		tinyxml2::XMLElement* childnode = rootNode->FirstChildElement("port");
		if(childnode)
		{
			telnet_port = childnode->IntText();
		}

		childnode = rootNode->FirstChildElement("password");
		if(childnode)
		{
			telnet_passwd = trimmedText(childnode);
		}

		childnode = rootNode->FirstChildElement("default_layer");
		if(childnode)
		{
			telnet_deflayer = trimmedText(childnode);
		}
	}

	rootNode = root->FirstChildElement("gameUpdateHertz");
	if(rootNode != NULL){
		gameUpdateHertz_ = rootNode->IntText();
	}

	rootNode = root->FirstChildElement("ip");
	if(rootNode != NULL)
	{
		strcpy(ip_, trimmedText(rootNode).c_str());
	}

	rootNode = root->FirstChildElement("port");
	if(rootNode != NULL){
		port_ = rootNode->IntText();
	}

	rootNode = root->FirstChildElement("entryScriptFile");
	if(rootNode != NULL)
	{
		strcpy(entryScriptFile_, trimmedText(rootNode).c_str());
	}

	rootNode = root->FirstChildElement("accountName");
	if(rootNode != NULL)
	{
		strcpy(accountName_, trimmedText(rootNode).c_str());
	}

	rootNode = root->FirstChildElement("useLastAccountName");
	if(rootNode != NULL)
	{
		useLastAccountName_ = trimmedText(rootNode) != "false";
	}

	rootNode = root->FirstChildElement("encrypt_login");
	if(rootNode != NULL)
	{
		encrypt_login_ = rootNode->IntText();
	}

	rootNode = root->FirstChildElement("aliasEntityID");
	if(rootNode != NULL)
	{
		EntityDef::entityAliasID((trimmedText(rootNode) == "true"));
	}

	rootNode = root->FirstChildElement("entitydefAliasID");
	if(rootNode != NULL){
		EntityDef::entitydefAliasID((trimmedText(rootNode) == "true"));
	}

	rootNode = root->FirstChildElement("isOnInitCallPropertysSetMethods");
	if (rootNode != NULL)
		isOnInitCallPropertysSetMethods_ = (trimmedText(rootNode) == "true");

	return true;
}

//-------------------------------------------------------------------------------------
uint32 Config::tcp_SOMAXCONN()
{
	return tcp_SOMAXCONN_;
}

//-------------------------------------------------------------------------------------
void Config::writeAccountName(const char* name)
{
	if(!useLastAccountName_)
		return;

	tinyxml2::XMLDocument document;
	const std::string path = Resmgr::getSingleton().matchRes(fileName_);

	if(document.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS)
	{
		ERROR_MSG(fmt::format("Config::writeAccountName: load {} is failed!\n",
			fileName_.c_str()));

		return;
	}

	tinyxml2::XMLElement* root = document.RootElement();
	tinyxml2::XMLElement* rootNode = root ? root->FirstChildElement("accountName") : NULL;
	if(rootNode != NULL)
	{
		rootNode->SetText(name);
	}

	document.SaveFile(path.c_str());
}

//-------------------------------------------------------------------------------------
}
