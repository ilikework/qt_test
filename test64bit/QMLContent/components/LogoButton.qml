import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic

Button{
    id: rootButton
    property url source: ""

    contentItem: Image {
        anchors.fill: parent
        anchors.margins: 12
        fillMode: Image.PreserveAspectFit
        source: rootButton.source
        smooth: true
        layer.enabled: true
    }
    background: Rectangle
    {
        radius: 16
        color:
        {
            if(parent.down)
                return "#00aaff"
            if(parent.hovered)
                return "orange"
            return "#00aaff"
        }
        Behavior on color { ColorAnimation { duration: 1200 } }
    }


    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: rootButton.clicked()
    }

    ToolTip.delay: 1000
    ToolTip.timeout: 5000
    ToolTip.visible: hovered
}
