import QtQuick
import QtQuick.Controls
import "components"

Item {
    id: root
    anchors.fill: parent

    readonly property int _i18nRev: appTranslator ? appTranslator.revision : 0

    signal loadPage(string page, var params)

    BackupAndRestore {
        id: bkrs
    }
    PreRecordDialog {
        id: preRecordDlg
    }

    Rectangle {
        anchors.fill: parent
        color: "#0b0b0b"
    }

    LanguageBar {
        id: langBar
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 16
        z: 10
    }

    Image {
        id: logo
        source: "images/newMM_Logo.png"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        fillMode: Image.PreserveAspectFit
        width: (parent.width > 0 && !isNaN(parent.width)) ? parent.width * 0.6 : 480
        smooth: true
    }

    Row {
        id: buttonRow
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: logo.bottom
        anchors.topMargin: (parent.height > 0 && !isNaN(parent.height)) ? parent.height * 0.08 : 48
        spacing: 10

        LogoButton {
            width: 100
            height: 100
            id: userButton
            source: "qrc:/images/user_icon.svg"
            ToolTip.text: { var _ = root._i18nRev; return appTranslator.translateText("用户管理") }
            onClicked: root.loadPage("customerManager.qml", {})
        }
        LogoButton {
            width: 100
            height: 100
            id: bakcupButton
            source: "qrc:/images/backup_icon.svg"
            ToolTip.text: { var _ = root._i18nRev; return appTranslator.translateText("资料备份") }
            onClicked: {
                bkrs.modality = Qt.WindowModal
                bkrs.show()
            }
        }
        LogoButton {
            width: 100
            height: 100
            id: preRecordButton
            source: "qrc:/images/prerecord_icon.svg"
            ToolTip.text: { var _ = root._i18nRev; return appTranslator.translateText("预录") }
            onClicked: {
                preRecordDlg.modality = Qt.WindowModal
                preRecordDlg.show()
            }
        }
        LogoButton {
            width: 100
            height: 100
            id: exitButton
            source: "qrc:/images/exit_icon.svg"
            ToolTip.text: { var _ = root._i18nRev; return appTranslator.translateText("退出程序") }
            onClicked: Qt.quit()
        }
    }
}
