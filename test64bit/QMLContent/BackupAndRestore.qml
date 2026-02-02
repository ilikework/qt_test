import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Controls.Basic
import "components"

Window {
    //visible: true
    width: 500
    height: 600
    flags: Qt.FramelessWindowHint
    property int baseFontSize: 20

    Connections {
        target: backupManager

        function onFinished(success, message) {
            // 弹出提示框或 console.log
            console.log(message)
            statusText.text = message
            if (success) {
                progressBar.visualFocus = true
            }
        }

        function onBackupProgress(progress) {
            // 更新 UI 上的进度条
            progressBar.value = progress
            statusText.text = "处理中: " + Math.floor(progress * 100) + "%"
        }

        function onRestoreProgress(progress) {
            progressBar.value = progress

            if (progress < 0.4) {
                statusText.text = "正在校验备份包..."
            } else if (progress >= 0.4 && progress < 0.6) {
                statusText.text = "正在安全暂存当前数据..." // 此时 DB 已关闭
            } else if (progress >= 0.6 && progress < 0.9) {
                statusText.text = "正在部署新文件..."
            } else {
                statusText.text = "正在重新连接数据库..."
            }
        }
    }

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

        // 定义按钮组，方便获取选中状态
        ButtonGroup {
            id: modeGroup
        }
        // 选项按钮组
            Row {
                id: modeRow
                spacing: 20
                anchors.top: parent.top
                anchors.topMargin: 70
                anchors.horizontalCenter: parent.horizontalCenter

                MMRadioButton {
                    id: backupRadio
                    text: "备份"
                    checked: true
                    ButtonGroup.group: modeGroup // 绑定到组
                }
                MMRadioButton {
                    id: restoreRadio
                    text: "恢复"
                    ButtonGroup.group: modeGroup
                }
                MMRadioButton {
                    id: importRadio
                    text: "导入用"
                    ButtonGroup.group: modeGroup
                }
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

            TextButton { text: "浏览"
                onClicked: {
                    if (backupRadio.checked) {
                        saveFileDialog.fileMode = FileDialog.SaveFile
                        saveFileDialog.title = "选择备份保存位置"
                        saveFileDialog.openWithDefaultName() // 调用之前写的带日期的函数
                    } else {
                        saveFileDialog.fileMode = FileDialog.OpenFile
                        saveFileDialog.title = "选择备份文件进行恢复"
                        saveFileDialog.currentFile = "" // 清空默认名
                        saveFileDialog.open()
                    }
                }
            }
        }

        // 2. 插入进度显示区域 (放在中间，top 设为约 430)
        Column {
            id: progressArea
            width: 400
            anchors.top: parent.top
            anchors.topMargin: 430 // 位于路径选择下方 100 像素
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 8

            // 只有在有进度或有消息时才显示，初始隐藏
            visible: progressBar.value > 0 || statusText.text !== ""

            Text {
                id: statusText
                text: ""
                color: "#aaaaaa"
                font.pixelSize: 14
                anchors.horizontalCenter: parent.horizontalCenter
            }

            ProgressBar {
                id: progressBar
                width: parent.width
                value: 0.0

                // 这里的 background 和 contentItem 保持你之前的自定义样式即可
                background: Rectangle {
                    implicitHeight: 6
                    color: "#444444"
                    radius: 3
                }
                contentItem: Item {
                    Rectangle {
                        width: progressBar.visualPosition * parent.width
                        height: 6
                        radius: 3
                        color: "#ffb300"
                    }
                }
            }
        }

        // 底部按钮
        Row {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 50

            TextButton { text: "开始"; width: 80
                onClicked: {
                    if (pathField.text === "") {
                        console.log("请先选择路径")
                        return
                    }

                    // 重置进度条
                    progressBar.value = 0

                    if (backupRadio.checked) {
                        // 执行备份逻辑
                        statusText.text = "准备备份..."
                        // 第二个参数控制是否包含照片，可以绑定到你的 MMCheckBox 状态
                        backupManager.startBackup(pathField.text, true)
                    }
                    else if (restoreRadio.checked) {
                        // 执行恢复逻辑
                        statusText.text = "准备恢复..."
                        backupManager.startRestore(pathField.text)
                    }
                    else if (importRadio.checked) {
                        statusText.text = "导入功能开发中..."
                    }
                }
            }
            TextButton { text: "返回"; width: 80
                onClicked: close()
            }
        }


    }

    // 保存文件对话框
    FileDialog {
        id: saveFileDialog
        title: "选择备份保存位置"
        fileMode: FileDialog.SaveFile
        nameFilters: ["Backup files (*.zip)"]

        // 动态设置默认文件名
        function openWithDefaultName() {
            let date = new Date()
            let year = date.getFullYear()
            let month = ("0" + (date.getMonth() + 1)).slice(-2)
            let day = ("0" + date.getDate()).slice(-2)
            let dateStr = year + month + day

            // 设置默认选中的文件（包含路径和文件名）
            // 注意：建议给一个默认目录，或者留空让系统决定
            currentFile = "file:///MMFaceBackup_" + dateStr + ".zip"
            open()
        }
        onAccepted: {
            pathField.text = selectedFile
        }
    }
}
