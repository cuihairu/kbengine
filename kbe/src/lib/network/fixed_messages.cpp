// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
#include "fixed_messages.h"
#include "resmgr/resmgr.h"
#include <tinyxml2.h>

namespace KBEngine {

KBE_SINGLETON_INIT(Network::FixedMessages);

namespace Network
{

//-------------------------------------------------------------------------------------
FixedMessages::FixedMessages() :
	_infomap(),
	_loadedFiles()
{
	new Resmgr();
	Resmgr::getSingleton().initialize();
}

//-------------------------------------------------------------------------------------
FixedMessages::~FixedMessages()
{
	_infomap.clear();
}

//-------------------------------------------------------------------------------------
bool FixedMessages::loadConfig(std::string fileName, bool notFoundError)
{
	const std::string path = Resmgr::getSingleton().matchRes(fileName);
	if (_loadedFiles.find(path) != _loadedFiles.end())
		return true;

	tinyxml2::XMLDocument document;

	if (document.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS)
	{
		if (notFoundError)
		{
#if KBE_PLATFORM == PLATFORM_WIN32
			printf("%s", (fmt::format("[ERROR]: FixedMessages::loadConfig: load {} is failed!\n", fileName.c_str())).c_str());
#endif

			if (DebugHelper::isInit())
			{
				ERROR_MSG(fmt::format("FixedMessages::loadConfig: load {} is failed!\n", fileName.c_str()));
			}
		}

		return false;
	}

	_loadedFiles.insert(path);

	tinyxml2::XMLElement* root = document.RootElement();
	if (root == NULL || root->FirstChildElement() == NULL)
	{
		// root鑺傜偣涓嬫病鏈夊瓙鑺傜偣浜?
		return true;
	}

	for (tinyxml2::XMLElement* element = root->FirstChildElement();
		element != nullptr;
		element = element->NextSiblingElement())
	{
		tinyxml2::XMLElement* idElement = element->FirstChildElement("id");

		FixedMessages::MSGInfo info;
		info.msgid = idElement ? idElement->IntText() : 0;
		info.msgname = element->Value() ? element->Value() : "";

		_infomap[info.msgname] = info;
	}

	return true;
}

//-------------------------------------------------------------------------------------
FixedMessages::MSGInfo* FixedMessages::isFixed(const char* msgName)
{
	MSGINFO_MAP::iterator iter = _infomap.find(msgName);
	if (iter != _infomap.end())
	{
		MSGInfo* infos = &iter->second;
		return infos;
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
bool FixedMessages::isFixed(MessageID msgid)
{
	MSGINFO_MAP::iterator iter = _infomap.begin();
	while (iter != _infomap.end())
	{
		FixedMessages::MSGInfo& infos = iter->second;
		if (infos.msgid == msgid)
			return true;

		++iter;
	}

	return false;
}

//-------------------------------------------------------------------------------------
}
}
