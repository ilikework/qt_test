import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic as Basic
import QtQuick.Layouts
import "."

/// 手动输入水分值（0–99），对标 TC30 frmMoisture 无水分笔模式
Item {
    id: root
    visible: false
    z: 2000
    anchors.fill: parent

    readonly property int _i18nRev: appTranslator ? appTranslator.revision : 0
    property int moistureValue: 50

    signal accepted(int value)
    signal cancelled()

    function open(initialValue) {
        var v = (typeof initialValue === "number") ? initialValue : 50
        moistureValue = Math.max(0, Math.min(99, Math.round(v)))
        valueField.text = String(moistureValue)
        visible = true
        valueField.forceActiveFocus()
        valueField.selectAll()
    }

    function close() { visible = false }

    function commit() {
        var n = parseInt(valueField.text, 10)
        if (isNaN(n))
            n = moistureValue
        n = Math.max(0, Math.min(99, n))
        moistureValue = n
        close()
        accepted(n)
    }

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
        width: Math.min(parent.width - 48, 380)
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
                text: { var _ = root._i18nRev; return appTranslator.translateText("水分测量") }
                color: "#ffffff"
                font.pixelSize: 16
                font.bold: true
                Layout.fillWidth: true
            }
            Text {
                text: { var _ = root._i18nRev; return appTranslator.translateText("请输入水分值（0–99）") }
                color: "#e0e0e0"
                font.pixelSize: 14
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                spacing: 12

                TextButton {
                    text: "−"
                    implicitWidth: 48
                    preferredHeight: 40
                    preferredFontPixelSize: 22
                    preferredRadius: 8
                    onClicked: {
                        moistureValue = Math.max(0, moistureValue - 1)
                        valueField.text = String(moistureValue)
                    }
                }

                Basic.TextField {
                    id: valueField
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 40
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: 22
                    font.bold: true
                    color: "#ffffff"
                    inputMethodHints: Qt.ImhDigitsOnly
                    validator: IntValidator { bottom: 0; top: 99 }
                    background: Rectangle {
                        radius: 6
                        color: "#1a1a1a"
                        border.color: valueField.activeFocus ? "#7cc0ff" : "#555"
                    }
                    onEditingFinished: {
                        var n = parseInt(text, 10)
                        if (isNaN(n))
                            n = root.moistureValue
                        n = Math.max(0, Math.min(99, n))
                        root.moistureValue = n
                        text = String(n)
                    }
                    Keys.onReturnPressed: root.commit()
                    Keys.onEnterPressed: root.commit()
                }

                TextButton {
                    text: "+"
                    implicitWidth: 48
                    preferredHeight: 40
                    preferredFontPixelSize: 22
                    preferredRadius: 8
                    onClicked: {
                        moistureValue = Math.min(99, moistureValue + 1)
                        valueField.text = String(moistureValue)
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                spacing: 12

                TextButton {
                    text: { var _ = root._i18nRev; return appTranslator.translateText("取消") }
                    implicitWidth: 100
                    preferredHeight: 36
                    preferredRadius: 8
                    onClicked: {
                        root.close()
                        root.cancelled()
                    }
                }
                TextButton {
                    text: { var _ = root._i18nRev; return appTranslator.translateText("确定") }
                    implicitWidth: 100
                    preferredHeight: 36
                    preferredRadius: 8
                    onClicked: root.commit()
                }
            }
        }
    }
}
