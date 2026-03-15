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
    readonly property string defaultTexName: "01_L-01_R_texture.jpg"

    property string objSource: ""
    property string textureSource: ""
    property string generationErrorMessage: ""

    function ensureObjGenerated() {
        if (!customerID || groupID <= 0) return
        var localObj = groupDir + "/" + defaultObjName
        if (mm3dManager.fileExists(localObj)) {
            applyObjAndTexturePaths()
            return
        }
        var left01 = groupDir + "/01_L.jpg"
        var right01 = groupDir + "/01_R.jpg"
        mm3dManager.runFaceRecon(left01, left01, right01, right01, groupDir, 0)
    }

    function applyObjAndTexturePaths() {
        var base = "file:///" + groupDir.replace(/\\/g, "/") + "/"
        root.objSource = base + defaultObjName
        root.textureSource = base + defaultTexName
    }

    function normalizedPath(p) {
        if (!p) return ""
        return String(p).replace(/\\/g, "/").replace(/\/+$/, "")
    }

    onIs3DViewActiveChanged: {
        if (is3DViewActive)
            Qt.callLater(ensureObjGenerated)
    }

    onGroupIDChanged: {
        root.objSource = ""
        root.textureSource = ""
        root.generationErrorMessage = ""
        if (root.is3DViewActive)
            Qt.callLater(ensureObjGenerated)
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
                anchors.margins: 8
                spacing: 6
                Text {
                    width: parent.width - 16
                    text: "当前 Group 下 3D 自动生成（01_L-01_R.obj）"
                    color: "#888"
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                }
                ListView {
                    id: subListView3D
                    width: parent.width - 16
                    height: parent.height - 30
                    model: root.subphotoes
                    clip: true
                    spacing: 10

                    delegate: Rectangle {
                        width: subListView3D.width - 4
                        height: 135
                        radius: 6
                        color: "#2a2a2e"
                        border.color: "#444"
                        border.width: 1

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
                    }
                }
            }
        }
    }
}
