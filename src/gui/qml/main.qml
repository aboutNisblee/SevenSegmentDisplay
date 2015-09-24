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
import QtQuick.Window 2.2
import QtQuick.Controls 1.3
import QtQuick.Dialogs 1.0
import de.nisble 1.0

Window {
    title: "SevenSegmentDisplay-Demo"
    visible: true
    width: 750
    height: 400

    Row {
        id: leftControlPane
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            margins: 5
        }
        width: childrenRect.width

        Label {
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            width: contentHeight
            rotation: -90
            font.pointSize: 12
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: qsTr("Digit size")
        }

        Slider {
            id: sliderDigitSize
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            orientation: Qt.Vertical
            value: 0.5
        }

        Label {
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            width: contentHeight
            rotation: -90
            font.pointSize: 12
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: qsTr("Digit count")
        }

        Slider {
            id: sliderDigitCount
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            orientation: Qt.Vertical
            maximumValue: 8
            minimumValue: 0
            tickmarksEnabled: true
            stepSize: 1
            value: 4
        }

        Label {
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            width: contentHeight
            rotation: -90
            font.pointSize: 12
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: qsTr("Precision")
        }

        Slider {
            id: sliderPrecision
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            orientation: Qt.Vertical
            maximumValue: (sliderDigitCount.value - 1 < 0) ? 0 : sliderDigitCount.value - 1
            //            maximumValue: sliderDigitCount.value
            minimumValue: 0
            tickmarksEnabled: true
            stepSize: 1
            value: 2
            onMaximumValueChanged: {
                //                console.log("onMaximumValueChanged to " + maximumValue
                //                            + " current value " + value + " choosing " + Math.min(
                //                                value, maximumValue))
                value = Math.min(value, maximumValue)
            }
        }
    }

    Rectangle {
        id: displayBorder
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: leftControlPane.right
            right: rightControlPane.left
            margins: 5
        }
        border {
            color: "black"
            width: 1
        }
        Timer {
            id: tBlink
            interval: 250
            onTriggered: {
                displayBorder.border.color = "black"
                displayBorder.border.width = 1
            }
        }

        SevenSegmentDisplay {
            id: display
            anchors.fill: parent
            digitSize: 200 * sliderDigitSize.value
            digitCount: sliderDigitCount.value
            precision: sliderPrecision.value
            bgColor: ccBgColor.color
            onColor: ccOnColor.color
            offColor: ccOffColor.color
            verticalAlignment: SevenSegmentDisplay.AlignCenter
            horizontalAlignment: SevenSegmentDisplay.AlignCenter

            value: -3.14
            onOverflow: {
                console.log("digit: Overflow")
                displayBorder.border.color = "red"
                displayBorder.border.width = 3
                tBlink.restart()
            }
        }
    }

    Rectangle {
        id: rightControlPane
        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
            margins: 5
        }
        width: 150

        Column {
            anchors.fill: parent
            anchors.margins: 5

            spacing: 10

            Label {
                width: parent.width
                text: qsTr("Options")
                font.bold: true
            }

            GroupBox {
                id: gbColor
                width: parent.width

                Column {
                    anchors.fill: parent
                    spacing: 10

                    ColorChooser {
                        id: ccBgColor
                        width: parent.width
                        text: qsTr("Background:")
                    }

                    ColorChooser {
                        id: ccOnColor
                        width: parent.width
                        text: qsTr("Enabled:")
                        color: "black"
                    }

                    ColorChooser {
                        id: ccOffColor
                        width: parent.width
                        text: qsTr("Disabled:")
                    }
                }
            } // GroupBox

            TabView {
                id: tabView
                width: parent.width
                Tab {
                    id: tabTimer
                    active: true
                    title: qsTr("Timer")
                    Item {
                        TimerControl {
                            anchors.fill: parent
                            digitCount: sliderDigitCount.value
                            precision: sliderPrecision.value
                            onTick: display.value = value
                        }
                    }
                }
                Tab {
                    id: tabManual
                    active: true
                    title: qsTr("Manual")
                    Item {
                        TextField {
                            anchors {
                                left: parent.left
                                right: parent.right
                                top: parent.top
                                margins: 5
                            }
                            placeholderText: qsTr("Enter value")
                            onTextChanged: display.string = text
                        }
                    }
                }
            } // TabView
        } // Column
    } // rightControlPane
}
