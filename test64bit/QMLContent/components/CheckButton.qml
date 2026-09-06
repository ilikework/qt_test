// MyButton.qml
import QtQuick 6.4
import QtQuick.Controls 6.4

Item {
    id: root
    implicitWidth: label.implicitWidth + 24
    width: implicitWidth

    property int buttonHeight: 40
    property int fontPixelSize: 20
    property int cornerRadius: 8
    property int borderW: 2

    height: buttonHeight

    // 文本
    property alias text: label.text

    // 状态
    property bool checked: false
    property bool hover: false
    /// false：点击不切换 checked，仅发出 clicked（由外部 Binding 单独驱动 checked，避免与异步结果竞态）
    property bool autoToggle: true

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
        radius: root.cornerRadius
        border.color: "#005577"
        border.width: root.borderW
        color: root.currentColor

        Behavior on color { ColorAnimation { duration: 120 } }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor

            onEntered: root.hover = true
            onExited: root.hover = false
            onClicked: {
                if (root.autoToggle)
                    root.checked = !root.checked
                root.clicked()
            }
        }
    }

    Text {
        id: label
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: root.fontPixelSize
        font.bold: true
        color: "white"
        elide: Text.ElideRight
    }

    ToolTip {
        visible: label.truncated && mouseArea.containsMouse
        text: label.text
        delay: 400
    }
}
