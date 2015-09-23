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
