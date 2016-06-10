#include "StdAfx.h"
#include "AT_Canvas.h"

namespace AT3
{

//-------------------------------------------------------------------------------------------------------------------
void Canvas::drawPolygon(const fVector2* vertices, unsigned int vertexCount, const fColor& color, Real width)
{
	glColor4f(color.r, color.g, color.b, color.a);
	glLineWidth((GLfloat)width);
	glBegin(GL_LINE_LOOP);
	for (unsigned int i = 0; i < vertexCount; ++i)
	{
		glVertex2d(vertices[i].x, vertices[i].y);
	}
	glEnd();
}

//-------------------------------------------------------------------------------------------------------------------
void Canvas::drawSolidPolygon(const fVector2* vertices, unsigned int vertexCount, const fColor& color)
{
	glColor4f(color.r, color.g, color.b, color.a);

	glBegin(GL_TRIANGLE_FAN);
	for (unsigned int i = 0; i < vertexCount; ++i)
	{
		glVertex2d(vertices[i].x, vertices[i].y);
	}
	glEnd();

}

//-------------------------------------------------------------------------------------------------------------------
void Canvas::drawSolidCircle(const fVector2& center, Real radius, const fColor& color)
{
	const float k_segments = 32.0f;
	const float k_increment = (float)MATH_PI_DOUBLE / k_segments;
	float theta = 0.0f;

	glColor4f(color.r, color.g, color.b, color.a);

	glBegin(GL_TRIANGLE_FAN);

	for (int i = 0; i < k_segments; ++i)
	{
		fVector2 p = center + radius * fVector2(cosf(theta), sinf(theta));

		glVertex2d(p.x, p.y);
		theta += k_increment;
	}
	glEnd();
}

}
