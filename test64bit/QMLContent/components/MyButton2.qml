// MyButton2.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

Button {
    id: rootButton
    anchors.horizontalCenter: parent.horizontalCenter   // 水平居中
    width: 200            // 宽度 = 父项宽度的 90%，防止 parent 为空时报错

    font.pixelSize: 18
    // 文字左对齐
    contentItem: Text {
        text: parent.text
        font: parent.font
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        elide: Text.ElideRight
        color: rootButton.enabled ? "white" : "#888"
        anchors.fill: parent
        anchors.leftMargin: 12   // 留点内边距
    }
	
    background: Rectangle {
        implicitWidth: 120
        implicitHeight: 40
        color: rootButton.pressed ? "#888" : "#00aaff"
        border.color: "#005577"
        border.width: 2
    }

}
