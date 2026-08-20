/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2026
***************************************************/
#pragma once

struct Value
{
	QString updateTime;
	QString valueName;
	QString valueData;
	QColor backColor;
	QColor frontColor;
	int lineCounts = 0;
};

typedef QVector<Value> ValueVector;
typedef QHash<QString, ValueVector::size_type> ValueHashMap;

struct ValueFilterResult
{
	bool		display;
	QString		wndTitle;
	uint16_t	fontColor;
	uint16_t	backColor;
};
