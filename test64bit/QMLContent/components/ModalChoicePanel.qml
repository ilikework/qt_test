import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

/// 全区域模态选择面板（用于拍摄保存后、分析工作流等）
Item {
    id: root
    visible: false
    z: 2000
    anchors.fill: parent

    property string boxTitle: ""
    property string boxMessage: ""
    /// [{ id: "x", text: "按钮" }, ...]
    property var choices: []

    signal choiceMade(string choiceId)

    function open() { visible = true }
    function close() { visible = false }

    Rectangle {
        anchors.fill: parent
        color: "#99000000"
        MouseArea {
            anchors.fill: parent
            onPressed: function(mouse) { mouse.accepted = true }
            onReleased: function(mouse) { mouse.accepted = true }
            onWheel: function(wheel) { wheel.accepted = true }
        }
    }

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width - 48, 420)
        implicitHeight: panelCol.implicitHeight + 32
        height: implicitHeight
        color: "#252525"
        border.color: "#555"
        radius: 8

        ColumnLayout {
            id: panelCol
            anchors.fill: parent
            anchors.margins: 16
            spacing: 14

            Text {
                text: root.boxTitle
                color: "#ffffff"
                font.pixelSize: 16
                font.bold: true
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            Text {
                text: root.boxMessage
                color: "#e0e0e0"
                font.pixelSize: 14
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                Layout.minimumHeight: 36
            }
            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                spacing: 8

                Repeater {
                    model: root.choices
                    TextButton {
                        Layout.fillWidth: true
                        Layout.preferredWidth: Math.min(panelCol.width - 32, 320)
                        implicitHeight: 36
                        text: modelData.text
                        onClicked: {
                            root.choiceMade(modelData.id)
                            root.close()
                        }
                    }
                }
            }
        }
    }
}
