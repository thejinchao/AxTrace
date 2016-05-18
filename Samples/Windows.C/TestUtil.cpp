#include "StdAfx.h"
#include "TestUtil.h"
#include <strsafe.h>

//--------------------------------------------------------------------------------------------
std::string convertToMultibyte(const wchar_t* szSource)
{
	std::string strRet;
	if(!szSource || szSource[0] == 0) return strRet;

	char szTemp[2048]={0};
	int nActualChars = WideCharToMultiByte(CP_ACP,	// code page
								0,						// performance and mapping flags
								(LPCWSTR) szSource,		// wide-character string
								-1,						// number of chars in string
								szTemp,					// buffer for new string
								2048,				// size of buffer
								NULL,					// default for unmappable chars
								NULL);					// set when default char used

	return std::string(szTemp);
}

//--------------------------------------------------------------------------------------------
int encoded_size(wchar_t code_point)
{
	if (((unsigned int)code_point) < 0x80)
		return 1;
	else if (((unsigned int)code_point) < 0x0800)
		return 2;
	else if (((unsigned int)code_point) < 0x10000)
		return 3;
	else
		return 4;
}

//--------------------------------------------------------------------------------------------
int encode_ucs_to_utf8(const wchar_t* src, unsigned char* dest, int dest_len, int src_len)
{
	// count length for null terminated source...
	if(src_len == 0)
	{
		src_len = (int)wcslen(src);
	}

	int destCapacity = dest_len;

	// while there is data in the source buffer,
	for (int idx = 0; idx < src_len; ++idx)
	{
		wchar_t	cp = src[idx];

		// check there is enough destination buffer to receive this encoded unit (exit loop & return if not)
		if (destCapacity < encoded_size(cp))
		{
			break;
		}

		if (cp < 0x80)
		{
			*dest++ = (unsigned char)cp;
			--destCapacity;
		}
		else if (cp < 0x0800)
		{
			*dest++ = (unsigned char)((cp >> 6) | 0xC0);
			*dest++ = (unsigned char)((cp & 0x3F) | 0x80);
			destCapacity -= 2;
		}
		else if (cp < 0x10000)
		{
			*dest++ = (unsigned char)((cp >> 12) | 0xE0);
			*dest++ = (unsigned char)(((cp >> 6) & 0x3F) | 0x80);
			*dest++ = (unsigned char)((cp & 0x3F) | 0x80);
			destCapacity -= 3;
		}
		else
		{
			*dest++ = (unsigned char)((/*cp >> 18*/0) | 0xF0);
			*dest++ = (unsigned char)(((cp >> 12) & 0x3F) | 0x80);
			*dest++ = (unsigned char)(((cp >> 6) & 0x3F) | 0x80);
			*dest++ = (unsigned char)((cp & 0x3F) | 0x80);
			destCapacity -= 4;
		}

	}

	return dest_len - destCapacity;
}

//--------------------------------------------------------------------------------------------
std::string convertWideToUtf8(const wchar_t* szSource)
{
	char szTemp[2048]={0};
	encode_ucs_to_utf8(szSource, (unsigned char*)szTemp, 2048, 0);

	return std::string(szTemp);
}

//--------------------------------------------------------------------------------------------
std::wstring convertToWide(const char* szSource)
{
	std::wstring strRet;
	if(!szSource || szSource[0] == 0) return strRet;

	wchar_t szTemp[2048]={0};
	::MultiByteToWideChar(CP_ACP, 0, szSource, (int)strlen(szSource), szTemp, 2048);

	return std::wstring(szTemp);
}

//--------------------------------------------------------------------------------------------
std::string trimstring(const std::string& strIn)
{
	std::string strOut = strIn;
	static const std::string EMPTY = "";

	std::string::size_type start = strOut.find_first_not_of("\t \r\n");
	if(start == std::string::npos) return EMPTY;
	strOut = strOut.substr(start, std::string::npos);

	std::string::size_type end = strOut.find_last_not_of("\t \r\n");
	if(end == std::string::npos) return EMPTY;
	strOut = strOut.substr(0, end+1);

	return strOut;
}

