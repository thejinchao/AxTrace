#pragma once

namespace AT3
{

class fColor
{
public:
	float r, g, b, a;

public:
	static uint32_t convert_RGB555_to_RGB888(uint16_t rgb, uint16_t a=255);

public:
	fColor() : r(0.f), g(0.f), b(0.f), a(0.f) {}
	fColor(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
	fColor(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a) : r(_r / 255.f), g(_g / 255.f), b(_b / 255.f), a(_a / 255.f) {}
	fColor(unsigned int color)
		: r((color & 0xFF) / 255.f)
		, g(((color >> 8) & 0xFF) / 255.f)
		, b(((color >> 16) & 0xFF) / 255.f)
		, a(((color >> 24) & 0xFF) / 255.f) {
	}
};

}
