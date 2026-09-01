/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2026
***************************************************/
#include "stdafx.h"
#include "AT4_CommonStruct.h"

//--------------------------------------------------------------------------------------------
QString MessageTime::toString(void) const
{
	const QTime t = QDateTime::fromMSecsSinceEpoch(epochTime).time();
	return QString("%1:%2 %3.%4")
		.arg(t.hour(), 2, 10, QLatin1Char('0'))
		.arg(t.minute(), 2, 10, QLatin1Char('0'))
		.arg(t.second(), 2, 10, QLatin1Char('0'))
		.arg(t.msec(), 3, 10, QLatin1Char('0'));
}
