// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_XMLP_H
#define KBE_XMLP_H

#include <string>

#include "common/common.h"
#include "common/smartpointer.h"
#include "dependencies/tinyxml/tinyxml.h"

namespace KBEngine {

#define XML_FOR_BEGIN(node) \
	do \
	{ \
	if(node->Type() != TiXmlNode::TINYXML_ELEMENT) \
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

	TiXmlElement* getRootElement(void) { return rootElement_; }
	TiXmlNode* getRootNode(const char* key = "");
	TiXmlNode* enterNode(TiXmlNode* node, const char* key);
	bool hasNode(TiXmlNode* node, const char* key);

	TiXmlDocument* getTxdoc() const { return txdoc_; }

	std::string getKey(const TiXmlNode* node);
	std::string getValStr(const TiXmlNode* node);
	std::string getVal(const TiXmlNode* node);
	int getValInt(const TiXmlNode* node);
	double getValFloat(const TiXmlNode* node);
	bool getBool(const TiXmlNode* node);

protected:
	TiXmlDocument* txdoc_;
	TiXmlElement* rootElement_;
	bool isGood_;
};

}

#endif // KBE_XMLP_H
