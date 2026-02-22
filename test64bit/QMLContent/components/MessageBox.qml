import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

// 通用模态消息框（Window，与 OfferingPickerDialog 一致）。可设置 boxTitle、boxMessage，确定按钮居中。
// 调用方请设置 transientParent 为主窗口以便居中（如 PreRecordDialog 中设为 preRecordWin）。
Window {
    id: root
    modality: Qt.ApplicationModal
    flags: Qt.Dialog | Qt.FramelessWindowHint
    width: 340
    height: 180
    minimumWidth: 300
    minimumHeight: 140
    color: "transparent"
    visible: false

    property string boxTitle: ""
    property string boxMessage: ""
    property string buttonText: "确定"
    signal confirmed()

    function open() { show() }

    onVisibleChanged: {
        if (visible && transientParent) {
            x = transientParent.x + (transientParent.width - width) / 2
            y = transientParent.y + (transientParent.height - height) / 2
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#252525"
        border.color: "#555"
        radius: 8

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Text {
                text: root.boxTitle
                color: "#ffffff"
                font.pixelSize: 16
                font.bold: true
            }
            Text {
                text: root.boxMessage
                color: "#e0e0e0"
                font.pixelSize: 14
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 40
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                spacing: 10
                TextButton {
                    text: root.buttonText
                    implicitWidth: 72
                    implicitHeight: 32
                    onClicked: {
                        root.confirmed()
                        root.close()
                    }
                }
            }
        }
    }
}
