/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/

#include "StdAfx.h"
#include "AT_Util.h"
#include "utf/ConvertUTF.h"

namespace AT3
{

//--------------------------------------------------------------------------------------------
void PrintMemoryToString(const char* pMemory, int sizeOfBytes, std::wstring& strOutput)
{
	// address
	wchar_t szTemp[MAX_PATH];
	StringCchPrintfW(szTemp, MAX_PATH, L"%08X\t", (unsigned int)(UINT_PTR)pMemory);

	strOutput = szTemp;

	//contents(as bytes data)
	for(int i=0; i<sizeOfBytes; i++)
	{
		StringCchPrintfW(szTemp, MAX_PATH, L"%02X ", (unsigned char)(pMemory[i]));
		strOutput += szTemp;
	}

	//contents(as character)
	for(int i=0; i<sizeOfBytes; i++)
	{
		char theChar = pMemory[i];
		if(theChar >= ' ' && theChar <= '~')
		{
			szTemp[0] = theChar; szTemp[1]=0;
		}
		else
		{
			szTemp[0]='.'; szTemp[1]=0;
		}

		strOutput += szTemp;
	}
}

//--------------------------------------------------------------------------------------------
wchar_t* _getStaticWideCharBuf(size_t sizeInChar, size_t* bufSizeInChar=0)
{
	static wchar_t* s_wszBuf = 0;
	static size_t	s_wszBufSizeInChar = 0;

	size_t oldSize = s_wszBufSizeInChar;
	while(sizeInChar>s_wszBufSizeInChar)
	{
		s_wszBufSizeInChar = ((s_wszBufSizeInChar==0) ? 1 : s_wszBufSizeInChar*2);
	}

	if(s_wszBufSizeInChar != oldSize)
	{
		wchar_t* newBuf = new wchar_t[s_wszBufSizeInChar];
		if(s_wszBuf)
		{
			delete[] s_wszBuf;
		}

		s_wszBuf = newBuf;
	}

	if(bufSizeInChar) *bufSizeInChar = s_wszBufSizeInChar;
	return s_wszBuf;
}

//--------------------------------------------------------------------------------------------
char* _getStaticCharBuf(size_t sizeInByte, size_t* bufSizeInByte = 0)
{
	static char* s_szBuf = 0;
	static size_t	s_szBufSizeInByte = 0;

	size_t oldSize = s_szBufSizeInByte;
	while (sizeInByte>s_szBufSizeInByte)
	{
		s_szBufSizeInByte = ((s_szBufSizeInByte == 0) ? 1 : s_szBufSizeInByte * 2);
	}

	if (s_szBufSizeInByte != oldSize)
	{
		char* newBuf = new char[s_szBufSizeInByte];
		if (s_szBuf)
		{
			delete[] s_szBuf;
		}

		s_szBuf = newBuf;
	}

	if (bufSizeInByte) *bufSizeInByte = s_szBufSizeInByte;
	return s_szBuf;
}

//--------------------------------------------------------------------------------------------
const wchar_t* convertUTF8ToUTF16(const char* utf8_string, size_t byte_len)
{
	enum { MAX_CHARACTER = 8*1024*1024 };

	if(utf8_string==0 || byte_len ==0) return L"";
	if(byte_len>MAX_CHARACTER) byte_len = MAX_CHARACTER;

	//get wsz buf
	size_t wszCharCount = 0;
	wchar_t* wszBuf = _getStaticWideCharBuf(byte_len, &wszCharCount);
	wszBuf[0]=0;

	UTF8*	pSourceStart = (UTF8*)utf8_string;
	UTF16*	pTargetStart = (UTF16*)wszBuf;

	ConvertUTF8toUTF16((const UTF8 **)&pSourceStart, pSourceStart+ byte_len, &pTargetStart, pTargetStart+wszCharCount, strictConversion);
	
	return wszBuf;
}

//--------------------------------------------------------------------------------------------
const char* convertUTF16ToUTF8(const wchar_t* utf16_string, size_t char_len)
{
	enum { MAX_CHARACTER = 8 * 1024 * 1024 };

	if (utf16_string == 0 || char_len == 0) return "";
	if (char_len>MAX_CHARACTER) char_len = MAX_CHARACTER;

	size_t bufLength = 0;
	char* buf = _getStaticCharBuf(char_len*4, &bufLength);

	UTF8* buf_start = (UTF8*)buf;
	const UTF16* msg_start = (const UTF16*)utf16_string;

	ConvertUTF16toUTF8(&msg_start, msg_start+char_len, &buf_start, buf_start+ bufLength, strictConversion);

	return buf;
}

//--------------------------------------------------------------------------------------------
AutoSizeBuf::AutoSizeBuf()
	: m_pBuf(0)
	, m_nSize(0)
{
}

//--------------------------------------------------------------------------------------------
AutoSizeBuf::~AutoSizeBuf()
{
	if(m_pBuf) delete[] m_pBuf;
	m_pBuf=0;
	m_nSize=0;
}

//--------------------------------------------------------------------------------------------
void* AutoSizeBuf::request(size_t sizeOfBytes)
{
	size_t oldSize = m_nSize;
	size_t newSize = m_nSize;
	while(sizeOfBytes>newSize)
	{
		newSize = ((newSize==0) ? 1 : newSize*2);
		if(newSize>MAX_SIZE_OF_BYTES) return 0;	//exception!
	}

	if(newSize != oldSize)
	{
		char* newBuf = new char[newSize];
		if(m_pBuf)
		{
			memcpy(newBuf, m_pBuf, oldSize);
			delete[] m_pBuf;
		}

		m_pBuf = newBuf;
		m_nSize = newSize;
	}

	return (void*)m_pBuf;
}

}