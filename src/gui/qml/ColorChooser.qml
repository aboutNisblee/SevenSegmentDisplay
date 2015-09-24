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
import QtQuick.Dialogs 1.0

Item {
    id: root
    property alias text: label.text
    property alias color: cd.currentColor

    implicitWidth: label.width + rect.width
    implicitHeight: label.height

    Item {
        anchors.fill: parent

        Label {
            id: label
            anchors.left: parent.left
            text: "Background"
        }
        Rectangle {
            id: rect
            anchors {
                right: parent.right
                verticalCenter: label.verticalCenter
            }
            border {
                width: 1
                color: "black"
            }
            height: label.height
            width: height
            color: cd.currentColor
        }
        MouseArea {
            anchors.fill: rect
            onClicked: cd.open()
        }
    }

    ColorDialog {
        id: cd
        property color backup: "transparent"
        title: "Choose the background color"
        showAlphaChannel: true
        color: "transparent"
        onAccepted: backup = color
        onRejected: currentColor = backup
    }
}
