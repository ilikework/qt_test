import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.AssetUtils

import "components"

Item {
    id: customerDetail
    visible: true

    signal loadPage(string page, var params)

    property string customerID: ""
    property int curIndex: -1
    property var groups: []

    ListModel { id: mainphotoes }
    ListModel { id: subphotoes }

    Component.onCompleted: {
        console.log("Component.onCompleted start")
        console.log("Item版: devicePixelRatio =", Qt.application.screens[0].devicePixelRatio,
                    "pixelDensity =", Qt.application.screens[0].pixelDensity)

        if (customerID === "0000001") {
            curIndex = 0
            groups = ["01"]
        } else {
            curIndex = 0
            groups = ["01", "02"]
        }

        mainphotoes.clear()
        for (var i = 0; i < groups.length; i++) {
            mainphotoes.append({
                                   photoL: `customers/${customerID}/${customerID}_${groups[i]}_01_L.jpg`,
                                   photoR: `customers/${customerID}/${customerID}_${groups[i]}_01_R.jpg`
                               })
        }

        loadsubphotoes(curIndex)
        console.log("Component.onCompleted end")
    }

    function loadsubphotoes(index) {
        subphotoes.clear()
        if (index >= 0) {
            for (var i = 1; i <= 8; i++) {
                var sub = String(i).padStart(2, "0")
                subphotoes.append({
                                      photoL: `customers/${customerID}/${customerID}_${groups[index]}_${sub}_L.jpg`,
                                      photoR: `customers/${customerID}/${customerID}_${groups[index]}_${sub}_R.jpg`
                                  })
            }
        }
    }

    /* ==== 上方缩略图栏 ==== */
    Rectangle {
        id: thumbBar
        width: parent.width
        height: 145
        color: "#222226"
        border.color: "#444"
        border.width: 1
        anchors.top: parent.top

        property int expandedIndex: -1

        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 8

            Button { text: "←"; Layout.preferredWidth: 60 }

            Item {
                Layout.fillWidth: true
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
                            width: 188
                            height: 128
                            radius: 4
                            color: index === curIndex ? "#2a2a2e" : "#333"
                            border.color: index === curIndex ? "#ffb300" : "#444"
                            border.width: 4

                            Row {
                                leftPadding: 4
                                topPadding: 4

                                Image {
                                    width: 90
                                    height: 120
                                    fillMode: Image.PreserveAspectFit
                                    source: photoL
                                }
                                Image {
                                    width: 90
                                    height: 120
                                    fillMode: Image.PreserveAspectFit
                                    source: photoR
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (curIndex !== index) {
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

    Rectangle {
        id: showgroupimgs
        width: 100 * 16
        height: thumbBar.height
        y: thumbBar.y + thumbBar.height
        color: "white"
        z: 100
        visible: false

        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 8

            Repeater {
                model: subphotoes
                delegate: Rectangle {
                    width: 188
                    height: 128
                    radius: 4
                    color: index === 0 ? "#2a2a2e" : "#333"
                    border.color: index === 0 ? "#ffb300" : "#444"
                    border.width: 4

                    Row {
                        leftPadding: 4
                        topPadding: 4

                        Image {
                            id: subThumbImgL
                            width: 90
                            height: 120
                            source: photoL
                            fillMode: Image.PreserveAspectFit
                            Layout.alignment: Qt.AlignVCenter
                        }
                        Image {
                            id: subThumbImgR
                            width: 90
                            height: 120
                            source: photoR
                            fillMode: Image.PreserveAspectFit
                            Layout.alignment: Qt.AlignVCenter
                            anchors.leftMargin: 8
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            mainphotoes.set(thumbBar.expandedIndex, {
                                                photoL: photoL,
                                                photoR: photoR
                                            })
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

        /* 左边按钮栏 */
        Rectangle {
            id: leftBar
            width: 240
            color: "#232325"
            Layout.fillHeight: true

            ScrollView {
                width: parent.width
                height: parent.height
                contentWidth: parent.width
                clip: true

                Column {
                    id: buttonColumn
                    spacing: 8
                    width: parent.width

                    CheckButton {
                        id: btnMain
                        text: "🖼️  主画面"
                        checked: true
                        onClicked: {
                            viewStack.currentIndex = 0
                            btn3D.checked = false
                            btn3DModule.checked = false
                            btn9face.checked = false
                        }
                    }
                    CheckButton {
                        id: btn3D
                        text: "👤  全3D人脸"
                        onClicked: {
                            viewStack.currentIndex = 1
                            btnMain.checked = false
                            btn3DModule.checked = false
                            btn9face.checked = false
                        }
                    }
                    CheckButton {
                        id: btn3DModule
                        text: "🗿  3D模型"
                        onClicked: {
                            viewStack.currentIndex = 1
                            btnMain.checked = false
                            btn3D.checked = false
                            btn9face.checked = false
                        }
                    }
                    CheckButton {
                        id: btn9face
                        text: "#⃣   九画面"
                        onClicked: {
                            viewStack.currentIndex = 2
                            btnMain.checked = false
                            btn3D.checked = false
                            btn3DModule.checked = false
                        }
                    }

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

                        MyButton2 {
                            text: "直线测量"
                            onClicked: console.log("直线测量模式")
                        }
                        MyButton2 {
                            text: "3点圆形测量"
                            onClicked: console.log("3点圆形测量模式")
                        }
                    }

                    CheckButton { text: "🛠️  系统工具" }
                    CheckButton {
                        text: "📷  拍摄"
                        onClicked: {
                            cameraDlg.parent = customerDetail
                            cameraDlg.open()
                        }
                    }
                    CheckButton {
                        text: "📄  报告"
                        onClicked: {
                            console.log("进入报告", customerID)
                            loadPage("customerReport.qml", { "customerID": customerID })
                        }
                    }
                    CheckButton {
                        text: "回到Home"
                        onClicked: {
                            console.log("回到Home")
                            loadPage("logo.qml", {})
                        }
                    }
                }
            }
        }

        /* 右侧 viewStack 占位区 */
        StackLayout {
            id: viewStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: 0
            clip: true

            /* 0: 主画面 */
            Row {
                id: mainRow
                spacing: 20
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.fillHeight: true

                Rectangle {
                    width: mainRow.width / 2 - 10
                    height: mainRow.height - 10
                    radius: 8
                    color: "#222"
                    border.color: "#ffb300"
                    Image {
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectFit
                        source: mainphotoes.count > 0 ? mainphotoes.get(curIndex).photoL : ""
                    }
                }
                Rectangle {
                    width: mainRow.width / 2 - 10
                    height: mainRow.height - 10
                    radius: 8
                    color: "#222"
                    border.color: "#ffb300"
                    Image {
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectFit
                        source: mainphotoes.count > 0 ? mainphotoes.get(curIndex).photoR : ""
                    }
                }
            }

            /* 1: 3D模式：这里只留一个空 Item 当占位，真正的 View3D 在下面 overlay 容器里 */
            Item { }

            /* 2: 九画面 */
            Rectangle {
                color: "#18181b"
                Label { anchors.centerIn: parent; text: "九画面" }
            }
        }
    }

    /* === 关键：单独的 View3D overlay 容器，不参与 StackLayout 布局 === */
    Item {
        id: parentItem3D
        z: 50
        visible: viewStack.currentIndex === 1

        anchors {
            top: thumbBar.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            leftMargin: leftBar.width    // 利用左栏宽度腾出 3D 区域
        }

        // 这里就是你最开始贴的那段 3D 代码，几乎原封不动搬过来：
        Item {
            id: inner3DArea
            anchors.centerIn: parent
            width: 800
            height: 600

            View3D {
                id: view3d
                anchors.fill: inner3DArea

                environment: ExtendedSceneEnvironment {
                    antialiasingMode: SceneEnvironment.MSAA
                    antialiasingQuality: SceneEnvironment.High
                    temporalAAEnabled: true
                }

                PerspectiveCamera {
                    id: camera
                    position: Qt.vector3d(0, 0, 30)
                }

                DirectionalLight {
                    id: sunLight
                    eulerRotation: Qt.vector3d(-45, 60, 0)
                    color: "white"
                }

                Node { id: originNode; position: Qt.vector3d(0, 0, 0) }

                Node {
                    id: modelContainer
                    parent: originNode
                    scale: Qt.vector3d(10, 10, 10)
                    RuntimeLoader {
                        source: "./customers/0000001/0000001_01.obj"
                    }
                }

                OrbitCameraController {
                    camera: camera
                    origin: originNode
                }

                // 如果你原来还有旋转光源，也一起搬过来
                // Node {
                //     id: lightPivot
                //     position: Qt.vector3d(0,0,0)
                //     DirectionalLight { ... }
                //     NumberAnimation on eulerRotation.y { ... }
                // }
            }
        }
    }
    CameraDlg { id: cameraDlg }
}
