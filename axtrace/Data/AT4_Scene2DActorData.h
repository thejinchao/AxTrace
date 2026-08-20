/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2026
***************************************************/
#pragma once

enum Actor2DType
{
	AT_CIRCLE,
	AT_QUAD,
	AT_TRIANGLE,
};

struct Actor2D
{
	qint64 actorID;
	QPointF pos;
	qreal dir;
	Actor2DType type;
	quint16 borderColor;
	quint16 fillColor;
	qreal size;
	QString info;
};

struct Actor2DFilterResult
{
	bool			display;
	Actor2DType		type;
	int				size;
	uint16_t		borderColor;
	uint16_t		fillColor;
};
