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


    property var settings: [
        {text:"RGB ISO", value:"100"},
        {text:"RGB 快门", value:"1/60"},
        {text:"RGB 光圈", value:"f/2.8"},
        {text:"RGB 白平衡", value:"Auto"},
        {text:"UV ISO", value:"100"},
        {text:"UV 快门", value:"1/60"},
        {text:"UV 光圈", value:"f/2.8"},
        {text:"UV 白平衡", value:"Auto"},
        {text:"PL ISO", value:"100"},
        {text:"PL 快门", value:"1/60"},
        {text:"PL 光圈", value:"f/2.8"},
        {text:"PL 白平衡", value:"Auto"},
        {text:"NPL ISO", value:"100"},
        {text:"NPL 快门", value:"1/60"},
        {text:"NPL 光圈", value:"f/2.8"},
        {text:"NPL 白平衡", value:"Auto"},
        {text:"图片尺寸", value:"1920x1080"},
        {text:"图片质量", value:"高"}
    ]

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

            Image {
                id: cameraPreview
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
                source: ""  // base64 或文件路径
                sourceSize.width: 1280
                sourceSize.height: 960
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
                                TextButton { text: "-"; Layout.preferredWidth: 35 }
                                Label {
                                    text: modelData.value
                                    color: "white"
                                    Layout.alignment: Qt.AlignCenter
                                    Layout.preferredWidth: 100
                                    horizontalAlignment: Text.AlignHCenter
                                    font.bold: true
                                }
                                TextButton { text: "+"; Layout.preferredWidth: 35 }
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
                                model: 4
                                delegate: Rectangle {
                                    width: parent.width / 4 - 10
                                    height: width * 4/3
                                    color: "white"

                                    Image {
                                        anchors.fill: parent
                                        anchors.margins: 2
                                        fillMode: Image.PreserveAspectFit
                                        source: ""   // photoX
                                    }
                                }
                            }
                        }

                        Flow {
                            width: parent.width
                            spacing: 10

                            Repeater {
                                model: 4
                                delegate: Rectangle {
                                    width: parent.width / 4 - 10
                                    height: width * 4/3
                                    color: "white"

                                    Image {
                                        anchors.fill: parent
                                        anchors.margins: 2
                                        fillMode: Image.PreserveAspectFit
                                        source: ""   // photoX
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
                        model: ["预览", "拍摄", "保存", "取消"]

                        delegate: Rectangle {
                            width: 120
                            height: 120
                            radius: 60
                            color: "#ccc"

                            Text {
                                anchors.centerIn: parent
                                text: modelData
                                color: "black"
                                font.pixelSize: 28
                                font.bold: true
                            }

                            MouseArea {
                                cursorShape: Qt.PointingHandCursor
                                anchors.fill: parent
                                onClicked:
                                {
                                    console.log(modelData, " clicked")
                                    if(modelData==="保存" || modelData==="取消")
                                    {
                                        close()
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
