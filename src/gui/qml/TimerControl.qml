/* Copyright (C) 2015  Moritz Nisblé <moritz.nisble@gmx.de>
**
** This file is part of SevenSegmentsDisplay.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published
** by the Free Software Foundation, either version 3 of the License,
** or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
import QtQuick 2.3
import QtQuick.Controls 1.3

Item {
    id: root
    property int digitCount
    property int precision
    property alias value: timer.i

    signal tick(real value)

    Column {
        id: col
        spacing: 5
        anchors {
            left: parent.left
            right: parent.right
            margins: 5
        }

        Label {
            id: lbInterval
            text: qsTr("Interval:")
            anchors.left: parent.left
            anchors.leftMargin: 5
        }
        SpinBox {
            id: sbInterval
            anchors {
                left: parent.left
                right: parent.right
            }
            minimumValue: 10
            maximumValue: 10000
            value: 1000
        }
        Label {
            id: lbSteps
            text: qsTr("Step size:")
            anchors.left: parent.left
            anchors.leftMargin: 5
        }
        SpinBox {
            id: sbSteps
            anchors {
                left: parent.left
                right: parent.right
            }
            minimumValue: -100
            maximumValue: 100
            decimals: root.precision
            value: 1.5
            stepSize: Math.pow(10, -root.precision)
        }
        Button {
            text: timer.running ? qsTr("Stop") : qsTr("Start")
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.left: parent.left
            anchors.leftMargin: 10
            onClicked: timer.running = !timer.running
        }
    }

    Timer {
        id: timer
        interval: sbInterval.value
        repeat: true
        running: false
        property real i: 0
        onTriggered: {
            i = (i + sbSteps.value) % Math.pow(
                        10, root.digitCount - root.precision - (i < 0 ? 1 : 0))
            tick(i)
        }
    }
}
