import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
//import Custom3D
import QtQuick3D 6.7
import QtQuick3D.Helpers
import QtQuick3D.AssetUtils
import com.magicmirror.components // Import the C++ registered QML types

import "components"


Item {
    id: customerDetail
    visible: true

    signal loadPage(string page, var params)
    property string customerID : ""
    property int curIndex:-1
    property var groups:[]
    // ListModel {
    //     id: mainphotoes
    // }
    // ListModel {
    //     id: subphotoes
    // }
    property var mainphotoes:  analyseModule.thumbphotoes
    property var subphotoes:[]
    property int currentGroupID: 0

    Component.onCompleted:
    {
        analyseModule.init(customerID);
        curIndex = 0;

        loadsubphotoes(0)


    }

    function loadsubphotoes(index)
    {
        currentGroupID = mainphotoes[index].GROUPID
        subphotoes = analyseModule.loadSub(mainphotoes[index].GROUPID)
        if(subphotoes.length>0)
        {
            // Let the list render first, then load the main editing area in the next frame
            Qt.callLater(() => {
                leftMain.source = subphotoes[0].photoL; // Assuming photoL is a URL or path
                leftMain.init(subphotoes[0].IXL, "_L");
                rightMain.source = subphotoes[0].photoR; // Assuming photoR is a URL or path
                rightMain.init(subphotoes[0].IXR, "_R");
            })
        }
    }


    /* ==== 上方缩略图栏 ==== */
    Rectangle
    {
        id: thumbBar
        width: parent.width
        height: 145
        color: "#222226"
        border.color: "#444"
        border.width: 1
        anchors.top: parent.top
        property int expandedIndex: -1   // ⭐ 当前展开的缩略图编号
        property int thumbCurrentPage: 1
        property int thumbPageSize: 0
        property int thumbTotalPages: thumbPageSize > 0 ? Math.max(1, Math.ceil(mainphotoes.length / thumbPageSize)) : 1

        RowLayout
        {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 8

            IconButton {
                Layout.preferredWidth:60
                Layout.preferredHeight:120
                source: "./images/left_icon.svg"
                enabled: thumbBar.thumbCurrentPage > 1
                opacity: enabled ? 1 : 0.4
                onClicked: {
                    if (thumbBar.thumbCurrentPage > 1)
                        thumbBar.thumbCurrentPage--
                }
            }
            // ⭐ 中间区域占满宽度
            Item {

                Layout.fillWidth: true     // 重点
                height: parent.height

                Row {
                    id: thumbRow
                    spacing: 8
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.right: parent.right

                    readonly property int delegateWidth: 188
                    property int rowWidth: delegateWidth + spacing

                    Repeater {
                        id: mainThumbList
                        model: mainphotoes.slice((thumbBar.thumbCurrentPage-1)*thumbBar.thumbPageSize, thumbBar.thumbCurrentPage*thumbBar.thumbPageSize)
                        delegate: Rectangle {
                            width: 188; height: 128; radius: 4
                            property int globalIndex: (thumbBar.thumbCurrentPage - 1) * thumbBar.thumbPageSize + index
                            color: globalIndex === curIndex ? "#2a2a2e" : "#333"
                            border.color: globalIndex === curIndex ? "#ffb300" : "#444"
                            border.width: 4

                            Row {
                                leftPadding: 4
                                topPadding: 4
                                //spacing: 8

                                Image {
                                    width: 90; height: 120
                                    fillMode: Image.PreserveAspectFit
                                    source: modelData.photoL
                                    sourceSize.width: 90
                                    sourceSize.height: 120
                                    asynchronous: true
                                }
                                Image {
                                    width: 90; height: 120
                                    fillMode: Image.PreserveAspectFit
                                    source: modelData.photoR
                                    sourceSize.width: 90
                                    sourceSize.height: 120
                                    asynchronous: true
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (curIndex !== globalIndex) {
                                        curIndex = globalIndex
                                        loadsubphotoes(curIndex)
                                        return
                                    }
                                    thumbBar.expandedIndex = (thumbBar.expandedIndex === globalIndex ? -1 : globalIndex)
                                }
                            }
                        }
                    }
                    onWidthChanged: {
                        thumbBar.thumbPageSize = Math.floor(thumbRow.width / thumbRow.rowWidth)
                    }
                }
            }
            IconButton {
                Layout.preferredWidth:60
                Layout.preferredHeight:120
                source: "./images/right_icon.svg"
                enabled: thumbBar.thumbCurrentPage < thumbBar.thumbTotalPages
                opacity: enabled ? 1 : 0.4
                onClicked: {
                    if (thumbBar.thumbCurrentPage < thumbBar.thumbTotalPages)
                        thumbBar.thumbCurrentPage++
                }
            }
        }
    }

    /* ==== 主内容布局区域 ==== */
    RowLayout {
        id: mainLayout
        anchors.top: thumbBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        /* ==== 左栏按钮区域 ==== */
        Rectangle {
            id: leftBar
            width: 240
            color: "#232325"
            Layout.fillHeight: true

            // 拍摄页时遮住左侧栏，只允许通过右侧 4 个按钮（预览/拍摄/保存/取消）操作，避免切页导致预览逻辑混乱
            Rectangle {
                anchors.fill: parent
                visible: viewStack.currentIndex === 2
                color: "#80000000"
                z: 1
                MouseArea {
                    anchors.fill: parent
                    onPressed: function(mouse) { mouse.accepted = true }
                    onReleased: function(mouse) { mouse.accepted = true }
                }
                Text {
                    anchors.centerIn: parent
                    text: "请使用右侧「取消」或「保存」\n退出拍摄"
                    color: "#aaa"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            ScrollView {
                width: parent.width
                height: parent.height
                contentWidth: parent.width
                clip: true

                Column {
                    id: buttonColumn
                    spacing: 8
                    width: parent.width

                    CheckButton
                    {
                        id:btnMain
                        text: "主画面"
                        checked: true
                        onClicked:
                        {
                            viewStack.currentIndex = 0
                            btn3D.checked = false
                            btn9face.checked = false
                            btnCamera.checked = false
                        }
                    }
                    CheckButton
                    {
                        id:btn3D
                        text: "3D人脸"
                        onClicked:
                        {
                            viewStack.currentIndex = 1
                            btnMain.checked = false
                            btn9face.checked = false
                            btnCamera.checked = false
                        }
                    }
                    CheckButton
                    {
                        id:btn9face
                        text: "九画面"
                        onClicked:
                        {
                            viewStack.currentIndex = 3
                            btnMain.checked = false
                            btn3D.checked = false
                            btnCamera.checked = false
                        }
                    }

                    CheckButton {
                        checked: false

                        text: "系统工具"
                    }
                    CheckButton
                    {
                        id: btnCamera
                        text: "拍摄"
                        onClicked:
                        {
                            viewStack.currentIndex = 2
                            btnMain.checked = false
                            btn3D.checked = false
                            btn9face.checked = false
                        }
                    }
                    CheckButton {
                        checked: false
                        text: "报告"
                        onClicked:
                        {
                            loadPage("customerReport.qml", { customerID: customerID })

                        }
                    }
                    CheckButton {
                        checked: false
                        text: "回到 用户一览"
                        onClicked: loadPage("customerManager.qml", {})
                    }
                    CheckButton {
                        checked: false

                        text: "回到Home"
                        onClicked:
                        {
                            loadPage("logo.qml",{})
                        }
                    }
                }
            }
        }

        /* ==== 主显示区（右侧内容） ==== */
        StackLayout {
            id: viewStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: 0
            clip: true
			//width: parent.width - leftBar.width; height: parent.height - thumbBar.height

            /* 0: 主画面 */
            RowLayout {
                //anchors.fill: parent
                anchors.margins: 10
                spacing: 20

                // ==========================================================
                // 左侧编辑区域：包含两个平分宽度且保持 3:4 比例的容器
                // ==========================================================
                RowLayout {
                    id: editorsContainer
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 20

                    // 左图片容器
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 8; color: "#222"; border.color: "#ffb300"
                        clip: true

                        Item {
                            anchors.centerIn: parent
                            //anchors.margins: 2 // 留出 2 像素的空隙，防止覆盖 borde
                            width: Math.min(parent.width, parent.height * 3 / 4)
                            height: width * 4 / 3
                            MMImageEditor { // Changed from MMImageEditor to ImageEditor
                                id: leftMain
                                anchors.fill: parent
                                anchors.margins: 2 // 留出 2 像素的空隙，防止覆盖 borde
                                //source: mainphotoes.count > 0 ? mainphotoes.get(curIndex).photoL : ""
                            }
                        }
                    }

                    // 右图片容器
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 8; color: "#222"; border.color: "#ffb300"
                        clip: true

                        Item {
                            anchors.centerIn: parent
                            width: Math.min(parent.width, parent.height * 3 / 4)
                            height: width * 4 / 3

                            MMImageEditor { // Changed from MMImageEditor to ImageEditor
                                id: rightMain
                                anchors.fill: parent
                                anchors.margins: 2 // 留出 2 像素的空隙，防止覆盖 borde
                                //source: mainphotoes.count > 0 ? mainphotoes.get(curIndex).photoR : ""
                            }
                        }
                    }
                }

                // ==========================================================
                // 右侧侧边栏：固定宽度
                // ==========================================================
                Rectangle {
                    id: sideBar
                    Layout.preferredWidth: 210
                    Layout.fillHeight: true
                    color: "#1e1e1e"
                    radius: 8
                    border.color: "#333"

                    // ListView 部分保持不变...
                    ListView {
                        id: subListView
                        anchors.fill: parent
                        anchors.margins: 5
                        model: subphotoes
                        clip: true
                        spacing: 10

                        delegate: Rectangle {
                            width: 195
                            height: 135
                            radius: 6
                            // 增加选中效果：如果当前主图正是这张，则高亮
                            color: "#2a2a2e"
                            border.color: (leftMain.source === modelData.photoL) ? "#ffb300" : "#444"
                            border.width: (leftMain.source === modelData.photoL) ? 2 : 1

                            Column {
                                anchors.centerIn: parent
                                spacing: 4
                                Row {
                                    spacing: 4
                                    Image {
                                        width: 90; height: 120
                                        source: modelData.photoL
                                        fillMode: Image.PreserveAspectFit
                                        sourceSize.width: 90
                                        sourceSize.height: 120
                                        asynchronous: true
                                        // 优化：平滑缩放
                                        //mipmap: true

                                    }
                                    Image {
                                        width: 90; height: 120
                                        source: modelData.photoR
                                        fillMode: Image.PreserveAspectFit
                                        sourceSize.width: 90
                                        sourceSize.height: 120
                                        asynchronous: true
                                        //mipmap: true
                                    }
                                }
                            } // Column

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                hoverEnabled: true
                                onEntered: parent.border.color = "#888"
                                onExited: if(leftMain.source !== modelData.photoL) parent.border.color = "#444"

                                onClicked: {
                                    // 强制触发 UI 刷新（如果模型没自动发信号）
                                    leftMain.source = modelData.photoL
                                    leftMain.init(modelData.IXL,"_L")
                                    rightMain.source = modelData.photoR
                                    rightMain.init(modelData.IXR,"_R")
                                }
                            } // MouseArea
                        }
                    }
                }
            }

            /* 1: 全3D人脸 / 3D模型 — 由 MM3DViewer 负责 3D 显示、侧边栏、自动按 Group 生成 obj、贴图从列表选 */
            MM3DViewer {
                Layout.fillWidth: true
                Layout.fillHeight: true
                subphotoes: customerDetail.subphotoes
                customerID: customerDetail.customerID
                groupID: customerDetail.currentGroupID
                is3DViewActive: viewStack.currentIndex === 1
            }

            /* 2: 拍摄 */
            CameraView {
                customerID: customerDetail.customerID
                isCameraViewActive: viewStack.currentIndex === 2
                onRequestShowMain: {
                    viewStack.currentIndex = 0   // 切回主画面
                    btnMain.checked = true
                    btnCamera.checked = false
                }
                onPhotoSaved: {
                    // 照片保存成功，重新加载缩略图
                    analyseModule.init(customerID)
                    curIndex = 0;
                    loadsubphotoes(curIndex)
                }
            }

            /* 3: 九画面 */
            Rectangle { color: "#18181b"; Label { anchors.centerIn: parent; text: "九画面" } }
        }
    }
}
