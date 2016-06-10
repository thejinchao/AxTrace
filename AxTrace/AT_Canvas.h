#pragma once

#include "AT_Math.h"
#include "AT_Color.h"

namespace AT3
{ 
/** ª≠≤º¿‡
*/
class Canvas
{
public:
	static void drawPolygon(const fVector2* vertices, unsigned int vertexCount, const fColor& color, Real width=(Real)1.0);
	static void drawSolidPolygon(const fVector2* vertices, unsigned int vertexCount, const fColor& color);

	static void drawSolidCircle(const fVector2& center, Real radius, const fColor& color);
};

}
