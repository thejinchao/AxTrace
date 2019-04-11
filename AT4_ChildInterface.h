#pragma once

class IChild : public QObjectUserData
{
public:
	enum Type
	{
		CT_LOG,
		CT_VALUE,
		CT_2DMAP,
	};

	virtual Type getType(void) const = 0;
	virtual QString getTitle(void) const = 0;

	virtual bool copyAble(void) const = 0;
	virtual void onCopy(void) const = 0;

	virtual void clean(void) = 0;

	virtual void saveAs(void) = 0;

	virtual void update(void) = 0;
};

