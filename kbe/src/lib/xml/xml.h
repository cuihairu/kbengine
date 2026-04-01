// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_XMLP_H
#define KBE_XMLP_H

#include <string>

#include "common/common.h"
#include "common/smartpointer.h"
#include <tinyxml2.h>

namespace KBEngine {

#define XML_FOR_BEGIN(node) \
	do \
	{ \
	if((node) == nullptr || (node)->ToElement() == nullptr) \
			continue;

#define XML_FOR_END(node) \
	}while((node = node->NextSibling()));

class XML : public RefCountable
{
public:
	XML();
	explicit XML(const char* xmlFile);
	~XML();

	bool isGood() const { return isGood_; }

	bool openSection(const char* xmlFile);

	tinyxml2::XMLElement* getRootElement(void) { return rootElement_; }
	tinyxml2::XMLNode* getRootNode(const char* key = "");
	tinyxml2::XMLNode* enterNode(tinyxml2::XMLNode* node, const char* key);
	bool hasNode(tinyxml2::XMLNode* node, const char* key);

	tinyxml2::XMLDocument* getTxdoc() const { return txdoc_; }

	std::string getKey(const tinyxml2::XMLNode* node);
	std::string getValStr(const tinyxml2::XMLNode* node);
	std::string getVal(const tinyxml2::XMLNode* node);
	int getValInt(const tinyxml2::XMLNode* node);
	double getValFloat(const tinyxml2::XMLNode* node);
	bool getBool(const tinyxml2::XMLNode* node);

protected:
	tinyxml2::XMLDocument* txdoc_;
	tinyxml2::XMLElement* rootElement_;
	bool isGood_;
};

}

#endif // KBE_XMLP_H
