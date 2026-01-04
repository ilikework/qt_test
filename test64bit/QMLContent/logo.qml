import QtQuick
import QtQuick.Controls
import "components"

Item {
    id: root
    anchors.fill: parent   // 自适应父容器大小

    signal loadPage(string page, var params)

    BackupAndRestore{
        id : bkrs
    }
    Rectangle {
        anchors.fill: parent
        color: "#0b0b0b"
    }

    // 居中 Logo
    Image {
        id: logo
        source: "images/newMM_Logo.png"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        fillMode: Image.PreserveAspectFit
        width: parent.width * 0.6   // 可根据屏幕宽度调整大小
        smooth: true
    }

    // 下方按钮组
    Row  {
        id: buttonRow
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: logo.bottom
        anchors.topMargin: parent.height * 0.08   // 根据父高调整按钮间距
        spacing: 10

        LogoButton {
            width: 100
            height: 100
            id:userButton
            source: "./images/user_icon.svg"
            ToolTip.text: "用户管理"
            onClicked: {
                //pageLoader.source = "customerManager.qml"
                root.loadPage("customerManager.qml", {})
            }
        }
        LogoButton {
            width: 100
            height: 100
            id:bakcupButton
            source: "./images/backup_icon.svg"
            ToolTip.text: "资料备份"
            onClicked: {
                bkrs.modality = Qt.WindowModal
                bkrs.show()
            }
        }
        LogoButton {
            width: 100
            height: 100
            id:exitButton
            source: "./images/exit_icon.svg"
            ToolTip.text: "退出程序"
            onClicked: {
                Qt.quit()
            }
        }
    }


}
