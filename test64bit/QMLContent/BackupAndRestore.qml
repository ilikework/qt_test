import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Controls.Basic
import QtCore
import "components"

Window { // 给主窗口添加一个ID
    id: backupRestoreWindow
    //visible: true
    width: 500
    height: 600
    flags: Qt.FramelessWindowHint
    property int baseFontSize: 20
    // Optional: set from parent (e.g. C++ backupManager). When null, Connections is no-op.
    //property var backupManager: null

    /** 每次进入本窗口时调用，清空上次的路径、结果、进度等 */
    function resetState() {
        pathField.text = ""
        statusText.text = ""
        progressBar.value = 0
        backupRadio.checked = true
        // 不设置 saveFileDialog.currentFile，设为空会触发 QML 警告 "Cannot set as a selected file because it doesn't exist"
    }

    onVisibleChanged: if (visible) resetState()

    Connections {
        target: backupManager
        enabled: backupManager !== null

        function onFinished(success, message) {
            // 弹出提示框或 console.log
            if (success) {
                progressBar.value = 1.0
            }
            statusText.text = message;

            messageDialog.boxMessage = message;
            messageDialog.open();
        }

        function onBackupProgress(progress) {
            // 更新 UI 上的进度条
            progressBar.value = progress
            statusText.text = "处理中: " + Math.floor(progress * 100) + "%"
        }

        function onRestoreProgress(progress) {
            progressBar.value = progress
            if (backupManager && backupManager.indeterminatePhase) return
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
        function onIndeterminatePhaseChanged() {
            if (backupManager && backupManager.indeterminatePhase)
                statusText.text = "正在压缩/解压，请稍候…"
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
                        saveFileDialog.openWithDefaultName()
                    } else {
                        saveFileDialog.fileMode = FileDialog.OpenFile
                        saveFileDialog.title = "选择备份文件进行恢复"
                        saveFileDialog.openForRestore()  // 恢复时对话框内不预填文件名
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

            // 有进度、有状态文字、或处于“无法取得进度”的压缩/解压阶段时显示
            visible: progressBar.value > 0 || statusText.text !== "" || (backupManager && backupManager.indeterminatePhase)

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
                // 压缩/解压时 C++ 每秒推进一点，这里只按数值显示
                indeterminate: false

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

        // 备份时：在 FileDialog 内给出默认文件名（文档目录 + MMFaceBackup_yyyyMMdd.zip），不预填页面上的 pathField
        function openWithDefaultName() {
            var docUrl = StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
            var date = new Date()
            var dateStr = date.getFullYear() + ("0" + (date.getMonth() + 1)).slice(-2) + ("0" + date.getDate()).slice(-2)
            var defaultName = "MMFaceBackup_" + dateStr + ".zip"
            currentFolder = docUrl
            currentFile = docUrl + "/" + defaultName
            open()
        }
        // 恢复时：只打开到文档目录，对话框内的文件名编辑框不预填（与备份不同）
        function openForRestore() {
            currentFolder = StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
            currentFile = currentFolder  // 选中的是目录，文件名编辑框为空
            open()
        }
        onAccepted: {
            pathField.text = selectedFile
        }
    }

    MessageBox {
        id: messageDialog
        transientParent: backupRestoreWindow // 消息框的父窗口设置为BackupAndRestore.qml的主窗口
        boxTitle: "操作完成"
        onConfirmed: {
            // 用户点击“确定”后关闭窗口
            backupRestoreWindow.close() // 调用主窗口的close()方法
        }
    }
}
