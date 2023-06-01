/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#pragma once

class IChild
{
public:
	enum Type
	{
		CT_LOG,
		CT_VALUE,
		CT_2DMAP,
	};

	const static char* PropertyName;// = "ChildInterface";

	virtual Type getType(void) const = 0;
	virtual QString getTitle(void) const = 0;

	virtual bool isPause(void) const = 0;
	virtual void switchPause(void) = 0;

	virtual bool copyAble(void) const = 0;
	virtual void onCopy(void) const = 0;

	virtual void clean(void) = 0;

	virtual void saveAs(void) = 0;

	virtual void update(void) = 0;

	//for map2d only
	virtual void flipX(void) { }

	virtual void rotateCW(void) { }
};

class ChildVariant
{
public:
	IChild* child;

	ChildVariant(IChild* _child = nullptr) : child(_child) {}
	ChildVariant(const ChildVariant& other) : child(other.child) {}
};

Q_DECLARE_METATYPE(ChildVariant)
