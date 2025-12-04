import QtQuick
import QtQuick.Controls
//import QtQuick.Controls.Material
import "components"

Window {
    //visible: true
    width: 500
    height: 600
    flags: Qt.FramelessWindowHint
    property int baseFontSize: 20

    Rectangle {
        anchors.fill: parent
        color: "#1c1c1c"

        // 标题
        Text {
            text: "备份与恢复"
            font.pixelSize: 24
            color: "white"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 20
        }

        // 选项按钮组
        Row {
            spacing: 20
            anchors.top: parent.top
            anchors.topMargin: 70
            anchors.horizontalCenter: parent.horizontalCenter

            MMRadioButton {
              text: "备份"
                checked: true
            }
            MMRadioButton { text: "恢复"}
            MMRadioButton { text: "导入用"}
        }

        // 复选框区域
        Column {
            anchors.top: parent.top
            anchors.topMargin: 130
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10

            Rectangle {
                width: 400; height: 180
                color: "#2c2c2c"
                radius: 5
                border.color: "#555555"

                Column {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 5

                    MMCheckBox { text: "客户信息"; checked: true }
                    MMCheckBox { text: "产品与报告信息"; checked: true }
                    MMCheckBox { text: "高级备份" }
                }
            }
        }

        // 文件路径选择
        Row {
            anchors.top: parent.top
            anchors.topMargin: 330
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10

            TextField {
                id: pathField
                width: 280
                y: 5
                placeholderText: "请选择文件保存路径"
                font.pixelSize: baseFontSize
            }

            TextButton { text: "浏览" }
        }

        // 底部按钮
        Row {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 50

            TextButton { text: "开始"; width: 80
            }
            TextButton { text: "返回"; width: 80
                onClicked: close()
            }
        }
    }
}
