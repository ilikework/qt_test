import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic

Button {
    id: customButton

    property bool isShowToolTip: false
    /// 未显式设 height 时的默认高度（各对话框保持 40；侧栏可改小）
    property int preferredHeight: 40
    property int preferredFontPixelSize: 20
    property int preferredRadius: 16

    implicitHeight: preferredHeight
    text: ""
    property string toolTip: ""

    //signal clicked()  // 自定义点击信号

    // 背景矩形
    background: Rectangle {
        id: bg
        implicitWidth: 120
        implicitHeight: customButton.preferredHeight
        anchors.fill: parent
        radius: customButton.preferredRadius
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
        font.pixelSize: customButton.preferredFontPixelSize
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
