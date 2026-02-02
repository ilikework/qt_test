import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
//import Custom3D
import QtQuick3D 6.7
import QtQuick3D.Helpers
import QtQuick3D.AssetUtils

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

    Component.onCompleted:
    {
        console.log("Component.onCompleted start")
        analyseModule.init(customerID);

        loadsubphotoes(0)


    }

    function loadsubphotoes(index)
    {

        subphotoes = analyseModule.loadSub(mainphotoes[index].GROUPID)
        if(subphotoes.length>0)
        {
            // 先让列表渲染，下一帧再加载主编辑区域
            Qt.callLater(() => {
                leftMain.source = subphotoes[0].photoL;
                leftMain.init(subphotoes[0].IXL,"_L");
                rightMain.source = subphotoes[0].photoR;
                rightMain.init(subphotoes[0].IXR,"_R");
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

        RowLayout
        {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 8

            IconButton {
                Layout.preferredWidth:60
                Layout.preferredHeight:120
                source: "./images/left_icon.svg"
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

                    property int rowWidth: 188*2+2
                    property int pageSize: 0
                    property int currentPage: 1

                    Repeater {
                        id: mainThumbList
                        model: mainphotoes.slice((thumbRow.currentPage-1)*thumbRow.pageSize, thumbRow.currentPage*thumbRow.pageSize)
                        delegate: Rectangle {
                            width: 188; height: 128; radius: 4
                            color: index === curIndex ? "#2a2a2e" : "#333"
                            border.color: index === curIndex ? "#ffb300" : "#444"
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
                                    if(curIndex!=index)
                                    {
                                        curIndex = index
                                        loadsubphotoes(curIndex)
                                        return
                                    }

                                    thumbBar.expandedIndex =
                                            (thumbBar.expandedIndex === index ? -1 : index)
                                }
                            }
                        }
                    }
                    onWidthChanged:
                    {
                        console.log("onWidthChanged in")
                        pageSize = Math.floor(thumbRow.width / thumbRow.rowWidth)
                    }
                }
            }
            IconButton {
                Layout.preferredWidth:60
                Layout.preferredHeight:120
                source: "./images/right_icon.svg"
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
            //padding: 20

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
                            btn3DModule.checked = false
                            btn9face.checked = false
                            btnCamera.checked = false
                        }
                    }
                    CheckButton
                    {
                        id:btn3D
                        text: "全3D人脸"
                        onClicked:
                        {
                            viewStack.currentIndex = 1
                            btnMain.checked = false
                            btn3DModule.checked = false
                            btn9face.checked = false
                            btnCamera.checked = false
                        }
                    }
                    CheckButton
                    {
                        id:btn3DModule
                        text: "3D模型"
                        onClicked:
                        {
                            viewStack.currentIndex = 1
                            btnMain.checked = false
                            btn3D.checked = false
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
                            btn3DModule.checked = false
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
                            btn3DModule.checked = false
                            btn9face.checked = false
                        }
                    }
                    CheckButton {
                        checked: false
                        text: "报告"
                        onClicked:
                        {
                            console.log("进入报告", customerID)
                            loadPage("customerReport.qml", { customerID: customerID })

                        }
                    }
                    CheckButton {
                        checked: false

                        text: "回到Home"
                        onClicked:
                        {
                            console.log("回到Home")
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
                            MMImageEditor {
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

                            MMImageEditor {
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

            /* 1: 3D */
            Item {
                id: parentItem
                //anchors.centerIn: parent
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                width: 800
                height: 600

                View3D {
                    id: view3d
                    anchors.fill: parentItem
                    camera: camera

                    environment: ExtendedSceneEnvironment {
                        antialiasingMode: SceneEnvironment.MSAA
                        antialiasingQuality: SceneEnvironment.High
                        backgroundMode: SceneEnvironment.Color
                        clearColor: "#202020"
                    }

                    PerspectiveCamera {
                        id: camera
                        position: Qt.vector3d(0, 0, 2)
                        clipNear: 0.1
                        clipFar: 1000
                    }

                    DirectionalLight {
                        id: sunLight
                        eulerRotation: Qt.vector3d(-30, 0, 0)
                        color: "white"
                        //brightness: 50
                    }

                    Node { id: originNode; position: Qt.vector3d(0,0,0) }

                    // ★ 我们自己的皮肤贴图（可控 mipmap & clamp）★
                    Texture {
                        id: skinTex
                        source:  btn3D.checked ? "./customers/0000001/0000001_33_MResult_2-0000001_41_texture.jpg":"./customers/0000001/0000001_33_MResult_2-0000001_44_texture.jpg"


                        generateMipmaps: false
                        minFilter: Texture.Linear
                        mipFilter: Texture.None
                        magFilter: Texture.Linear

                        // clamp 与否都行，主要是禁止 mipmap
                        tilingModeHorizontal: Texture.ClampToEdge
                        tilingModeVertical: Texture.ClampToEdge

                    }

                    // ★ 我们自己的材质，绑定上面的 texture ★
                    PrincipledMaterial {
                        id: skinMat
                        baseColor: "white"
                        roughness: 1.0
                        metalness: 0.0
                        specularAmount: 0.0

                        baseColorMap: skinTex
                    }

                    // 3. RuntimeLoader 只负责 load，不要在声明里写 materials:
                    RuntimeLoader {
                        id: importNode
                        parent: originNode
                        //source: btn3D.checked ? "./customers/0000001/0000001_33_MResult_2-0000001_41.obj":"./customers/0000001/0000001_33_MResult_2-0000001_44.obj"
                        source: "./customers/0000001/0000001_33_MResult_2-0000001_41.obj"

                        // 状态变化时调用
                        onStatusChanged: {
                            console.log("RuntimeLoader status =", status, "error:", errorString)

                            if (status === RuntimeLoader.Success) {
                                // 缩放整个导入的模型
                                importNode.scale = Qt.vector3d(1, 1, 1)

                                // 给所有子 Model 换材质
                                applyMaterialRecursively(importNode)
                            }
                        }

                        // 递归函数：从某个 node 往下找所有有 materials 属性的对象
                        function applyMaterialRecursively(node) {
                            console.log( node)
                            if (!node)
                                return

                            // 注意：这里不要在 QML 静态代码里写 materials:
                            // 而是运行时检查有没有这个属性
                            if ("materials" in node) {
                                console.log("found Model-like node, set materials:", node)
                                node.materials = [ skinMat ]
                            }

                            var kids = node.children
                            for (var i = 0; i < kids.length; ++i) {
                                applyMaterialRecursively(kids[i])
                            }
                        }
                    }

                    // 旋转灯光（和原来一样）
                    Node {
                        id: lightPivot
                        position: Qt.vector3d(0,0,0)

                        DirectionalLight {
                            id: rotatingLight
                            eulerRotation: Qt.vector3d(-45,0,0)
                            color: "white"
                            //brightness: 40
                        }

                        NumberAnimation on eulerRotation.y {
                            from: 0
                            to: 360
                            duration: 10000
                            loops: Animation.Infinite
                            running: true
                        }
                    }

                    OrbitCameraController {
                        camera: camera
                        origin: originNode
                    }

                    Component.onCompleted: {
                        console.log("View3D size =", width, height)
                    }
                }
            }

            /* 2: 拍摄 */
            CameraView{
                customerID : customerDetail.customerID
                onRequestShowMain: {
                    viewStack.currentIndex = 0   // 切回主画面
                }}

            /* 3: 九画面 */
            Rectangle { color: "#18181b"; Label { anchors.centerIn: parent; text: "九画面" } }
        }
    }

}
