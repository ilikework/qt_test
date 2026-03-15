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

    Connections {
        target: camClient
    }

    // 进入拍摄页时：若已连接相机则直接进入预录（开预览）；未连接则显示“请连接相机”，用户连上后点「预览」即可
    onIsCameraViewActiveChanged: {
        if (isCameraViewActive && camClient.connected)
            camClient.startPreview()
        if (!isCameraViewActive)
            statusMessage = ""
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
                        text: "请连接相机后，点击下方「预览」按钮打开预览"
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
        // 右侧控制栏（固定宽度）
        // ============================
        Rectangle {
            id: controls
            color: "#181818"
            Layout.preferredWidth: 640
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                // =====================
                // 参数区域（两列布局）
                // =====================
                GridLayout {
                    id: paramGrid
                    //flow: GridLayout.TopToBottom
                    columns: 2
                    rowSpacing: 12
                    columnSpacing: 50
                    Layout.fillWidth: true
                    Layout.leftMargin:25
                    Repeater {
                        model: settings
                        ColumnLayout {
                            //anchors.margins: 10
                            spacing: 4
                            RowLayout {
                                Label { text: modelData.text;color: "white"; font.bold: true ; Layout.preferredWidth: 80}
                                spacing: 6
                                TextButton { text: "-"; Layout.preferredWidth: 24; Layout.preferredHeight: 24; onClicked: stepSetting(index, -1) }
                                Label {
                                    text: modelData.value
                                    color: "white"
                                    Layout.alignment: Qt.AlignCenter
                                    Layout.preferredWidth: 100
                                    horizontalAlignment: Text.AlignHCenter
                                    font.bold: true
                                }
                                TextButton { text: "+"; Layout.preferredWidth: 24 ; Layout.preferredHeight: 24; onClicked: stepSetting(index, 1)}
                            }
                        }
                    }
                }

                // ============================
                // 图片预览区域 - 两行 4 列
                // ============================
                Flickable {
                    id: previewArea
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    contentHeight: flowContent.height
                    clip: true

                    Column {
                        id: flowContent
                        spacing: 10
                        width: previewArea.width

                        Flow {
                            width: parent.width
                            spacing: 10

                            Repeater {
                                model: camClient.left_pics
                                delegate: Rectangle {
                                    width: parent.width / 4 - 10
                                    height: width * 4/3
                                    color: "white"

                                    Image {
                                        anchors.fill: parent
                                        anchors.margins: 2
                                        fillMode: Image.PreserveAspectFit
                                        cache: false
                                        source: modelData   // photoX
                                    }
                                }
                            }
                        }

                        Flow {
                            width: parent.width
                            spacing: 10

                            Repeater {
                                model: camClient.right_pics
                                delegate: Rectangle {
                                    width: parent.width / 4 - 10
                                    height: width * 4/3
                                    color: "white"

                                    Image {
                                        anchors.fill: parent
                                        anchors.margins: 2
                                        fillMode: Image.PreserveAspectFit
                                        cache: false
                                        source: modelData   // photoX
                                    }
                                }
                            }
                        }
                    }
                }

                // ============================
                // 底部金属按钮 4 枚
                // ============================
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 30
                    Layout.alignment: Qt.AlignHCenter

                    Repeater {
                        model: [
                            { key: "preview", text: "预览" },
                            { key: "shoot",   text: "拍摄" },
                            { key: "save",    text: "保存" },
                            { key: "cancel",  text: "取消" }
                        ]
                        delegate: Rectangle {
                            width: 120
                            height: 120
                            radius: 60
                            color: modelData.key === "preview" && !camClient.connected ? "#666" : "#ccc"
                            opacity: modelData.key === "preview" && !camClient.connected ? 0.8 : 1

                            Text {
                                anchors.centerIn: parent
                                text: modelData.text
                                color: "black"
                                font.pixelSize: 28
                                font.bold: true
                            }
                            MouseArea {
                                cursorShape: Qt.PointingHandCursor
                                anchors.fill: parent
                                onClicked: {
                                    switch (modelData.key) {
                                    case "save":
                                        camClient.save()
                                        cameraRoot.photoSaved()
                                        cameraRoot.requestShowMain()
                                        break
                                    case "cancel":
                                        camClient.cancel()
                                        cameraRoot.requestShowMain()
                                        break
                                    case "preview":
                                        if (camClient.connected) {
                                            cameraRoot.statusMessage = ""
                                            camClient.startPreview()
                                        } else {
                                            cameraRoot.showStatusMessage("请先连接相机")
                                        }
                                        break
                                    case "shoot":
                                        camClient.capture()
                                        break
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
