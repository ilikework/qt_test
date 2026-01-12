import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "components"

Item   {
    id: cameraRoot
    width: 1920
    height: 1080
    anchors.centerIn: parent
    property string customerID : ""

    signal requestShowMain()   // 请求返回主画面

    Connections {
        target: camClient
        function onLog(msg) { console.log("[client]", msg) }
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
          camClient.setCustomerID(customerID)
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
                source: camClient.previewOn
                        ? ("image://camera/frame?" + camClient.frameToken)
                        : ""                  // stop 时置空
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
                            color: "#ccc"

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
                                onClicked:
                                {
                                    console.log(modelData.text, " clicked")
                                    switch (modelData.key)
                                    {
                                       case "save":
                                           camClient.save()
                                           cameraRoot.requestShowMain()
                                           break
                                       case "cancel":
                                           camClient.cancel()
                                           cameraRoot.requestShowMain()
                                           break

                                       case "preview":
                                           //cameraRoot.startPreview()
                                            console.log("clicked preview, camClient=", camClient)
                                           camClient.startPreview()
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
