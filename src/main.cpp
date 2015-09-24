/* Copyright (C) 2015  Moritz Nisblé <moritz.nisble@gmx.de>
 *
 * This file is part of SevenSegmentsDisplay.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
