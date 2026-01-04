import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic

Button {
    id: rootButton
    property url source: ""
    property real iconScaleY: 2    // 竖向放大倍数

    contentItem: Item {
        anchors.fill: parent

        // 让“绘制缩放”以按钮中心为原点，保证居中
        transform: Scale {
            xScale: 1
            yScale: rootButton.iconScaleY
            origin.x: width / 2
            origin.y: height / 2
        }

        Image {
            id: icon
            anchors.centerIn: parent
            width: parent.width - 24
            height: parent.height - 24
            fillMode: Image.PreserveAspectFit
            source: rootButton.source
            smooth: true
            layer.enabled: true
        }
    }

    background: Rectangle {
        radius: 16
        color: parent.down ? "#00aaff"
             : parent.hovered ? "orange"
             : "#00aaff"
        Behavior on color { ColorAnimation { duration: 1200 } }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: rootButton.clicked()
    }
}
