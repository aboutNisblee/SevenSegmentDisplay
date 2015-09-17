import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Controls 1.3
import QtQuick.Dialogs 1.0
import de.nisble 1.0

Window {
    title: "SevenSegmentDisplay-Demo"
    visible: true
    width: 600
    height: 500

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
            text: "Digit size"
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
            text: "Digit count"
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
            value: 3
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
            text: "Precision"
        }

        Slider {
            id: sliderPrecision
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            orientation: Qt.Vertical
            maximumValue: sliderDigitCount.value - 1
            minimumValue: 0
            tickmarksEnabled: true
            stepSize: 1
            value: 0
            onMaximumValueChanged: value = Math.min(value, maximumValue)
        }
    }

    Rectangle {
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

        SevenSegmentDisplay {
            id: display
            anchors.fill: parent
            digitSize: 200 * sliderDigitSize.value
            digitCount: sliderDigitCount.value
            precision: sliderPrecision.value
            bgColor: ccBgColor.currentColor
            onColor: ccOnColor.currentColor
            offColor: ccOffColor.currentColor
            verticalAlignment: SevenSegmentDisplay.AlignCenter
            horizontalAlignment: SevenSegmentDisplay.AlignCenter

            value: 0
            onOverflow: console.log("digit: overflow")
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
        width: Math.max(gbColor.width, gbValue.width)

        Column {
            anchors.fill: parent
            anchors.margins: 5

            spacing: 10

            GroupBox {
                id: gbColor
                anchors.horizontalCenter: parent.horizontalCenter

                title: "Color"

                Column {
                    spacing: 10

                    Button {
                        id: btBgColor
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Background"
                        onClicked: ccBgColor.open()
                    }
                    Button {
                        id: btOnColor
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Enabled"
                        onClicked: ccOnColor.open()
                    }
                    Button {
                        id: btOffColor
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Disabled"
                        onClicked: ccOffColor.open()
                    }
                }
            } // GroupBox

            GroupBox {
                id: gbValue
                anchors.horizontalCenter: parent.horizontalCenter

                title: "Value"

                Column {
                    anchors.fill: parent
                    spacing: 10

                    ExclusiveGroup {
                        id: inputChooser
                    }
                    RadioButton {
                        text: "Manual"
                        checked: true
                        exclusiveGroup: inputChooser
                    }
                    RadioButton {
                        text: "Timer"
                        checked: false
                        exclusiveGroup: inputChooser
                    }
                }
            } // GroupBox
        } // Column
    } // rightControlPane

    ColorDialog {
        id: ccBgColor
        property color backup: "transparent"
        title: "Choose the background color"
        showAlphaChannel: true
        color: "transparent"
        onAccepted: backup = color
        onRejected: display.bgColor = ccBgColor.backup
    }

    ColorDialog {
        id: ccOnColor
        property color backup: "transparent"
        title: "Choose the color for enabled segments"
        showAlphaChannel: true
        color: "black"
        onAccepted: backup = color
        onRejected: display.onColor = ccOnColor.backup
    }

    ColorDialog {
        id: ccOffColor
        property color backup: "transparent"
        title: "Choose the color for disabled segments"
        showAlphaChannel: true
        color: "transparent"
        onAccepted: backup = color
        onRejected: display.offColor = ccOffColor.backup
    }

    Timer {
        id: timer
        interval: 1000
        repeat: true
        running: true
        property int i: 0
        onTriggered: {
            i = (i + 1) % Math.pow(10, display.digitCount)
            display.value = i
        }
    }
}
