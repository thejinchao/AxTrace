/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2026
***************************************************/
#pragma once

enum Color {
	kColBlack = 0x000,
	kColWhite = 0xFFF,
	kColRed = 0x00F,
	kColGreen = 0x0F0,
	kColBlue = 0xF00,
	kColGray = 0x777,
	kColYellow = 0x0FF,
	kColOrange = 0x06F,
	kColViolet = 0xF0F
};

struct MessageTime
{
	qint64 epochTime;

	QString toString(void) const;

	bool operator==(const MessageTime& o) const { return epochTime == o.epochTime; }
	bool operator!=(const MessageTime& o) const { return epochTime != o.epochTime; }
	bool operator<(const MessageTime& o) const { return epochTime < o.epochTime; }
	bool operator>(const MessageTime& o) const { return epochTime > o.epochTime; }
};
