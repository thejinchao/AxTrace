#include "StdAfx.h"
#include "AT_Canvas.h"

namespace AT3
{

//-------------------------------------------------------------------------------------------------------------------
void Canvas::drawPolygon(const fVector2* vertices, unsigned int vertexCount)
{
	glBegin(GL_TRIANGLE_FAN);
	for (unsigned int i = 0; i < vertexCount; ++i)
	{
		glVertex2d(vertices[i].x, vertices[i].y);
	}
	glEnd();

}

}
