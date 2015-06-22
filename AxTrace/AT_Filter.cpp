/***************************************************

				 	AXIA|Trace3

							(C) Copyright  Jean. 2013
***************************************************/

#include "StdAfx.h"
#include "AT_Filter.h"

namespace AT3
{

//--------------------------------------------------------------------------------------------
Filter::Filter()
{
}

//--------------------------------------------------------------------------------------------
Filter::~Filter()
{
}

//--------------------------------------------------------------------------------------------
void Filter::onTraceMessage(const LogMessage* message, Result& result)
{
	//default
	result.display = true;
	result.wndTitle = "default";
	result.fontColor.ext = kSystemDefault;
	result.backColor.ext = kSystemDefault;
}

//--------------------------------------------------------------------------------------------
void Filter::onValueMessage(const ValueMessage* message, Result& result)
{
	//default
	result.display = true;
	result.wndTitle = "default";
	result.fontColor.ext = kSystemDefault;
	result.backColor.ext = kSystemDefault;
}

}
