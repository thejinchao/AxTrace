#pragma once

namespace AT3
{

//predefine 
class LogMessage;
class ValueMessage;

class Filter
{
public:
	enum { kSystemDefault = 1 };

	struct ExtraColor
	{
		COLORREF		col;
		unsigned int	ext;
	};

	struct Result
	{
		bool		display;
		std::string wndTitle;
		ExtraColor	fontColor;
		ExtraColor	backColor;
	};

public:
	void onTraceMessage(const LogMessage* message, Result& result);
	void onValueMessage(const ValueMessage* message, Result& result);

public:
	Filter();
	virtual ~Filter();
};

}
