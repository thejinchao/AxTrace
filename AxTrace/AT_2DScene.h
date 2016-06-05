#pragma once

#include "AT_Math.h"

namespace AT3
{

//pre-define
class Graphics2DFrameWnd;

class Graphics2DScene
{
public:
	void cleanScene(Real x_size, Real y_size);

	void render(void);

private:
	Graphics2DFrameWnd* m_frame;

	fVector2 m_size;

public:
	Graphics2DScene(Graphics2DFrameWnd* frame);
	~Graphics2DScene();
};

}
