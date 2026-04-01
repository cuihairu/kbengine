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

	delete txdoc_;
	txdoc_ = new tinyxml2::XMLDocument();
	rootElement_ = NULL;

	if(txdoc_->LoadFile(pathbuf) != tinyxml2::XML_SUCCESS)
	{
		const std::string message = fmt::format("XML::openSection: {}, error: {}\n", pathbuf, txdoc_->ErrorStr());
		fprintf(stderr, "%s", message.c_str());

		isGood_ = false;
		return false;
	}

	rootElement_ = txdoc_->RootElement();
	isGood_ = true;
	return true;
}

tinyxml2::XMLNode* XML::getRootNode(const char* key)
{
	if(rootElement_ == NULL)
		return rootElement_;

	if(strlen(key) > 0)
	{
		tinyxml2::XMLElement* node = rootElement_->FirstChildElement(key);
		if(node == NULL)
			return NULL;
		return node->FirstChild();
	}

	return rootElement_->FirstChild();
}

tinyxml2::XMLNode* XML::enterNode(tinyxml2::XMLNode* node, const char* key)
{
	do
	{
		if(node == NULL || node->ToElement() == NULL)
			continue;

		if(getKey(node) == key)
		{
			tinyxml2::XMLNode* childNode = node->FirstChild();
			do
			{
				if(!childNode || childNode->ToComment() == NULL)
					break;
			}
			while((childNode = childNode->NextSibling()));

			return childNode;
		}
	}
	while((node = node->NextSibling()));

	return NULL;
}

bool XML::hasNode(tinyxml2::XMLNode* node, const char* key)
{
	do
	{
		if(node == NULL || node->ToElement() == NULL)
			continue;

		if(getKey(node) == key)
			return true;

	}
	while((node = node->NextSibling()));

	return false;
}

std::string XML::getKey(const tinyxml2::XMLNode* node)
{
	if(node == NULL)
		return "";

	return strutil::kbe_trim(node->Value());
}

std::string XML::getValStr(const tinyxml2::XMLNode* node)
{
	const tinyxml2::XMLText* ptext = node != NULL ? node->ToText() : NULL;
	if(ptext == NULL)
		return "";

	return strutil::kbe_trim(ptext->Value());
}

std::string XML::getVal(const tinyxml2::XMLNode* node)
{
	const tinyxml2::XMLText* ptext = node != NULL ? node->ToText() : NULL;
	if(ptext == NULL)
		return "";

	return ptext->Value();
}

int XML::getValInt(const tinyxml2::XMLNode* node)
{
	const tinyxml2::XMLText* ptext = node != NULL ? node->ToText() : NULL;
	if(ptext == NULL)
		return 0;

	return atoi(strutil::kbe_trim(ptext->Value()).c_str());
}

double XML::getValFloat(const tinyxml2::XMLNode* node)
{
	const tinyxml2::XMLText* ptext = node != NULL ? node->ToText() : NULL;
	if(ptext == NULL)
		return 0.f;

	return atof(strutil::kbe_trim(ptext->Value()).c_str());
}

bool XML::getBool(const tinyxml2::XMLNode* node)
{
	std::string s = strutil::toUpper(getValStr(node));

	if(s == "TRUE")
		return true;

	if(s == "FALSE")
		return false;

	return getValInt(node) > 0;
}

}
