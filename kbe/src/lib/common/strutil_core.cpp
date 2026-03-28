// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "common/strutil.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>

namespace KBEngine {
namespace strutil {

int bytes2string(unsigned char* src, int srcsize, unsigned char* dst, int dstsize)
{
	if (dst != NULL)
	{
		*dst = 0;
	}

	if (src == NULL || srcsize <= 0 || dst == NULL || dstsize <= srcsize * 2)
	{
		return 0;
	}

	const char szTable[] = "0123456789ABCDEF";

	for (int i = 0; i < srcsize; ++i)
	{
		*dst++ = szTable[src[i] >> 4];
		*dst++ = szTable[src[i] & 0x0f];
	}

	*dst = 0;
	return srcsize * 2;
}

int string2bytes(unsigned char* src, unsigned char* dst, int dstsize)
{
	if (src == NULL)
	{
		return 0;
	}

	int len = static_cast<int>(std::strlen(reinterpret_cast<char*>(src)));
	if (len <= 0 || len % 2 != 0 || dst == NULL || dstsize < len / 2)
	{
		return 0;
	}

	str_toupper(reinterpret_cast<char*>(src));

	for (int i = 0; i < len / 2; ++i)
	{
		int value = 0;
		unsigned char* current = src + i * 2;
		std::sscanf(reinterpret_cast<char*>(current), "%02x", &value);
		dst[i] = static_cast<unsigned char>(value);
	}

	return len / 2;
}

std::string toLower(const std::string& str)
{
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});
	return result;
}

std::string toUpper(const std::string& str)
{
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
		return static_cast<char>(std::toupper(c));
	});
	return result;
}

std::string& kbe_ltrim(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char c) {
		return !std::isspace(c);
	}));
	return s;
}

std::string& kbe_rtrim(std::string& s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char c) {
		return !std::isspace(c);
	}).base(), s.end());
	return s;
}

std::string kbe_trim(std::string s)
{
	return kbe_ltrim(kbe_rtrim(s));
}

int kbe_replace(std::string& str, const std::string& pattern, const std::string& newpat)
{
	int count = 0;
	const size_t replacement_size = newpat.size();
	const size_t pattern_size = pattern.size();

	for (size_t pos = str.find(pattern, 0);
		pos != std::string::npos;
		pos = str.find(pattern, pos + replacement_size))
	{
		str.replace(pos, pattern_size, newpat);
		++count;
	}

	return count;
}

int kbe_replace(std::wstring& str, const std::wstring& pattern, const std::wstring& newpat)
{
	int count = 0;
	const size_t replacement_size = newpat.size();
	const size_t pattern_size = pattern.size();

	for (size_t pos = str.find(pattern, 0);
		pos != std::wstring::npos;
		pos = str.find(pattern, pos + replacement_size))
	{
		str.replace(pos, pattern_size, newpat);
		++count;
	}

	return count;
}

int kbe_splits(const std::string& s, const std::string& delim, std::vector<std::string>& out_result, const bool keep_empty)
{
	if (delim.empty())
	{
		out_result.push_back(s);
		return static_cast<int>(out_result.size());
	}

	std::string::const_iterator substart = s.begin();

	while (true)
	{
		const std::string::const_iterator subend = std::search(substart, s.end(), delim.begin(), delim.end());
		std::string temp(substart, subend);
		if (keep_empty || !temp.empty())
		{
			out_result.push_back(temp);
		}

		if (subend == s.end())
		{
			break;
		}

		substart = subend + delim.size();
	}

	return static_cast<int>(out_result.size());
}

} // namespace strutil
} // namespace KBEngine
