/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2026
***************************************************/
#pragma once

#include "Data/AT4_CommonStruct.h"

struct Value
{
	int index;
	MessageTime updateTime;
	QString updateTimeStr;
	QString valueName;
	QString valueData;
	QColor backColor;
	QColor frontColor;
	int lineCounts = 0;
};

typedef QVector<Value> ValueVector;
typedef QHash<QString, ValueVector::size_type> ValueHashMap;
typedef QVector<ValueVector::size_type> ValueIndexVector;

struct ValueFilterResult
{
	bool		display;
	QString		wndTitle;
	uint16_t	fontColor;
	uint16_t	backColor;
};
