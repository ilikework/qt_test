import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "components"

Item   {
    id: cameraRoot
    width: 1920
    height: 1080
    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    property string customerID : ""
    property bool isCameraViewActive: false
    property string statusMessage: ""

    function showStatusMessage(msg) {
        statusMessage = msg
        // 不再定时清除，一直显示直到离开拍摄页
    }

    signal requestShowMain()   // 请求返回主画面
    signal photoSaved()      // 照片保存成功信号

    // 进入拍摄页时：若已连接相机则直接进入预录（开预览）；未连接则显示“请连接相机”，用户连上后点「预览」即可
    onIsCameraViewActiveChanged: {
        if (isCameraViewActive && camClient.connected)
            camClient.startPreview()
        if (!isCameraViewActive) {
            statusMessage = ""
            settingsPanelOpen = false
        }
    }
    Connections {
        target: camClient
        function onConnectedChanged() {
            if (isCameraViewActive && camClient.connected)
                camClient.startPreview()
        }
        function onPreviewOnChanged() {
            if (camClient.previewOn)
                cameraRoot.statusMessage = ""
        }
        // open 失败不再设 statusMessage，让提示层统一显示「请点击预览」，不抢「请连接相机」的展示
        function onPreviewOpenFailed(msg) {
            if (msg)
                cameraRoot.showStatusMessage(msg)
            /// 预览未建立时 previewOn 不变，显式恢复按钮未选（配合 autoToggle: false）
            previewBtn.checked = false
        }
        function onOpenFinished(ok, msg) {
            if (!ok && msg)
                cameraRoot.showStatusMessage(msg)
            if (!ok)
                previewBtn.checked = false
        }
        function onCaptureFinished(ok) {
            cameraRoot.captureBusy = false
        }
    }


    property var settings: camClient.settings
    property var isos: camClient.isos
    property var exposuretimes: camClient.exposuretimes
    property var apertures: camClient.apertures
    property var wbs: camClient.wbs
    property var sizes: camClient.sizes
    property var qualities: camClient.qualities


    function optionsForItem(itemKey) {
        if (itemKey.indexOf("iso") !== -1) return isos
        if (itemKey.indexOf("exposuretime") !== -1) return exposuretimes
        if (itemKey.indexOf("aperture") !== -1) return apertures
        if (itemKey.indexOf("wb") !== -1) return wbs
        if (itemKey === "ImageSize") return sizes
        if (itemKey === "ImageQuality") return qualities
        return []
    }

    /// 连拍进行中（用于「拍摄」按钮 checked，结束后恢复「预览」checked）
    property bool captureBusy: false
    /// 设定面板展开（拍摄中强制关闭）
    property bool settingsPanelOpen: false

    /// 设定区尽量加长：按钮区 + ColumnLayout 缝、底栏缩略带再压一点，把垂直空间让给设置滚动区
    readonly property int settingsAreaHeight: {
        if (!settingsPanelOpen)
            return 0
        // 5 个 32px 按钮 + 列内 4 间距 + 与 ScrollView/List 的布局余量（已略收紧）
        var btnBlock = 5 * 32 + 4 * 4 + 18
        var listReserve = 26
        var margin = 6
        var raw = controls.height - btnBlock - listReserve - margin
        return Math.max(160, Math.floor(raw))
    }

    onCaptureBusyChanged: {
        if (captureBusy)
            settingsPanelOpen = false
    }

    function stepSetting(rowIndex, step) {
        if (!settings || settings.length === 0) return

        // 取要改的项（拷贝）
        var item = Object.assign({}, settings[rowIndex])

        // 用 key 去拿候选项（不要用 item.text！）
        // item.text 是 "RGB ISO" 这种标题
        var opts = optionsForItem(item.key)
        if (!opts || opts.length === 0) return

        var cur = item.curIndex
        if (cur < 0) cur = 0

        var next = cur + step
        if (next < 0) next = 0
        if (next >= opts.length) next = opts.length - 1

        item.curIndex = next
        item.value = opts[next].text      // ✅ 用于 UI 显示
        item.rawValue = opts[next].value  // ✅ 用于后续设相机/写DB（建议）

        // 用新数组替换 settings（最稳触发刷新）
        var arr = settings.slice()
        arr[rowIndex] = item
        settings = arr

        camClient.onSettingChanged(rowIndex, item.key,item.value, item.rawValue)
    }

    // ✅ 初始化完成时传给 C++
    Component.onCompleted: {
      if (customerID !== "")
      {
          camClient.init(customerID)
      }
    }

    Rectangle {
        anchors.fill: parent
        color: "#111"
    }

    RowLayout {
        anchors.fill: parent

        // ============================
        // 左侧相机预览区
        // ============================
        Rectangle {
            id: cameraView
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: "black"
            clip: true

            Image {
                id: cameraPreview
                anchors.fill: parent
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
                visible: camClient.connected && camClient.previewOn
                source: camClient.previewOn
                        ? ("image://camera/frame?" + camClient.frameToken)
                        : ""
            }

            // 提示层：只要没有预览就显示，避免被 open 失败的红字抢掉
            Rectangle {
                id: messageOverlay
                anchors.fill: parent
                color: cameraRoot.statusMessage !== "" ? "#1a0a0a" : "#1a1a1a"
                visible: !camClient.previewOn

                Column {
                    anchors.centerIn: parent
                    spacing: 16

                    // 未连上相机程序（socket 未连接）：程序会自动启动 exe 再连，理论上应能连上，若不能则提示联系客服
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "未能连接相机程序，请联系客服。"
                        color: "#e04040"
                        font.pixelSize: 32
                        font.bold: true
                        visible: !camClient.connected
                    }

                    // 已连上程序但未开预览：请连接相机硬件后点预览，红色
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "正在连接相机打开预览"
                        color: "#e04040"
                        font.pixelSize: 28
                        font.bold: true
                        visible: camClient.connected && !camClient.previewOn && cameraRoot.statusMessage === ""
                    }

                    // 仅 startpreview 失败时显示红字（open 失败不再设 statusMessage）
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: cameraRoot.statusMessage
                        color: "#e04040"
                        font.pixelSize: 32
                        font.bold: true
                        visible: cameraRoot.statusMessage !== ""
                    }
                }
            }
        }

        // ============================
        // 右侧栏：与 MM3DViewer 同宽、同风格（162）
        // ============================
        Rectangle {
            id: controls
            color: "#1e1e1e"
            radius: 8
            border.color: "#333"
            Layout.preferredWidth: 162
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 4
                spacing: 4

                // ----- 上部：功能键竖排（预览/拍摄为 CheckButton；保存/取消为普通按钮） -----
                Column {
                    Layout.fillWidth: true
                    spacing: 4

                    Item {
                        width: parent.width
                        height: 32
                        opacity: camClient.connected ? 1 : 0.45
                        CheckButton {
                            id: previewBtn
                            anchors.horizontalCenter: parent.horizontalCenter
                            buttonHeight: 32
                            fontPixelSize: 16
                            cornerRadius: 6
                            borderW: 1
                            autoToggle: false
                            text: "预览"
                            onClicked: {
                                if (camClient.previewOn)
                                    return
                                if (camClient.connected) {
                                    cameraRoot.statusMessage = ""
                                    camClient.startPreview()
                                } else {
                                    cameraRoot.showStatusMessage("请先连接相机")
                                }
                            }
                        }
                        Binding {
                            target: previewBtn
                            property: "checked"
                            value: camClient.previewOn && !cameraRoot.captureBusy
                        }
                    }

                    Item {
                        width: parent.width
                        height: 32
                        opacity: camClient.previewOn ? 1 : 0.45
                        CheckButton {
                            id: shootBtn
                            anchors.horizontalCenter: parent.horizontalCenter
                            buttonHeight: 32
                            fontPixelSize: 16
                            cornerRadius: 6
                            borderW: 1
                            text: "拍摄"
                            onClicked: {
                                if (cameraRoot.captureBusy || !camClient.previewOn || !camClient.connected)
                                    return
                                cameraRoot.captureBusy = true
                                camClient.capture()
                            }
                        }
                        Binding {
                            target: shootBtn
                            property: "checked"
                            value: cameraRoot.captureBusy
                        }
                    }

                    Item {
                        width: parent.width
                        height: 32
                        TextButton {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: parent.parent.width * 0.92
                            preferredHeight: 32
                            preferredFontPixelSize: 16
                            preferredRadius: 10
                            text: "保存"
                            onClicked: {
                                camClient.save()
                                cameraRoot.photoSaved()
                                cameraRoot.requestShowMain()
                            }
                        }
                    }

                    Item {
                        width: parent.width
                        height: 32
                        TextButton {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: parent.parent.width * 0.92
                            preferredHeight: 32
                            preferredFontPixelSize: 16
                            preferredRadius: 10
                            text: "取消"
                            onClicked: {
                                camClient.cancel()
                                cameraRoot.requestShowMain()
                            }
                        }
                    }

                    Item {
                        width: parent.width
                        height: 32
                        opacity: cameraRoot.captureBusy ? 0.45 : 1
                        CheckButton {
                            id: settingsBtn
                            anchors.horizontalCenter: parent.horizontalCenter
                            buttonHeight: 32
                            fontPixelSize: 16
                            cornerRadius: 6
                            borderW: 1
                            text: "设定"
                            onClicked: {
                                if (cameraRoot.captureBusy)
                                    return
                                cameraRoot.settingsPanelOpen = !cameraRoot.settingsPanelOpen
                            }
                        }
                        Binding {
                            target: settingsBtn
                            property: "checked"
                            value: cameraRoot.settingsPanelOpen
                        }
                    }
                }

                // ----- 设定：窄栏单列参数（高度随侧栏变化，超出部分垂直滚动 + 滚动条）
                ScrollView {
                    id: settingsScroll
                    visible: cameraRoot.settingsPanelOpen
                    Layout.fillWidth: true
                    Layout.preferredHeight: cameraRoot.settingsAreaHeight
                    Layout.maximumHeight: cameraRoot.settingsAreaHeight
                    clip: true
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    ScrollBar.vertical.policy: ScrollBar.AsNeeded

                    Column {
                        id: settingsParamCol
                        width: Math.max(1, settingsScroll.availableWidth)
                        spacing: 7

                        Repeater {
                            model: settings
                            Column {
                                width: settingsParamCol.width
                                spacing: 4
                                Label {
                                    width: parent.width
                                    text: modelData.text
                                    color: "#ddd"
                                    font.pixelSize: 11
                                    font.bold: true
                                    wrapMode: Text.WordWrap
                                }
                                RowLayout {
                                    width: parent.width
                                    spacing: 4
                                    TextButton {
                                        text: "-"
                                        Layout.preferredWidth: 28
                                        Layout.preferredHeight: 28
                                        onClicked: stepSetting(index, -1)
                                    }
                                    Label {
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 1
                                        width: parent.width
                                        text: modelData.value
                                        color: "white"
                                        horizontalAlignment: Text.AlignHCenter
                                        font.pixelSize: 11
                                        font.bold: true
                                        wrapMode: Text.WordWrap
                                    }
                                    TextButton {
                                        text: "+"
                                        Layout.preferredWidth: 28
                                        Layout.preferredHeight: 28
                                        onClicked: stepSetting(index, 1)
                                    }
                                }
                            }
                        }
                    }
                }

                // ----- 拍摄结果：与 3D 侧栏类似，每行左+右两张 -----
                ListView {
                    id: shotPairList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: cameraRoot.settingsPanelOpen ? 28 : 72
                    clip: true
                    spacing: 2
                    model: camClient.left_pics

                    delegate: Rectangle {
                        width: shotPairList.width
                        height: 100
                        radius: 6
                        color: "#2a2a2e"
                        border.color: "#444"
                        border.width: 1

                        Row {
                            anchors.centerIn: parent
                            spacing: 2
                            Rectangle {
                                width: 67
                                height: 89
                                radius: 2
                                color: "transparent"
                                border.color: "#555"
                                border.width: 1
                                Image {
                                    anchors.centerIn: parent
                                    width: 65
                                    height: 87
                                    fillMode: Image.PreserveAspectFit
                                    sourceSize.width: 65
                                    sourceSize.height: 87
                                    asynchronous: true
                                    cache: false
                                    source: modelData
                                }
                            }
                            Rectangle {
                                width: 67
                                height: 89
                                radius: 2
                                color: "transparent"
                                border.color: "#555"
                                border.width: 1
                                Image {
                                    anchors.centerIn: parent
                                    width: 65
                                    height: 87
                                    fillMode: Image.PreserveAspectFit
                                    sourceSize.width: 65
                                    sourceSize.height: 87
                                    asynchronous: true
                                    cache: false
                                    source: index < camClient.right_pics.length ? camClient.right_pics[index] : ""
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
