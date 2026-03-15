import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D 6.7
import QtQuick3D.Helpers
import QtQuick3D.AssetUtils

// 3D 人脸/模型：每个 Group 用 01_L.jpg+01_R.jpg 生成，exe 产出 01_L-01_R.obj、01_L-01_R_texture.jpg，贴图用 exe 默认。
// 切换到全3D人脸时若该 Group 尚无 obj 则自动生成。
Item {
    id: root

    property var subphotoes: []
    property string customerID: ""
    property int groupID: 0
    property bool is3DViewActive: false

    readonly property string groupIdStr: groupID < 10 ? ("0" + groupID) : String(groupID)
    readonly property string groupDir: applicationDirPath + "/customers/" + customerID + "/" + groupIdStr
    readonly property string defaultObjName: "01_L-01_R.obj"

    /// 当前选中的贴图对应 subphotoes 的索引，默认第一组
    property int selectedTextureIndex: 0

    property string objSource: ""
    property string textureSource: ""
    property string generationErrorMessage: ""

    /// true = 无人操作，obj 左右摆动；false = 用户正在操作，停止摆动
    property bool swingIdle: true

    function ensureObjGenerated() {
        if (!customerID || groupID <= 0) return
        var localObj = groupDir + "/" + defaultObjName
        if (mm3dManager.fileExists(localObj)) {
            applyObjAndTexturePaths()
            return
        }
        var json = buildFaceReconJson()
        if (!json) {
            generationErrorMessage = "无可用左右图数据，无法生成 3D"
            return
        }
        mm3dManager.runFaceRecon(json)
    }

    /// 根据 subphotoes[index] 的左右图路径得到贴图文件名：{左图名}-{右图名}.png
    function getTextureFileName(index) {
        if (!subphotoes || index < 0 || index >= subphotoes.length) return ""
        var leftPath = String(subphotoes[index].photoL || "")
        var rightPath = String(subphotoes[index].photoR || "")
        leftPath = leftPath.replace(/^file:\/\/\//, "").replace(/\\/g, "/")
        rightPath = rightPath.replace(/^file:\/\/\//, "").replace(/\\/g, "/")
        var leftName = leftPath.split("/").pop().replace(/\.[^.]*$/, "")
        var rightName = rightPath.split("/").pop().replace(/\.[^.]*$/, "")
        if (!leftName || !rightName) return ""
        return leftName + "-" + rightName + ".png"
    }

    function applyObjAndTexturePaths() {
        var base = "file:///" + groupDir.replace(/\\/g, "/") + "/"
        root.objSource = base + defaultObjName
        var idx = selectedTextureIndex
        if (idx < 0 || idx >= (subphotoes ? subphotoes.length : 0)) idx = 0
        var texName = getTextureFileName(idx)
        root.textureSource = texName ? (base + texName) : ""
    }

    function normalizedPath(p) {
        if (!p) return ""
        return String(p).replace(/^file:\/\/\//, "").replace(/\\/g, "/").replace(/\/+$/, "")
    }

    /// 从 subphotoes（8 对）构建 FaceRecon 所需 JSON：model + info[左图obj, 左图贴图数组, 右图obj, 右图贴图数组, 输出目录]
    function buildFaceReconJson() {
        if (!subphotoes || subphotoes.length === 0) return ""
        var leftPaths = []
        var rightPaths = []
        for (var i = 0; i < subphotoes.length; i++) {
            leftPaths.push(normalizedPath(subphotoes[i].photoL || ""))
            rightPaths.push(normalizedPath(subphotoes[i].photoR || ""))
        }
        if (leftPaths[0] === "" || rightPaths[0] === "") return ""
        var outDir = normalizedPath(groupDir)
        var obj = {
            "model": 5,
            "info": [ leftPaths[0], leftPaths, rightPaths[0], rightPaths, outDir ]
        }
        return JSON.stringify(obj)
    }

    onIs3DViewActiveChanged: {
        if (is3DViewActive)
            Qt.callLater(ensureObjGenerated)
    }

    onGroupIDChanged: {
        root.objSource = ""
        root.textureSource = ""
        root.generationErrorMessage = ""
        root.selectedTextureIndex = 0
        if (root.is3DViewActive)
            Qt.callLater(ensureObjGenerated)
    }

    onSelectedTextureIndexChanged: {
        if (root.objSource && root.textureSource) {
            var base = "file:///" + groupDir.replace(/\\/g, "/") + "/"
            var texName = getTextureFileName(selectedTextureIndex)
            if (texName) root.textureSource = base + texName
        }
    }

    Connections {
        target: mm3dManager
        function onFinished(success, outputDir) {
            console.log("[MM3D] onFinished success:", success, "outputDir:", outputDir)
            if (success)
                root.generationErrorMessage = ""
            if (!success || !outputDir) return
            var outNorm = root.normalizedPath(outputDir)
            var groupNorm = root.normalizedPath(root.groupDir)
            if (outNorm !== groupNorm) return
            root.objSource = ""
            root.textureSource = ""
            reloadTimer.start()
        }
        function onErrorMessage(msg) {
            console.warn("MM3D:", msg)
            root.generationErrorMessage = msg || "3D生成失败"
        }
    }

    Timer {
        id: reloadTimer
        interval: 250
        repeat: false
        onTriggered: {
            root.applyObjAndTexturePaths()
        }
    }

    /// 30 秒无操作后恢复 obj 左右摆动
    Timer {
        id: idleTimer
        interval: 30000
        repeat: false
        onTriggered: root.swingIdle = true
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 20

        Item {
            id: viewArea
            Layout.fillWidth: true
            Layout.fillHeight: true

            View3D {
                id: view3d
                anchors.fill: parent
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
                }

                Node { id: originNode; position: Qt.vector3d(0, 0, 0) }

                Texture {
                    id: skinTex
                    source: root.textureSource || "."
                    generateMipmaps: false
                    minFilter: Texture.Linear
                    mipFilter: Texture.None
                    magFilter: Texture.Linear
                    tilingModeHorizontal: Texture.ClampToEdge
                    tilingModeVertical: Texture.ClampToEdge
                }

                PrincipledMaterial {
                    id: skinMat
                    baseColor: "white"
                    roughness: 1.0
                    metalness: 0.0
                    specularAmount: 0.0
                    baseColorMap: skinTex
                }

                RuntimeLoader {
                    id: importNode
                    parent: originNode
                    source: root.objSource || ""

                    onStatusChanged: {
                        if (status === RuntimeLoader.Success) {
                            importNode.scale = Qt.vector3d(1, 1, 1)
                            applyMaterialRecursively(importNode)
                        }
                    }

                    function applyMaterialRecursively(node) {
                        if (!node) return
                        if ("materials" in node) node.materials = [ skinMat ]
                        var kids = node.children
                        for (var i = 0; i < kids.length; ++i) applyMaterialRecursively(kids[i])
                    }
                }

                /// 人脸中心轴左右 ±90° 摆动，有 obj 且 30 秒无操作时运行
                SequentialAnimation {
                    id: swingAnimation
                    running: root.swingIdle && !!root.objSource
                    loops: Animation.Infinite
                    NumberAnimation {
                        target: importNode
                        property: "eulerRotation.y"
                        from: -90
                        to: 90
                        duration: 4000
                        easing.type: Easing.InOutSine
                    }
                    NumberAnimation {
                        target: importNode
                        property: "eulerRotation.y"
                        from: 90
                        to: -90
                        duration: 4000
                        easing.type: Easing.InOutSine
                    }
                }

                Node {
                    id: lightPivot
                    position: Qt.vector3d(0, 0, 0)
                    DirectionalLight {
                        id: rotatingLight
                        eulerRotation: Qt.vector3d(-45, 0, 0)
                        color: "white"
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
                    xInvert: true
                    yInvert: false
                }
            }

            /// 仅在 View3D 内按下鼠标后才停止摆动并重置 30 秒无操作计时，避免只移入/移动就停
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.AllButtons
                propagateComposedEvents: true
                hoverEnabled: true
                onPressed: function(mouse) {
                    root.swingIdle = false
                    idleTimer.restart()
                    mouse.accepted = false
                }
                onPositionChanged: function(mouse) {
                    if (mouse.buttons !== 0) {
                        root.swingIdle = false
                        idleTimer.restart()
                    }
                    mouse.accepted = false
                }
                onReleased: function(mouse) {
                    idleTimer.restart()
                    mouse.accepted = false
                }
            }

            Rectangle {
                anchors.fill: viewArea
                color: "#80000000"
                visible: mm3dManager.running
                BusyIndicator { anchors.centerIn: parent; running: true }
                Text {
                    anchors.centerIn: parent
                    anchors.verticalCenterOffset: 30
                    text: "正在生成 3D 模型…"
                    color: "white"
                }
            }

            Rectangle {
                anchors.fill: viewArea
                color: "#1a0a0a"
                visible: root.generationErrorMessage !== ""
                Text {
                    anchors.centerIn: parent
                    text: root.generationErrorMessage
                    color: "#e04040"
                    font.pixelSize: 32
                    font.bold: true
                }
            }
        }

        Rectangle {
            id: sideBar3D
            Layout.preferredWidth: 210
            Layout.fillHeight: true
            color: "#1e1e1e"
            radius: 8
            border.color: "#333"

            Column {
                anchors.fill: parent
                anchors.margins: 2
                spacing: 2

                Item {
                    width: parent.width - 2
                    height: parent.height - 30
                    anchors.horizontalCenter: parent.horizontalCenter

                    ListView {
                        id: subListView3D
                        anchors.fill: parent
                        anchors.leftMargin: 2
                        anchors.rightMargin: 2
                        anchors.topMargin: 2
                        anchors.bottomMargin: 2
                        model: root.subphotoes
                        clip: true
                        spacing: 10

                        delegate: Rectangle {
                            width: 200
                            height: 148
                            anchors.horizontalCenter: parent.horizontalCenter
                            radius: 6
                            property bool hovered: false
                            color: root.selectedTextureIndex === index ? "#3a3a4e" : "#2a2a2e"
                            border.color: hovered ? "#888" : (root.selectedTextureIndex === index ? "#ffb300" : "#444")
                            border.width: root.selectedTextureIndex === index ? 2 : 1

                            Row {
                                anchors.centerIn: parent
                                spacing: 4
                                Image {
                                    width: 90
                                    height: 120
                                    source: modelData.photoL
                                    fillMode: Image.PreserveAspectFit
                                    sourceSize.width: 90
                                    sourceSize.height: 120
                                    asynchronous: true
                                }
                                Image {
                                    width: 90
                                    height: 120
                                    source: modelData.photoR
                                    fillMode: Image.PreserveAspectFit
                                    sourceSize.width: 90
                                    sourceSize.height: 120
                                    asynchronous: true
                                }
                            }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            hoverEnabled: true
                            onEntered: parent.hovered = true
                            onExited: parent.hovered = false
                            onClicked: root.selectedTextureIndex = index
                        }
                    }
                }
                }
            }
        }
    }
}
