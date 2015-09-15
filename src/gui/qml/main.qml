import QtQuick 2.3
import QtQuick.Window 2.2
import de.nisble 1.0

Window {
    id: root
    visible: true
    width: 600
    height: 500

    MouseArea {
        anchors.fill: parent
        onClicked: timer.running = !timer.running
        onDoubleClicked: Qt.quit()
    }

    Row {
        anchors.fill: parent

        SevenSegmentDisplay {
            id: digit

            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            width: parent.width / 2
            digitSize: 120
            digitCount: 2
            bgColor: "black"
            onColor: "red"
            offColor: "black"
            verticalAlignment: SevenSegmentDisplay.AlignCenter
            horizontalAlignment: SevenSegmentDisplay.AlignCenter

            value: 0
//            onValueChanged: {
//                if (!(value % 2))
//                    visible = !visible
//            }
        }

        Column {
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            width: parent.width / 2

            SevenSegmentDisplay {
                id: digit2
                anchors {
                    left: parent.left
                    right: parent.right
                }
                height: parent.height / 2

                digitSize: 60
                digitCount: 4
                bgColor: "orange"
                onColor: "black"
                offColor: "gray"
                verticalAlignment: SevenSegmentDisplay.AlignCenter
                horizontalAlignment: SevenSegmentDisplay.AlignCenter

                value: 0
            }

            Text {
                id: text
                anchors {
                    left: parent.left
                    right: parent.right
                }
                height: parent.height / 2
                font.pointSize: 60
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: qsTr("0")
            }
        }
    }

    Timer {
        id: timer
        interval: 250
        repeat: true
        running: true
        property int i: 0
        onTriggered: {
            i = (i + 1) % 10
            digit.value = i
            digit2.value = i
            text.text = i
        }
    }
}
