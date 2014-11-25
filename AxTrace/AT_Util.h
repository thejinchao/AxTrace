/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#pragma once

namespace AT3
{

/** print memory contents to string(for debug)
*/
void PrintMemoryToString(const char* pMemory, int sizeOfBytes, std::wstring& strOutput);

/** auto lock
*/
struct AutoLock
{
	LPCRITICAL_SECTION	pCriticalSection;
	AutoLock(LPCRITICAL_SECTION p) : pCriticalSection(p) { ::EnterCriticalSection(pCriticalSection); }
	~AutoLock() { ::LeaveCriticalSection(pCriticalSection); }
};

/** convert to utf16 string(return as static memory block, call by main thread only)*/
const wchar_t* convertUTF8ToUTF16(const char* utf8_string, size_t len_string);

/**memory buf
*/
class AutoSizeBuf
{
public:
	enum { MAX_SIZE_OF_BYTES=1024*1024*256 };

	void*	request(size_t sizeOfBytes);
	size_t	capcity(void) const { return m_nSize; }

private:
	char*	m_pBuf;
	size_t	m_nSize;

public:
	AutoSizeBuf();
	~AutoSizeBuf();
};

}