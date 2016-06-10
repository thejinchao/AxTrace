#include "StdAfx.h"
#include "AT_Color.h"

namespace AT3
{

//--------------------------------------------------------------------------------------------
uint32_t fColor::convert_RGB555_to_RGB888(uint16_t rgb, uint16_t a)
{
	// 0bbb bbgg gggr rrrr
	//          |
	//          v
	// 0000 0000 bbbb b000 gggg g000 rrrr r000
	uint32_t r = (rgb & 0x000F) * 0xFF / 0xF;
	uint32_t g = ((rgb & 0x00F0) >> 4) * 0xFF / 0xF;
	uint32_t b = ((rgb & 0x0F00) >> 8) * 0xFF / 0xF;
	
	return ((uint32_t)(((BYTE)(r) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(b)) << 16)) | (((DWORD)(BYTE)(a)) << 24));
}

}
