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
    /// 用户选择「做分析」后进入；控制自动定位与精修引导
    property bool analyseWorkflowActive: false
    /// 主画面右侧「分析」按钮触发（自动定位后不弹精修选择，直接占位分析）
    property bool analyseFromMainButton: false
    /// 右侧子图列表当前选中项（不用 source 字符串比较：路径/url 格式易不一致）
    property int subListSelectedIndex: 0

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
        subListSelectedIndex = subphotoes.length > 0 ? 0 : -1
        if(subphotoes.length>0)
        {
            // Let the list render first, then load the main editing area in the next frame
            Qt.callLater(() => {
                leftMain.init(subphotoes[0].IXL, "_L");
                leftMain.source = subphotoes[0].photoL;
                rightMain.init(subphotoes[0].IXR, "_R");
                rightMain.source = subphotoes[0].photoR;
            })
        }
    }

    function selectGroupById(groupId) {
        for (var i = 0; i < mainphotoes.length; i++) {
            if (mainphotoes[i].GROUPID === groupId) {
                curIndex = i
                if (thumbBar.thumbPageSize > 0)
                    thumbBar.thumbCurrentPage = Math.floor(i / thumbBar.thumbPageSize) + 1
                loadsubphotoes(i)
                return true
            }
        }
        if (mainphotoes.length > 0) {
            curIndex = mainphotoes.length - 1
            loadsubphotoes(curIndex)
        }
        return false
    }

    function showContourAndAskRefine(message) {
        leftMain.reloadDrawings()
        rightMain.reloadDrawings()
        leftMain.enterShowContour()
        rightMain.enterShowContour()
        postAutoMarkDialog.boxMessage = message || "请选择下一步："
        postAutoMarkDialog.open()
    }

    function beginAnalyseWorkflow() {
        if (!analyseWorkflowActive || currentGroupID <= 0)
            return
        analyseFromMainButton = false
        if (!faceAnalyseManager.groupNeedsAutoMark(customerID, currentGroupID)) {
            showContourAndAskRefine("左右脸轮廓已就绪，请选择下一步：")
            return
        }
        autoMarkDialog.open()
    }

    function runAnalysePlaceholder() {
        leftMain.reloadDrawings()
        rightMain.reloadDrawings()
        leftMain.enterShowContour()
        rightMain.enterShowContour()
        statusMsgBox.boxTitle = "提示"
        statusMsgBox.boxMessage = "图片分析功能开发中，轮廓已就绪。"
        statusMsgBox.open()
        analyseWorkflowActive = false
        analyseFromMainButton = false
    }

    /// 主画面右侧「分析」：无轮廓则自动定位，完成后进入占位分析
    function startGroupAnalyse() {
        if (!faceAnalyseManager) {
            console.warn("faceAnalyseManager not available")
            return
        }
        if (currentGroupID <= 0 || subphotoes.length === 0) {
            statusMsgBox.boxTitle = "提示"
            statusMsgBox.boxMessage = "请先选择要分析的照片组。"
            statusMsgBox.open()
            return
        }
        if (faceAnalyseManager.busy)
            return

        analyseFromMainButton = true
        analyseWorkflowActive = true

        if (faceAnalyseManager.groupNeedsAutoMark(customerID, currentGroupID)) {
            if (!faceAnalyseManager.ensureDetector()) {
                analyseWorkflowActive = false
                analyseFromMainButton = false
                return
            }
            faceAnalyseManager.autoMarkGroup(customerID, currentGroupID)
        } else {
            showContourAndAskRefine("左右脸轮廓已就绪，请选择下一步：")
        }
    }


    /* ==== 上方缩略图栏（整体约 72.5% 缩放） ==== */
    Rectangle
    {
        id: thumbBar
        width: parent.width
        height: 105
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
            anchors.margins: 7
            spacing: 6

            IconButton {
                Layout.preferredWidth: 44
                Layout.preferredHeight: 87
                source: "qrc:/images/left_icon.svg"
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
                    spacing: 6
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.right: parent.right

                    readonly property int delegateWidth: 141
                    property int rowWidth: delegateWidth + spacing

                    Repeater {
                        id: mainThumbList
                        model: mainphotoes.slice((thumbBar.thumbCurrentPage-1)*thumbBar.thumbPageSize, thumbBar.thumbCurrentPage*thumbBar.thumbPageSize)
                        delegate: Rectangle {
                            width: 141; height: 93; radius: 4
                            property int globalIndex: (thumbBar.thumbCurrentPage - 1) * thumbBar.thumbPageSize + index
                            color: globalIndex === curIndex ? "#2a2a2e" : "#333"
                            border.color: globalIndex === curIndex ? "#ffb300" : "#444"
                            border.width: 4

                            Row {
                                leftPadding: 3
                                topPadding: 3
                                //spacing: 8

                                Image {
                                    width: 65; height: 87
                                    fillMode: Image.PreserveAspectFit
                                    source: modelData.photoL
                                    sourceSize.width: 65
                                    sourceSize.height: 87
                                    asynchronous: true
                                }
                                Image {
                                    width: 65; height: 87
                                    fillMode: Image.PreserveAspectFit
                                    source: modelData.photoR
                                    sourceSize.width: 65
                                    sourceSize.height: 87
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
                Layout.preferredWidth: 44
                Layout.preferredHeight: 87
                source: "qrc:/images/right_icon.svg"
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
            width: 192
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
                            btnCamera.checked = false
                        }
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
                        }
                    }
                    CheckButton {
                        checked: false
                        text: "报告"
                        onClicked:
                        {
                            loadPage("customerReport.qml", { customerID: customerID, currentGroupID: currentGroupID })

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
                    Layout.preferredWidth: 162
                    Layout.fillHeight: true
                    color: "#1e1e1e"
                    radius: 8
                    border.color: "#333"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 5
                        spacing: 8

                        TextButton {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            preferredFontPixelSize: 16
                            preferredRadius: 8
                            text: "分析"
                            enabled: currentGroupID > 0 && subphotoes.length > 0
                                    && !(faceAnalyseManager && faceAnalyseManager.busy)
                            onClicked: customerDetail.startGroupAnalyse()
                        }

                        ListView {
                            id: subListView
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            model: subphotoes
                            clip: true
                            spacing: 10

                        delegate: Item {
                            width: ListView.view.width
                            height: 98
                            readonly property bool thumbSelected: index === customerDetail.subListSelectedIndex

                            Rectangle {
                                id: subListThumbRect
                                width: 141
                                height: 98
                                anchors.horizontalCenter: parent.horizontalCenter
                                radius: 6
                                color: parent.thumbSelected ? "#3a3a4e" : "#2a2a2e"
                                property bool thumbHovered: false
                                // 悬停：银灰外框；选中（点击后主图已是该项）：橙色外框
                                border.color: parent.thumbSelected ? "#ffb300"
                                    : (thumbHovered ? "#b8b8c0" : "#444")
                                border.width: parent.thumbSelected ? 2 : 1

                                Column {
                                    anchors.centerIn: parent
                                    spacing: 3
                                    Row {
                                        spacing: 3
                                        Image {
                                            width: 65; height: 87
                                            source: modelData.photoL
                                            fillMode: Image.PreserveAspectFit
                                            sourceSize.width: 65
                                            sourceSize.height: 87
                                            asynchronous: true
                                        }
                                        Image {
                                            width: 65; height: 87
                                            source: modelData.photoR
                                            fillMode: Image.PreserveAspectFit
                                            sourceSize.width: 65
                                            sourceSize.height: 87
                                            asynchronous: true
                                        }
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    hoverEnabled: true
                                    onEntered: subListThumbRect.thumbHovered = true
                                    onExited: subListThumbRect.thumbHovered = false

                                    onClicked: {
                                        customerDetail.subListSelectedIndex = index
                                        leftMain.init(modelData.IXL, "_L")
                                        leftMain.source = modelData.photoL
                                        rightMain.init(modelData.IXR, "_R")
                                        rightMain.source = modelData.photoR
                                    }
                                } // MouseArea
                            }
                        }
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
                onPhotoSaved: function(groupId, startAnalyse) {
                    analyseModule.init(customerID)
                    selectGroupById(groupId)
                    analyseWorkflowActive = startAnalyse
                    if (startAnalyse)
                        Qt.callLater(beginAnalyseWorkflow)
                }
            }
        }
    }

    Connections {
        target: faceAnalyseManager
        function onAutoMarkFinished(success, message) {
            if (success) {
                showContourAndAskRefine(message)
            } else {
                statusMsgBox.boxTitle = "定位失败"
                statusMsgBox.boxMessage = message
                statusMsgBox.open()
                analyseWorkflowActive = false
                analyseFromMainButton = false
            }
        }
        function onErrorMessage(msg) {
            statusMsgBox.boxTitle = "错误"
            statusMsgBox.boxMessage = msg
            statusMsgBox.open()
        }
    }

    ModalChoicePanel {
        id: autoMarkDialog
        anchors.fill: parent
        boxTitle: "自动定位"
        boxMessage: "是否现在对左右脸做自动轮廓定位？"
        choices: [
            { id: "start", text: "开始" },
            { id: "later", text: "稍后" }
        ]
        onChoiceMade: function(choiceId) {
            if (choiceId === "start") {
                if (faceAnalyseManager.ensureDetector())
                    faceAnalyseManager.autoMarkGroup(customerID, currentGroupID)
            } else {
                analyseWorkflowActive = false
            }
        }
    }

    ModalChoicePanel {
        id: postAutoMarkDialog
        anchors.fill: parent
        boxTitle: "轮廓定位结果"
        boxMessage: "请选择下一步："
        choices: [
            { id: "analyse", text: "继续分析" },
            { id: "refine", text: "手动精修轮廓" }
        ]
        onChoiceMade: function(choiceId) {
            if (choiceId === "refine") {
                leftMain.enterEditContour()
                rightMain.enterEditContour()
            } else {
                runAnalysePlaceholder()
            }
        }
    }

    MessageBox {
        id: statusMsgBox
    }

    /// 定位进行中提示（InputFunnelBlocker 在 App 层拦截点击）
    Rectangle {
        anchors.fill: parent
        visible: faceAnalyseManager && faceAnalyseManager.busy
        color: "#66000000"
        z: 1500
        Text {
            anchors.centerIn: parent
            text: "正在自动定位轮廓…"
            color: "#ffffff"
            font.pixelSize: 22
            font.bold: true
        }
    }
}
