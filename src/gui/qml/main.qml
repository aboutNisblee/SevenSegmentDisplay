import QtQuick 2.3
import QtQuick.Window 2.2
import de.nisble 1.0

Window {
    visible: true
    width: 300
    height: 500

    MouseArea {
        anchors.fill: parent
        onClicked: timer.running = !timer.running
        onDoubleClicked: Qt.quit()
    }

    SevenSegmentDisplay {
        id: digit
        anchors {
            top: parent.top
            bottom: parent.bottom
        }
        value: 0
    }

    Timer {
        id: timer
        interval: 1000
        repeat: true
        running: true
        property int i: 0
        onTriggered: {
            i = (i + 1) % 10
            digit.value = i
        }
    }
}
