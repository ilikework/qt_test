import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls.Material
import "components"

Dialog {
    //visible: true
    id:dlg
    width: 800
    height: 450
    modal: true

    Component.onCompleted: {
        // dlg.parent 可能是 ApplicationWindow
        dlg.anchors.centerIn = dlg.parent
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#18181b"

            ScrollView {
                anchors.fill: parent

                Rectangle {
                    width: 600
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "#222226"
                    radius: 16
                    anchors.topMargin: 48
                    anchors.top: parent.top
                    //padding: 32

                    ColumnLayout {
                        width: parent.width
                        spacing: 24

                        // 标题
                        Text {
                            text: "客户信息填写"
                            font.pixelSize: 22
                            font.bold: true
                            color: "#ffffff"
                            Layout.alignment: Qt.AlignHCenter
                            Rectangle {
                                anchors.bottom: parent.bottom
                                anchors.left: parent.left
                                width: parent.width
                                height: 1
                                color: "#444444"
                            }
                        }

                        RowLayout {
                            spacing: 32
                            //anchors.horizontalCenter: parent.horizontalCenter

                            // 左侧照片
                            ColumnLayout {
                                spacing: 12
                                width: 150

                                Rectangle {
                                    width: 150
                                    height: 200
                                    radius: 8
                                    color: "#ffffff"
                                    border.color: "#444444"
                                    Image {
                                        id: userPhoto
                                        anchors.fill: parent
                                        anchors.margins: 0
                                        fillMode: Image.PreserveAspectCrop
                                        source: "images/user_icon.svg"
                                    }
                                }

                                FileDialog {
                                    id: fileDialog
                                    title: "选择照片"
                                    nameFilters: ["Images (*.png *.jpg *.jpeg *.bmp)"]
                                    onAccepted: {
                                        userPhoto.source = fileDialog.fileUrl
                                    }
                                }
                                RowLayout {
                                    spacing: 28
                                    TextButton {
                                        text: "选择照片"
                                        onClicked: fileDialog.open()
                                    }

                                    TextButton {
                                        text: "拍摄"
                                        onClicked: {
                                            console.log("这里可集成摄像头拍摄功能")
                                        }
                                    }
                                }
                            }

                            // 右侧表单
                            ColumnLayout {
                                spacing: 16
                                Layout.fillWidth: true

                                // 客户编号
                                RowLayout {
                                    spacing: 8
                                    Label { text: "客户编号"; color: "#ffb300"; font.bold: true; width: 90 }
                                    TextField { text: "自动生成"; readOnly: true; Layout.fillWidth: true; background: Rectangle { color: "#fbeeee"; radius: 6; border.color: "#444444"} }
                                }

                                // 客户姓名
                                RowLayout {
                                    spacing: 8
                                    Label { text: "客户姓名"; color: "#ffb300"; font.bold: true; width: 90 }
                                    TextField { placeholderText: "请输入客户姓名"; Layout.fillWidth: true; background: Rectangle { color: "#fbeeee"; radius: 6; border.color: "#444444"} }
                                }

                                // 登记日
                                RowLayout {
                                    spacing: 8
                                    Label { text: "登记日"; color: "#ffb300"; font.bold: true; width: 90 }
                                    DatePicker {
                                    }

                                }

                                // 性别 + 生日
                                RowLayout {
                                    spacing: 8

                                    Label { text: "性别"; color: "#ffb300"; font.bold: true; width: 90 }
                                    ComboBox { model: ["请选择", "男", "女"]; Layout.fillWidth: true }

                                    Label { text: "生日"; color: "#ffb300"; font.bold: true; width: 70; horizontalAlignment: Text.AlignRight }
                                    DatePicker {
                                    }
                                }

                                // Email
                                RowLayout {
                                    spacing: 8
                                    Label { text: "Email"; color: "#ffb300"; font.bold: true; width: 90 }
                                    TextField { placeholderText: "请输入Email"; Layout.fillWidth: true; background: Rectangle { color: "#fbeeee"; radius: 6; border.color: "#444444"} }
                                }

                                // 电话
                                RowLayout {
                                    spacing: 8
                                    Label { text: "电话"; color: "#ffb300"; font.bold: true; width: 90 }
                                    TextField { placeholderText: "请输入电话"; Layout.fillWidth: true; background: Rectangle { color: "#fbeeee"; radius: 6; border.color: "#444444"} }
                                }

                                // 按钮
                                RowLayout {
                                    spacing: 28
                                    Layout.alignment: Qt.AlignRight
                                    TextButton { text: "保存"; width: 200; height: 50
                                        onClicked: {
                                            dlg.accepted()   // 触发外部 onAccepted
                                            dlg.close()
                                        }
                                    }
                                    TextButton { text: "取消"; width: 200; height: 50;

                                        onClicked: {
                                            dlg.close()
                                        }
                                    }
                                }

                            } // ColumnLayout 右侧
                        } // RowLayout
                    } // ColumnLayout
                } // Rectangle container
            } // ScrollView
        } // Rectangle background
    }
}
