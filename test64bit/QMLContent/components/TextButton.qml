import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic

Button {
    id: customButton

    // 支持外部设置宽高
    //width: 120
    //height: 40

    property bool isShowToolTip: false
    text: ""
    property string toolTip: ""

    //signal clicked()  // 自定义点击信号

    // 背景矩形
    background: Rectangle {
        id: bg
        implicitWidth: 120
        implicitHeight: 40
        anchors.fill: parent
        radius: 16
        color: {
            if(!customButton.enabled)
                return "#aaaaaa"       // disable颜色
            if (customButton.down)
                return "#0033aa"   // 按下颜色
            if (customButton.hovered)
                return "#0077cc"   // hover 颜色
            if(customButton.enabled)
                return "#00aaff"       // 默认颜色


        }
        Behavior on color { ColorAnimation { duration: 200 } }
    }

    // 鼠标区域
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: customButton.clicked()
    }

    // 文本居中显示
    contentItem: Text {
        anchors.centerIn: parent
        text: customButton.text
        color: "white"
        //font.pixelSize: Math.min(customButton.height * 0.5, 20)
        font.pixelSize: 20
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    // ToolTip
    ToolTip.delay: 500
    ToolTip.timeout: 5000
    ToolTip.visible: customButton.isShowToolTip ? hovered : false
    ToolTip.text: customButton.toolTip
}
