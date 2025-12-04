import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.AssetUtils

import "components"


Window {
    id: customerDetail
    visible: true

    //signal loadPage(string page, var params)
    property string customerID : ""
    property int curIndex:-1
    property var groups:[]
    ListModel {
        id: mainphotoes
    }
    ListModel {
        id: subphotoes
    }
    //property var mainphotoes: []
    //property var subphotoes:[]

    Component.onCompleted:
    {
        console.log("Component.onCompleted start")
        if(customerID==="0000001")
        {
            curIndex =0
            groups = ["01"]
        }
        else
        {
            curIndex =0
            groups = ["01","02"]

        }

        mainphotoes.clear()
        for(var i=0;i<groups.length;i++)
            mainphotoes.append({photoL:`customers/${customerID}/${customerID}_${groups[i]}_01_L.jpg`,
                              photoR:`customers/${customerID}/${customerID}_${groups[i]}_01_R.jpg`})
        //console.log("mainphotoes size =", mainphotoes.length)
        //console.log(JSON.stringify(mainphotoes))
        //console.log("curIndex =", curIndex)

        loadsubphotoes(curIndex)

        console.log("Window版: devicePixelRatio =",
                    Screen.devicePixelRatio,
                    "pixelDensity =", Screen.pixelDensity)
        //thumbRow.update()
        console.log("Component.onCompleted end")


    }

    function loadsubphotoes(index)
    {
        subphotoes.clear()

        if(index>=0)
        {
            for(var i=1;i<=8;i++)
            {
                var sub = String(i).padStart(2, "0")
                subphotoes.append({ photoL:`customers/${customerID}/${customerID}_${groups[index]}_${sub}_L.jpg`,
                                    photoR:`customers/${customerID}/${customerID}_${groups[index]}_${sub}_R.jpg`})
            }
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

            Button { text: "←"; Layout.preferredWidth: 60 }
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

                    Repeater {
                        id: mainThumbList
                        model: mainphotoes
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
                                    source: photoL
                                }
                                Image {
                                    width: 90; height: 120
                                    fillMode: Image.PreserveAspectFit
                                    source: photoR
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

                                    showgroupimgs.visible =
                                            thumbBar.expandedIndex === index
                                }
                            }
                        }
                    }
                }
            }
           Button { text: "→"; Layout.preferredWidth: 60 }
        }
    }
    Rectangle
    {
        id:showgroupimgs
        width: 100 *16
        height: thumbBar.height
        y : thumbBar.y + thumbBar.height
        color: "white"
        z :1
        visible: false //thumbBar.expandedIndex === index
        RowLayout
        {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 8

            Repeater
            {
                model: subphotoes
                delegate: Rectangle
                {
                    width: 188; height: 128; radius: 4
                    color: index === 0 ? "#2a2a2e" : "#333"
                    border.color: index === 0 ? "#ffb300" : "#444"
                    border.width: 4
                    Row
                    {
                        leftPadding: 4
                        topPadding:  4
                        Image
                        {
                            id:subThumbImgL
                            width:90
                            height:120
                            source: photoL
                            fillMode: Image.PreserveAspectFit
                            Layout.alignment: Qt.AlignVCenter
                        }
                        Image
                        {
                            id:subThumbImgR
                            width:90
                            height:120
                            source: photoR
                            fillMode: Image.PreserveAspectFit
                            Layout.alignment: Qt.AlignVCenter
                            anchors.leftMargin: 8
                        }
                    }
                    MouseArea
                    {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked:
                        {
                            mainphotoes.set(thumbBar.expandedIndex,{photoL:photoL,photoR:photoR})
                            mainphotoesChanged()
                            console.log("已替换 index:", thumbBar.expandedIndex,
                                        "photoL:", modelData.photoL,
                                        "photoR:", modelData.photoR)
                            showgroupimgs.visible = false
                            thumbBar.expandedIndex = -1
                        }
                    }


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
                        text: "🖼️  主画面"
                        checked: true
                        onClicked:
                        {
                            viewStack.currentIndex = 0
                            btn3D.checked = false
                            btn3DModule.checked = false
                            btn9face.checked = false

                        }
                    }
                    CheckButton
                    {
                        id:btn3D
                        text: "👤  全3D人脸"
                        onClicked:
                        {
                            viewStack.currentIndex = 1
                            btnMain.checked = false
                            btn3DModule.checked = false
                            btn9face.checked = false

                        }
                    }
                    CheckButton
                    {
                        id:btn3DModule
                        text: "🗿  3D模型"
                        onClicked:
                        {
                            viewStack.currentIndex = 1
                            btnMain.checked = false
                            btn3D.checked = false
                            btn9face.checked = false

                        }
                    }
                    //CheckButton  { text: "📊  定量分析"; onClicked: viewStack.currentIndex = 3 }
                    CheckButton
                    {
                        id:btn9face
                        text: "#⃣   九画面"
                        onClicked:
                        {
                            viewStack.currentIndex = 2
                            btnMain.checked = false
                            btn3D.checked = false
                            btn3DModule.checked = false

                        }
                    }
                    //MyButton  { text: "🔲  网格定位"; onClicked: viewStack.currentIndex = 5 }

                    /* ==== 测量按钮及子菜单 ==== */
                    CheckButton {
                        id: btnMeasure
                        text: "📏  测量"
                        onClicked: measureMenu.visible = checked
                    }

                    Column {
                        id: measureMenu
                        visible: false
                        spacing: 2
						width: parent.width
                        opacity: visible ? 1 : 0
                        Behavior on opacity { NumberAnimation { duration: 250 } }
                        Behavior on visible { PropertyAnimation { duration: 250 } }

                        MyButton2 {
                            text: "直线测量"
                            onClicked: console.log("直线测量模式")
                        }
                        MyButton2 {
                            text: "3点圆形测量"
                            onClicked: console.log("3点圆形测量模式")
                        }
                    }

                    CheckButton {
                        checked: false

                        text: "🛠️  系统工具"
                    }
                    CheckButton
                    {
                        checked: false
                        text: "📷  拍摄"
                        onClicked: {
                            //cameraDlg.modality = Qt.WindowModal
                            cameraDlg.parent = customerDetail
                            cameraDlg.open()
                        }
                    }
                    CheckButton {
                        checked: false
                        text: "📄  报告"
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
            Row {
                spacing: 20
                anchors.centerIn: parent
                Rectangle {
                    width: viewStack.width/2 -10; height: viewStack.height -10
                    radius: 8; color: "#222"; border.color: "#ffb300"
                    Image { anchors.fill: parent; fillMode: Image.PreserveAspectFit; source: mainphotoes.count > 0 ? mainphotoes.get(curIndex).photoL : ""}
                }
                Rectangle {
                    width: viewStack.width/2 -10; height: viewStack.height -10
                    radius: 8; color: "#222"; border.color: "#ffb300"
                    Image { anchors.fill: parent; fillMode: Image.PreserveAspectFit; source: mainphotoes.count > 0 ? mainphotoes.get(curIndex).photoR : "" }
                }
            }

            /* 1: 3D模式 */
            Item {
                id:parentItem
                anchors.centerIn: parent
                width:  800//viewStack.width-10
                height: 600//viewStack.height -10
                //radius: 8; color: "#222"; border.color: "#ffb300"
                View3D {
                    id:view3d
                    anchors.fill: parentItem
                    Component.onCompleted: {
                        console.log("View3D size =", width, height)
                    }

                    environment: ExtendedSceneEnvironment {
                        // 背景统一成白的
                        backgroundMode: SceneEnvironment.Color
                        clearColor: "white"

                        // 尝试只用 MSAA，不叠 SSAA/TAA/FXAA
                        antialiasingMode: SceneEnvironment.MSAA
                        antialiasingQuality: SceneEnvironment.High

                        fxaaEnabled: false
                        temporalAAEnabled: false
                    }

                    PerspectiveCamera { id: camera; position: Qt.vector3d(0,0,30) }
                    DirectionalLight {
                        id: sunLight
                        // Set rotation using Euler angles (degrees)
                        eulerRotation: Qt.vector3d(-45, 60, 0)
                        color: "white"
                        //intensity: 1.0
                    }
                    Node { id: originNode; position: Qt.vector3d(0,0,0) }


                    Node { id: modelContainer; parent: originNode; scale: Qt.vector3d(10,10,10)
                        RuntimeLoader { source: "./customers/0000001/0000001_01.obj" }
                    }
                    Texture {
                        id: skinTex
                        source: "./customers/0000001/0000001_01_texture.jpg"
                        minFilter: Texture.LinearMipMapLinear
                        magFilter: Texture.Linear
                        //mipmap: true
                    }
                    // Orbit controller
                    OrbitCameraController {
                        camera: camera
                        origin: originNode
                    }

                    // Light pivot: rotates around model
                    Node {
                        id: lightPivot
                        position: Qt.vector3d(0,0,0)

                        DirectionalLight {
                            id: rotatingLight
                            eulerRotation: Qt.vector3d(-45,0,0)
                            color: "white"
                            //intensity: 1.0
                        }

                        // Animate the pivot's rotation
                        NumberAnimation on eulerRotation.y {
                            from: 0
                            to: 360
                            duration: 10000
                            loops: Animation.Infinite
                            running: true
                        }
                    }
                }
            }

            /* 2: 九画面 */
            Rectangle { color: "#18181b"; Label { anchors.centerIn: parent; text: "九画面" } }
        }
    }


    CameraDlg {
        id: cameraDlg
        // onAccepted: {
        //     console.log("保存:", userName)
        // }
    }

}
