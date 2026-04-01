// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "xml.h"
namespace KBEngine{

XML::XML():
	txdoc_(NULL),
	rootElement_(NULL),
	isGood_(false)
{
}

XML::XML(const char* xmlFile):
	txdoc_(NULL),
	rootElement_(NULL),
	isGood_(false)
{
	openSection(xmlFile);
}

XML::~XML()
{
	if(txdoc_)
	{
		txdoc_->Clear();
		delete txdoc_;
		txdoc_ = NULL;
		rootElement_ = NULL;
	}
}

bool XML::openSection(const char* xmlFile)
{
	char pathbuf[MAX_PATH];
	kbe_snprintf(pathbuf, MAX_PATH, "%s", xmlFile);

	txdoc_ = new TiXmlDocument((char*)&pathbuf);

	if(!txdoc_->LoadFile())
	{
		const std::string message = fmt::format("TiXmlNode::openXML: {}, error!\n", pathbuf);
		fprintf(stderr, "%s", message.c_str());

		isGood_ = false;
		return false;
	}

	rootElement_ = txdoc_->RootElement();
	isGood_ = true;
	return true;
}

TiXmlNode* XML::getRootNode(const char* key)
{
	if(rootElement_ == NULL)
		return rootElement_;

	if(strlen(key) > 0)
	{
		TiXmlNode* node = rootElement_->FirstChild(key);
		if(node == NULL)
			return NULL;
		return node->FirstChild();
	}

	return rootElement_->FirstChild();
}

TiXmlNode* XML::enterNode(TiXmlNode* node, const char* key)
{
	do
	{
		if(node->Type() != TiXmlNode::TINYXML_ELEMENT)
			continue;

		if(getKey(node) == key)
		{
			TiXmlNode* childNode = node->FirstChild();
			do
			{
				if(!childNode || childNode->Type() != TiXmlNode::TINYXML_COMMENT)
					break;
			}
			while((childNode = childNode->NextSibling()));

			return childNode;
		}
	}
	while((node = node->NextSibling()));

	return NULL;
}

bool XML::hasNode(TiXmlNode* node, const char* key)
{
	do
	{
		if(node->Type() != TiXmlNode::TINYXML_ELEMENT)
			continue;

		if(getKey(node) == key)
			return true;

	}
	while((node = node->NextSibling()));

	return false;
}

std::string XML::getKey(const TiXmlNode* node)
{
	if(node == NULL)
		return "";

	return strutil::kbe_trim(node->Value());
}

std::string XML::getValStr(const TiXmlNode* node)
{
	const TiXmlText* ptext = node->ToText();
	if(ptext == NULL)
		return "";

	return strutil::kbe_trim(ptext->Value());
}

std::string XML::getVal(const TiXmlNode* node)
{
	const TiXmlText* ptext = node->ToText();
	if(ptext == NULL)
		return "";

	return ptext->Value();
}

int XML::getValInt(const TiXmlNode* node)
{
	const TiXmlText* ptext = node->ToText();
	if(ptext == NULL)
		return 0;

	return atoi(strutil::kbe_trim(ptext->Value()).c_str());
}

double XML::getValFloat(const TiXmlNode* node)
{
	const TiXmlText* ptext = node->ToText();
	if(ptext == NULL)
		return 0.f;

	return atof(strutil::kbe_trim(ptext->Value()).c_str());
}

bool XML::getBool(const TiXmlNode* node)
{
	std::string s = strutil::toUpper(getValStr(node));

	if(s == "TRUE")
		return true;

	if(s == "FALSE")
		return false;

	return getValInt(node) > 0;
}

}
