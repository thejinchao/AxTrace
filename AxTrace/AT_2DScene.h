#pragma once

#include "AT_Math.h"
#include "AT_Color.h"

namespace AT3
{

//pre-define
class Graphics2DFrameWnd;

class Graphics2DScene
{
public:
	void cleanScene(Real x_size, Real y_size, const fColor& borderColor, const fColor& backgroundColor);
	void updateActor(unsigned int id, Real x, Real y, Real dir);

	void render(void);

private:
	Graphics2DFrameWnd* m_frame;
	bool m_active;

	fVector2 m_size;
	fColor m_backgroundColor;
	fColor m_borderColor;

	//pre-define
	struct Actor;

	//Actor map
	typedef std::map< unsigned int, Actor* > ActorMap;
	ActorMap m_actorMap;


public:
	Graphics2DScene(Graphics2DFrameWnd* frame);
	~Graphics2DScene();
};

}
