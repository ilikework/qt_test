// MyButton.qml
import QtQuick 6.4
import QtQuick.Controls 6.4

Item {
    id: root
    width: parent ? parent.width * 0.9 : 200
    height: 40

    // 文本
    property alias text: label.text

    // 状态
    property bool checked: false
    property bool hover: false

    // 颜色
    property color normalColor: "#00aaff"
    property color hoverColor: "#33bbff"
    property color checkedColor: "orange"
    property color currentColor: checked ? checkedColor : hover ? hoverColor : normalColor

    // 自定义 clicked 信号
    signal clicked()

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: 8
        border.color: "#005577"
        border.width: 2
        color: root.currentColor

        Behavior on color { ColorAnimation { duration: 120 } }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor

            onEntered: root.hover = true
            onExited: root.hover = false
            onClicked: {
                root.checked = !root.checked
                root.clicked()   // 发射自定义 clicked 信号
            }
        }
    }

    Text {
        id: label
        anchors.fill: parent
        anchors.leftMargin: 12
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        font.pixelSize: 20
        font.bold: true
        color: "white"
    }
}
