/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2026
***************************************************/
#pragma once

class IChildWindow
{
public:
	virtual ~IChildWindow() = default;

	enum Type
	{
		CT_LOG,
		CT_VALUE,
		CT_2DMAP,
	};

	virtual Type getType(void) const = 0;

	virtual bool isPause(void) const = 0;
	virtual void switchPause(void) = 0;

	virtual bool copyAble(void) const = 0;
	virtual void onCopy(void) const = 0;

	virtual void clean(void) = 0;

	virtual void saveAs(void) = 0;
};
Q_DECLARE_INTERFACE(IChildWindow, "AT4.IChildWindow")

