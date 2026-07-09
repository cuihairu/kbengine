// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SERVER_ERRORS_XML_H
#define KBE_SERVER_ERRORS_XML_H

#include "common/md5.h"
#include "resmgr/resmgr.h"
#include <map>
#include <tinyxml2.h>

namespace KBEngine {
namespace xml {

using ServerErrorDescriptions = std::map<uint16, std::pair<std::string, std::string> >;

inline void collectServerErrorDescriptions(const tinyxml2::XMLDocument& document,
	ServerErrorDescriptions& errors)
{
	const tinyxml2::XMLElement* root = document.RootElement();
	if (root == nullptr)
	{
		return;
	}

	for (const tinyxml2::XMLElement* element = root->FirstChildElement();
		element != nullptr;
		element = element->NextSiblingElement())
	{
		const tinyxml2::XMLElement* idElement = element->FirstChildElement("id");
		const tinyxml2::XMLElement* descrElement = element->FirstChildElement("descr");

		const uint16 id = idElement ? uint16(idElement->IntText()) : 0;
		const std::string name = element->Value() ? element->Value() : "";
		const char* descr = descrElement ? descrElement->GetText() : nullptr;
		errors[id] = std::make_pair(name, descr ? descr : "");
	}
}

template <typename Callback>
inline bool forEachServerErrorDescriptionDocument(const char* resourcePath,
	const char* context,
	bool required,
	Callback&& callback)
{
	FILE* file = Resmgr::getSingleton().openRes(resourcePath);
	if (file == nullptr)
	{
		return !required;
	}

	fclose(file);

	tinyxml2::XMLDocument document;
	const std::string path = Resmgr::getSingleton().matchRes(resourcePath);

	if (document.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS)
	{
		const std::string message = fmt::format("{}: load {} is failed! error: {}\n",
			context, path, document.ErrorStr());
		fprintf(stderr, "%s", message.c_str());
		return false;
	}

	tinyxml2::XMLElement* root = document.RootElement();
	if (root == nullptr)
	{
		return true;
	}

	for (tinyxml2::XMLElement* element = root->FirstChildElement();
		element != nullptr;
		element = element->NextSiblingElement())
	{
		tinyxml2::XMLElement* idElement = element->FirstChildElement("id");
		tinyxml2::XMLElement* descrElement = element->FirstChildElement("descr");
		callback(element, idElement, descrElement);
	}

	return true;
}

inline bool loadServerErrorDescriptions(ServerErrorDescriptions& errors, const char* context)
{
	if (!forEachServerErrorDescriptionDocument(
			"server/server_errors_defaults.xml",
			context,
			true,
			[&errors](const tinyxml2::XMLElement* element,
				const tinyxml2::XMLElement* idElement,
				const tinyxml2::XMLElement* descrElement)
			{
				const uint16 id = idElement ? uint16(idElement->IntText()) : 0;
				const std::string name = element->Value() ? element->Value() : "";
				const char* descr = descrElement ? descrElement->GetText() : nullptr;
				errors[id] = std::make_pair(name, descr ? descr : "");
			}))
	{
		return false;
	}

	if (!forEachServerErrorDescriptionDocument(
			"server/server_errors.xml",
			context,
			false,
			[&errors](const tinyxml2::XMLElement* element,
				const tinyxml2::XMLElement* idElement,
				const tinyxml2::XMLElement* descrElement)
			{
				const uint16 id = idElement ? uint16(idElement->IntText()) : 0;
				const std::string name = element->Value() ? element->Value() : "";
				const char* descr = descrElement ? descrElement->GetText() : nullptr;
				errors[id] = std::make_pair(name, descr ? descr : "");
			}))
	{
		return false;
	}

	return true;
}

inline bool appendServerErrorDescriptionsDigest(KBE_MD5& md5, int32& isize, const char* context)
{
	const auto appendDocument = [&md5, &isize](const tinyxml2::XMLElement* element,
		const tinyxml2::XMLElement* idElement,
		const tinyxml2::XMLElement* descrElement)
	{
		const int32 id = idElement ? idElement->IntText() : 0;
		md5.append((void*)&id, sizeof(int32));

		const std::string name = element->Value() ? element->Value() : "";
		md5.append((void*)name.c_str(), name.size());

		const char* descr = descrElement ? descrElement->GetText() : nullptr;
		const std::string descrText = descr ? descr : "";
		md5.append((void*)descrText.c_str(), descrText.size());
		isize++;
	};

	if (!forEachServerErrorDescriptionDocument(
			"server/server_errors_defaults.xml",
			context,
			true,
			appendDocument))
	{
		return false;
	}

	md5.append((void*)&isize, sizeof(int32));

	if (!forEachServerErrorDescriptionDocument(
			"server/server_errors.xml",
			context,
			false,
			appendDocument))
	{
		return false;
	}

	md5.append((void*)&isize, sizeof(int32));
	return true;
}

}
}

#endif // KBE_SERVER_ERRORS_XML_H
