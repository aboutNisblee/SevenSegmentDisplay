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
