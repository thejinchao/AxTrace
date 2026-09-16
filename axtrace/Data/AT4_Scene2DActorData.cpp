/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2026
***************************************************/
#include "stdafx.h"
#include "AT4_Scene2DActorData.h"
#include "AT4_Common.h"

void Actor2DFilterResult::fillDefault()
{
	//default value
	display = true;
	type = Actor2DType::AT_CIRCLE;
	size = 10;
	borderColor = kColBlack;
	fillColor = kColWhite;
}
