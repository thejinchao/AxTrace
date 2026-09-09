/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2026
***************************************************/
#pragma once

struct MessageTime
{
	qint64 epochTime;

	QString toString(void) const;

	bool operator==(const MessageTime& o) const { return epochTime == o.epochTime; }
	bool operator!=(const MessageTime& o) const { return epochTime != o.epochTime; }
	bool operator<(const MessageTime& o) const { return epochTime < o.epochTime; }
	bool operator>(const MessageTime& o) const { return epochTime > o.epochTime; }
};
