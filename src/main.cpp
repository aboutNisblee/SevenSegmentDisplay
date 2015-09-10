/**
 * \file main.cpp
 *
 * \date 04.09.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <gui/sevensegmentdisplay.hpp>

int main(int argc, char* argv[])
{
	QGuiApplication app(argc, argv);

	qmlRegisterType<SevenSegmentDisplay>("de.nisble", 1, 0, "SevenSegmentDisplay");

	QQmlApplicationEngine engine;
	engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
	return app.exec();
}
