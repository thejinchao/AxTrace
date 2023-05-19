/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"

#include "AT4_System.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(AT4);
	qputenv("QT_PLUGIN_PATH", QString("./plugins").toLocal8Bit());

	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	QSurfaceFormat::setDefaultFormat(format);

	QApplication::setStyle(QStyleFactory::create("Fusion"));

	System theSystem;
	if (!theSystem.init(argc, argv)) {
		return 0;
	}

	return theSystem.run();
}
