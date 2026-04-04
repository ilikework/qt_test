import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D 6.7
import QtQuick3D.Helpers
import QtQuick3D.AssetUtils
import "components"

// 3D 人脸/模型：每个 Group 用左右图（如 01_L.jpg + 01_R.jpg）生成。
// 贴图命名约定：{左图文件名不含扩展名}-{右图文件名不含扩展名}.png，例如 01_L-01_R.png；obj 一般为 01_L-01_R.obj。
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

    /// 变脸模式：左侧图与右侧图可分别选择，贴图按比例合成
    property bool morphMode: false
    /// 变脸模式下选中的“左半脸”贴图索引
    property int selectedLeftIndex: 0
    /// 变脸模式下选中的“右半脸”贴图索引
    property int selectedRightIndex: 0
    /// 变脸左右分界固定 50:50（不调）
    readonly property real morphLeftRatio: 0.5
    /// 变脸：局部轴 0=X，1=Y，2=Z（Y 用右侧竖滑条；Z 用滚轮）
    property int morphBlendAxis: 0
    /// 局部坐标 → 0~1：coord = clamp(axis * scale + bias, 0, 1)；轴缩放固定为原滑条最大值 20，不可调
    readonly property real morphMeshScale: 20.0
    property real morphMeshBias: 0.0
    /// 轴偏移：局部 X ±15，局部 Y 约 ±19~17，局部 Z -11~11
    readonly property real morphBiasMin: morphBlendAxis === 0 ? -15.0
        : (morphBlendAxis === 1 ? -19.0 : -11.0)
    readonly property real morphBiasMax: morphBlendAxis === 0 ? 15.0
        : (morphBlendAxis === 1 ? 17.0 : 11.0)
    /// Z 轴：滚轮每「刻度」步进（angleDelta 按 120 归一）
    property real morphZWheelStep: 0.12
    /// 接缝柔化固定为最小，不可调
    readonly property real morphBlendFeather: 0.012
    /// 与 CustomMaterial uEmissiveFactor 同源：普通模式与变脸共用
    property vector3d skinEmissiveFactor: Qt.vector3d(1, 1, 1)
    /// Principled：自发光相对贴图强度（略低于 1，留出漫反射/高光让方向光可见）
    property real skinPrincipledEmissiveToBaseRatio: 0.42
    readonly property vector3d skinPrincipledEmissiveScaled: Qt.vector3d(
        skinEmissiveFactor.x * skinPrincipledEmissiveToBaseRatio,
        skinEmissiveFactor.y * skinPrincipledEmissiveToBaseRatio,
        skinEmissiveFactor.z * skinPrincipledEmissiveToBaseRatio)
    /// Unshaded：1=片元内手动 sRGB→线性（贴图工作流正确）；0=假定采样已是线性
    property real morphUnshadedAssumeSrgbTexture: 1.0
    /// Unshaded 线性段整体压暗，贴近 Principled（略提亮以接近普通模式观感）
    property real morphUnshadedPrincipledMatchGain: 0.88
    /// 色度放大（1=不变）；须配合片内公式 luma+chroma*boost，不会误提亮
    property real morphUnshadedSaturationBoost: 1.06
    /// 1=片内再乘 sceneEnvironment.exposure（易与全局重复变亮）；默认 0
    property real morphUnshadedApplySceneExposure: 0.0
    /// 变脸 lit 材质走引擎默认受光；以下仍保留供以后扩展或调场景
    property real morphShadedLightingGain: 5.0 //3.65
    /// 仅乘在 AMBIENT_LIGHT 的 scene 项上：越小旋转灯越「主导」明暗
    property real morphShadedAmbientFactor: 0.20 //0.52
    /// 半球底亮（与 TOTAL_AMBIENT_COLOR 无关）：避免无 IBL 时整模全黑；略大则旋转感略弱
    property real morphShadedHemisphereFill: 0.1 // 0.30
    /// 变脸+3D打光：场景方向光补亮（片内另有水平模拟光）
    property real morphShadedSunBrightness: 0.1
    /// 与 3DDemo material.frag 一致：开灯 albedo 色度/暖色微调
    property real litChromaBoost: 1.08
    property vector3d litWarmTint: Qt.vector3d(1.04, 1.0, 0.98)
    /// 3D 打光：水平光源方位角（度），-90=左侧射向模型，0=正面，90=右侧
    property real lightYawDeg: 0
    /// 3D 打光：片内模拟方向光强度（与 simLightBoost 相乘；滑条调主观明暗）
    property real light3DBrightness: 3.15
    /// 片内模拟光总倍率（默认 10 ≈ 相对未乘前「至少亮一个数量级」，对齐旧 Principled+DirectionalLight）
    property real simLightBoost: 10.0
    /// 无打光时写入 DIFFUSE 的系数（一般为 1；偏暗/偏亮可微调）
    property real morphFlatUnlitLevel: 1.0

    property string objSource: ""
    property string textureSource: ""
    property string generationErrorMessage: ""

    /// 变脸左右贴图 URL：优先按列表解析；subphotoes 未就绪或解析为空时回退 textureSource（避免 morph 左右 URL 为空、采样黑图）
    readonly property string leftTextureSource: morphMode ? ((((subphotoes && selectedLeftIndex >= 0 && selectedLeftIndex < subphotoes.length) ? textureUrlForIndex(selectedLeftIndex) : "") || textureSource)) : ""
    readonly property string rightTextureSource: morphMode ? ((((subphotoes && selectedRightIndex >= 0 && selectedRightIndex < subphotoes.length) ? textureUrlForIndex(selectedRightIndex) : "") || textureSource)) : ""

    /// true = 无人操作，obj 左右摆动；false = 用户正在操作，停止摆动
    property bool swingIdle: true
    /// 人脸中心轴 ±90° 半周期时长（ms）；相对原 4000ms 摆动角速度降低约 30%（时长 ÷0.7）
    readonly property int swingHalfCycleMs: 5714

    /// true = 使用同级目录 01_L-01_R_relief.png 作为贴图（3D模型按钮）
    property bool reliefTextureMode: false
    /// true = 启用片内 3D 打光；false = 平面显示（等同原 Unshaded）
    property bool rotatingLight3DEnabled: false
    /// 为 false 时关闭 [MM3D] 材质刷新日志（默认开启，便于在「应用程序输出」里看到）
    property bool mm3dDebugLogs: true

    Component.onCompleted: {
        console.warn("[MM3D] MM3DViewer 已加载；材质调试日志 mm3dDebugLogs=", mm3dDebugLogs,
                     "（若看不到本行，请检查 Qt Creator「应用程序输出」或 qml 控制台是否被过滤）")
    }

    function refresh3DMaterial() {
        if (importNode.status !== RuntimeLoader.Success) {
            if (mm3dDebugLogs)
                console.warn("[MM3D] refresh3DMaterial skipped: RuntimeLoader status=", importNode.status)
            return
        }
        if (mm3dDebugLogs) {
            console.warn("[MM3D] refresh3DMaterial morph=", morphMode, "bright=", light3DBrightness,
                         "light3D=", rotatingLight3DEnabled)
        }
        importNode.applyMaterialRecursively(importNode)
    }

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

    /// 根据 subphotoes 得到贴图主名：左图基名 + "-" + 右图基名（与 01_L-01_R.png 中 "01_L-01_R" 一致）
    function getTextureStem(index) {
        if (!subphotoes || index < 0 || index >= subphotoes.length) return ""
        var leftPath = String(subphotoes[index].photoL || "")
        var rightPath = String(subphotoes[index].photoR || "")
        leftPath = leftPath.replace(/^file:\/\/\//, "").replace(/\\/g, "/")
        rightPath = rightPath.replace(/^file:\/\/\//, "").replace(/\\/g, "/")
        var leftName = leftPath.split("/").pop().replace(/\.[^.]*$/, "")
        var rightName = rightPath.split("/").pop().replace(/\.[^.]*$/, "")
        if (!leftName || !rightName) return ""
        return leftName + "-" + rightName
    }

    /// 在 groupDir 下解析贴图文件名：首选 {stem}.png（如 01_L-01_R.png），再尝试 jpg / 旧版 *_texture.* 等
    function resolveTextureFileNameForIndex(index) {
        var stem = getTextureStem(index)
        var dir = groupDir.replace(/\\/g, "/")
        var candidates = []
        if (stem) {
            candidates = [
                stem + ".png",
                stem + ".jpg",
                stem + ".jpeg",
                stem + "_texture.png",
                stem + "_texture.jpg",
                stem + "_texture.jpeg"
            ]
        }
        // 无 subphotoes 或 stem 推导失败时，首组仍可尝试目录内常见名
        if (index === 0) {
            candidates.push("01_L-01_R.png", "01_L-01_R.jpg", "01_L-01_R.jpeg")
            candidates.push("01_L-01_R_texture.jpg", "01_L-01_R_texture.png", "01_L-01_R_texture.jpeg")
        }
        if (candidates.length === 0)
            return ""
        for (var i = 0; i < candidates.length; i++) {
            var full = dir + "/" + candidates[i]
            if (mm3dManager.fileExists(full))
                return candidates[i]
        }
        console.warn("[MM3D] 未找到贴图，已尝试:", stem, "… 于", dir)
        return ""
    }

    function textureUrlForIndex(index) {
        var rel = resolveTextureFileNameForIndex(index)
        if (!rel) return ""
        return "file:///" + groupDir.replace(/\\/g, "/") + "/" + rel
    }

    function applyObjAndTexturePaths() {
        var base = "file:///" + groupDir.replace(/\\/g, "/") + "/"
        root.objSource = base + defaultObjName
        if (root.reliefTextureMode) {
            var reliefRel = "01_L-01_R_relief.png"
            var reliefFull = groupDir.replace(/\\/g, "/") + "/" + reliefRel
            if (mm3dManager.fileExists(reliefFull))
                root.textureSource = base + reliefRel
            else
                root.textureSource = ""
            return
        }
        var idx = selectedTextureIndex
        if (idx < 0 || idx >= (subphotoes ? subphotoes.length : 0)) idx = 0
        var rel = resolveTextureFileNameForIndex(idx)
        root.textureSource = rel ? (base + rel) : ""
        if (root.textureSource)
            console.log("[MM3D] 贴图:", root.textureSource)
        else
            console.warn("[MM3D] 未设置贴图 URL，groupDir=", groupDir, "obj=", root.objSource)
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
        root.selectedLeftIndex = 0
        root.selectedRightIndex = 0
        root.reliefTextureMode = false
        if (root.is3DViewActive)
            Qt.callLater(ensureObjGenerated)
    }

    onMorphModeChanged: {
        console.log("[MM3D] morphMode changed:", morphMode)
        if (morphMode) {
            if (reliefTextureMode)
                reliefTextureMode = false
            selectedLeftIndex = selectedTextureIndex
            selectedRightIndex = selectedTextureIndex
        }
        console.log("[MM3D] morphMode on → leftTextureSource:", leftTextureSource, "rightTextureSource:", rightTextureSource, "fallback textureSource:", textureSource)
        Qt.callLater(refresh3DMaterial)
    }

    onSelectedLeftIndexChanged: {
        if (morphMode)
            Qt.callLater(refresh3DMaterial)
    }
    onSelectedRightIndexChanged: {
        if (morphMode)
            Qt.callLater(refresh3DMaterial)
    }

    onMorphMeshBiasChanged: {
        if (morphMode)
            Qt.callLater(refresh3DMaterial)
    }

    onMorphBlendAxisChanged: {
        if (morphBlendAxis < 0)
            morphBlendAxis = 0
        else if (morphBlendAxis > 2)
            morphBlendAxis = 2
        if (morphBlendAxis === 2) {
            morphMeshBias = 11
        } else {
            if (morphMeshBias < morphBiasMin) morphMeshBias = morphBiasMin
            else if (morphMeshBias > morphBiasMax) morphMeshBias = morphBiasMax
        }
        if (morphMode)
            Qt.callLater(refresh3DMaterial)
    }

    onRotatingLight3DEnabledChanged: Qt.callLater(refresh3DMaterial)
    onMorphFlatUnlitLevelChanged: Qt.callLater(refresh3DMaterial)
    onSimLightBoostChanged: Qt.callLater(refresh3DMaterial)

    onSelectedTextureIndexChanged: {
        if (root.reliefTextureMode) return
        if (!root.objSource) return
        var base = "file:///" + groupDir.replace(/\\/g, "/") + "/"
        var rel = resolveTextureFileNameForIndex(selectedTextureIndex)
        root.textureSource = rel ? (base + rel) : ""
    }

    onReliefTextureModeChanged: {
        if (!root.objSource) return
        var base = "file:///" + groupDir.replace(/\\/g, "/") + "/"
        if (reliefTextureMode)
            root.textureSource = base + "01_L-01_R_relief.png"
        else {
            var rel = resolveTextureFileNameForIndex(selectedTextureIndex)
            root.textureSource = rel ? (base + rel) : ""
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

                // 曝光/色调映射与 Unshaded 里 uSceneExposure、uTonemapFlavor 对齐（默认 Linear + exposure 1）
                environment: ExtendedSceneEnvironment {
                    id: sceneEnvironment
                    antialiasingMode: SceneEnvironment.MSAA
                    antialiasingQuality: SceneEnvironment.High
                    backgroundMode: SceneEnvironment.Color
                    clearColor: "#202020"
                    tonemapMode: SceneEnvironment.TonemapModeLinear
                    exposure: 1.0
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
                    color: "#fff5ed"
                    // 3D 开灯：暖补光（与 3DDemo 一致）；单张贴图与变脸均走 CustomMaterial lit
                    brightness: root.rotatingLight3DEnabled ? root.morphShadedSunBrightness : 0.0
                }

                // 主光：平行光，方向 = 父节点 Y(-lightYawDeg，与滑条「左负右正」观感一致) + 子节点俯仰 -45°
                Node {
                    id: keyLightPivot
                    position: Qt.vector3d(0, 0, 0)
                    eulerRotation: Qt.vector3d(0, -root.lightYawDeg, 0)
                    DirectionalLight {
                        id: keyDirectionalLight
                        eulerRotation: Qt.vector3d(-45, 0, 0)
                        color: "#fff6f0"
                        brightness: root.rotatingLight3DEnabled
                            ? Math.min(3.0, Math.max(0.0, root.light3DBrightness * 0.28))
                            : 0.0
                        castsShadow: false
                    }
                }

                Node { id: originNode; position: Qt.vector3d(0, 0, 0) }

                Texture {
                    id: morphTexLeft
                    source: root.morphMode ? root.leftTextureSource : root.textureSource
                    generateMipmaps: false
                    minFilter: Texture.Linear
                    mipFilter: Texture.None
                    magFilter: Texture.Linear
                    tilingModeHorizontal: Texture.ClampToEdge
                    tilingModeVertical: Texture.ClampToEdge
                }
                Texture {
                    id: morphTexRight
                    source: root.morphMode ? root.rightTextureSource : root.textureSource
                    generateMipmaps: false
                    minFilter: Texture.Linear
                    mipFilter: Texture.None
                    magFilter: Texture.Linear
                    tilingModeHorizontal: Texture.ClampToEdge
                    tilingModeVertical: Texture.ClampToEdge
                }

                // 变脸 + 3D：morph_blend_lit.frag（MAIN + DIRECTIONAL_LIGHT + 与 3DDemo 一致的 albedo 暖调）
                CustomMaterial {
                    id: skinMorphMatLit
                    cullMode: CustomMaterial.NoCulling
                    shadingMode: CustomMaterial.Shaded
                    vertexShader: "qrc:/morph_blend.vert"
                    fragmentShader: "qrc:/morph_blend_lit.frag"
                    property TextureInput leftTex: TextureInput {
                        texture: morphTexLeft
                    }
                    property TextureInput rightTex: TextureInput {
                        texture: morphTexRight
                    }
                    property real uLeftRatio: root.morphLeftRatio
                    property real uUseMeshAxis: 1.0
                    property int uBlendAxis: root.morphBlendAxis
                    property real uMeshScale: root.morphMeshScale
                    property real uMeshBias: -root.morphMeshBias
                    property real uFeather: root.morphBlendFeather
                    property vector3d uEmissiveFactor: root.skinPrincipledEmissiveScaled
                    property real uSceneExposure: sceneEnvironment.exposure
                    property real uAssumeSrgbTexture: root.morphUnshadedAssumeSrgbTexture
                    property real uApplySceneExposure: root.morphUnshadedApplySceneExposure
                    property real uLitChromaBoost: root.litChromaBoost
                    property vector3d uLitWarmTint: root.litWarmTint
                }

                // 变脸、关 3D：Unshaded 平面
                CustomMaterial {
                    id: skinMorphMatUnlit
                    cullMode: CustomMaterial.NoCulling
                    shadingMode: CustomMaterial.Unshaded
                    vertexShader: "qrc:/morph_blend.vert"
                    fragmentShader: "qrc:/morph_blend_unlit.frag"
                    property TextureInput leftTex: TextureInput {
                        texture: morphTexLeft
                    }
                    property TextureInput rightTex: TextureInput {
                        texture: morphTexRight
                    }
                    property real uLeftRatio: root.morphLeftRatio
                    property real uUseMeshAxis: 1.0
                    property int uBlendAxis: root.morphBlendAxis
                    property real uMeshScale: root.morphMeshScale
                    property real uMeshBias: -root.morphMeshBias
                    property real uFeather: root.morphBlendFeather
                    property vector3d uEmissiveFactor: root.skinPrincipledEmissiveScaled
                    property real uSceneExposure: sceneEnvironment.exposure
                    property int uTonemapFlavor: sceneEnvironment.tonemapMode === SceneEnvironment.TonemapModeAces ? 1 : 0
                    property real uAssumeSrgbTexture: root.morphUnshadedAssumeSrgbTexture
                    property real uPrincipledMatchGain: root.morphUnshadedPrincipledMatchGain
                    property real uUnshadedSaturationBoost: root.morphUnshadedSaturationBoost
                    property real uApplySceneExposure: root.morphUnshadedApplySceneExposure
                    property real uEncodeSrgbToDisplay: 0.0
                }

                RuntimeLoader {
                    id: importNode
                    parent: originNode
                    source: root.objSource || ""

                    onStatusChanged: {
                        console.log("[MM3D] RuntimeLoader status:", status, "objSource:", root.objSource, "textureSource:", root.textureSource)
                        if (status === RuntimeLoader.Success) {
                            importNode.scale = Qt.vector3d(1, 1, 1)
                            applyMaterialRecursively(importNode)
                        }
                    }

                    function applyMaterialRecursively(node) {
                        if (!node) return
                        if ("materials" in node) {
                            var matPick = root.rotatingLight3DEnabled ? skinMorphMatLit : skinMorphMatUnlit
                            node.materials = []
                            node.materials = [ matPick ]
                            if (root.mm3dDebugLogs) {
                                console.warn("[MM3D] applyMaterialRecursively mat=",
                                             root.rotatingLight3DEnabled ? "skin lit" : "skin unlit",
                                             "morph=", root.morphMode, "light3D=", root.rotatingLight3DEnabled,
                                             "bright=", root.light3DBrightness)
                            }
                        }
                        var kids = node.children
                        for (var i = 0; i < kids.length; ++i) applyMaterialRecursively(kids[i])
                    }
                }

                Connections {
                    target: root
                    function onTextureSourceChanged() {
                        if (importNode.status === RuntimeLoader.Success)
                            Qt.callLater(root.refresh3DMaterial)
                    }
                    function onSwingIdleChanged() {
                        if (!root.swingIdle) {
                            originNode.eulerRotation = Qt.vector3d(0, 0, 0)
                            importNode.eulerRotation = Qt.vector3d(0, 0, 0)
                        }
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
                        duration: root.swingHalfCycleMs
                        easing.type: Easing.InOutSine
                    }
                    NumberAnimation {
                        target: importNode
                        property: "eulerRotation.y"
                        from: 90
                        to: -90
                        duration: root.swingHalfCycleMs
                        easing.type: Easing.InOutSine
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

            /// Z 轴：变脸模式下在主画面上用滚轮调节偏移（覆盖在 View3D 上，避免与相机轨道抢滚轮）
            Item {
                anchors.fill: parent
                visible: root.morphMode && root.morphBlendAxis === 2
                z: 103
                WheelHandler {
                    onWheel: function (event) {
                        var dy = event.angleDelta.y / 120.0
                        var nb = root.morphMeshBias - dy * root.morphZWheelStep
                        if (nb < root.morphBiasMin)
                            nb = root.morphBiasMin
                        if (nb > root.morphBiasMax)
                            nb = root.morphBiasMax
                        root.morphMeshBias = nb
                    }
                }
            }

            /// Y 轴偏移：主画面右侧竖条，拖动方向与屏幕上下一致（底栏之上）
            Item {
                id: morphYSliderPanel
                visible: root.morphMode && root.morphBlendAxis === 1
                z: 101
                width: 56
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.bottom: view3dBottomOverlay.top
                anchors.bottomMargin: 6

                Rectangle {
                    anchors.fill: parent
                    color: "#66000000"
                    radius: 4
                    border.color: "#40ffffff"
                    border.width: 1
                }
                Label {
                    id: morphYValueLabel
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 6
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    text: root.morphMeshBias.toFixed(2)
                    color: "#ccc"
                    font.pixelSize: 11
                }
                Slider {
                    orientation: Qt.Vertical
                    anchors.top: parent.top
                    anchors.topMargin: 8
                    anchors.bottom: morphYValueLabel.top
                    anchors.bottomMargin: 4
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 36
                    from: root.morphBiasMin
                    to: root.morphBiasMax
                    stepSize: 0.02
                    /// 与模型局部 Y 分界方向对齐：滑条上推对应画面上移（相对原颠倒映射）
                    value: root.morphBiasMin + root.morphBiasMax - root.morphMeshBias
                    onMoved: root.morphMeshBias = root.morphBiasMin + root.morphBiasMax - value
                }
            }

            /// 主画面底部：变脸控件在上，3D 光源角度/亮度在最下（需在 MouseArea 之上才能拖动滑条）
            Column {
                id: view3dBottomOverlay
                z: 100
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 8
                spacing: 8
                width: parent.width - 16

                /// 变脸：图标按钮循环 X→Y→Z；X 底栏横滑条，Y 右侧竖滑条，Z 主画面滚轮
                Column {
                    width: parent.width
                    visible: root.morphMode
                    spacing: 6
                    RowLayout {
                        width: parent.width
                        spacing: 6
                        ToolButton {
                            id: morphAxisBtn
                            implicitWidth: 44
                            implicitHeight: 44
                            padding: 4
                            flat: true
                            display: AbstractButton.IconOnly
                            hoverEnabled: true
                            ToolTip.visible: hovered
                            ToolTip.delay: 400
                            ToolTip.text: root.morphBlendAxis === 0 ? "分界轴 X（点按→Y）"
                                          : (root.morphBlendAxis === 1 ? "分界轴 Y（点按→Z）" : "分界轴 Z（点按→X）")
                            onClicked: root.morphBlendAxis = (root.morphBlendAxis + 1) % 3
                            /// 图1 左右分界 / 图2 上下分界 / 图3 中心圆点（内嵌矢量，不依赖外部 png）
                            contentItem: Item {
                                implicitWidth: 36
                                implicitHeight: 36
                                Rectangle {
                                    anchors.fill: parent
                                    radius: 3
                                    color: "#b8b8b8"
                                    border.width: 1
                                    border.color: "#5a5a5a"
                                }
                                Rectangle {
                                    anchors.fill: parent
                                    anchors.margins: 4
                                    color: "#a8a8a8"
                                    border.width: 1
                                    border.color: "#1a1a1a"
                                    Item {
                                        anchors.centerIn: parent
                                        width: 22
                                        height: 22
                                        Row {
                                            visible: root.morphBlendAxis === 0
                                            anchors.centerIn: parent
                                            spacing: 0
                                            Rectangle { width: 11; height: 22; color: "#252525" }
                                            Rectangle { width: 11; height: 22; color: "#e0e0e0" }
                                        }
                                        Column {
                                            visible: root.morphBlendAxis === 1
                                            anchors.centerIn: parent
                                            spacing: 0
                                            Rectangle { width: 22; height: 11; color: "#e8e8e8" }
                                            Rectangle { width: 22; height: 11; color: "#252525" }
                                        }
                                        Item {
                                            visible: root.morphBlendAxis === 2
                                            anchors.centerIn: parent
                                            width: 22
                                            height: 22
                                            Rectangle {
                                                anchors.fill: parent
                                                color: "#d8d8d8"
                                            }
                                            Rectangle {
                                                width: 8
                                                height: 8
                                                radius: 4
                                                anchors.centerIn: parent
                                                color: "#1a1a1a"
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        Slider {
                            Layout.fillWidth: true
                            visible: root.morphBlendAxis === 0
                            from: root.morphBiasMin
                            to: root.morphBiasMax
                            stepSize: 0.02
                            value: root.morphMeshBias
                            onMoved: root.morphMeshBias = value
                        }
                        Item {
                            Layout.fillWidth: true
                            visible: root.morphBlendAxis === 2
                        }
                        Label {
                            visible: root.morphBlendAxis === 0 || root.morphBlendAxis === 2
                            text: root.morphBlendAxis === 2
                                  ? (root.morphMeshBias.toFixed(2) + " ·滚轮")
                                  : root.morphMeshBias.toFixed(2)
                            color: "#ccc"
                            font.pixelSize: 11
                            Layout.minimumWidth: root.morphBlendAxis === 2 ? 64 : 40
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: light3dPanelCol.implicitHeight + 16
                    visible: root.rotatingLight3DEnabled
                    color: "#99000000"
                    radius: 6
                    border.color: "#40ffffff"
                    border.width: 1
                    Column {
                        id: light3dPanelCol
                        x: 8
                        y: 8
                        width: parent.width - 16
                        spacing: 6
                        Label {
                            width: parent.width
                            text: "3D 光源（水平角：左侧 -90° · 正面 0° · 右侧 90°）"
                            color: "#ccc"
                            font.pixelSize: 11
                        }
                        RowLayout {
                            width: parent.width
                            Slider {
                                Layout.fillWidth: true
                                from: -90
                                to: 90
                                stepSize: 1
                                live: true
                                value: root.lightYawDeg
                                onMoved: root.lightYawDeg = value
                            }
                            Label {
                                text: root.lightYawDeg.toFixed(0) + "°"
                                color: "#eee"
                                font.pixelSize: 12
                                Layout.minimumWidth: 40
                            }
                        }
                        RowLayout {
                            width: parent.width
                            Label {
                                text: "亮度"
                                color: "#aaa"
                                font.pixelSize: 11
                                Layout.minimumWidth: 40
                            }
                            Slider {
                                Layout.fillWidth: true
                                from: 0.2
                                to: 5.0
                                stepSize: 0.05
                                live: true
                                value: root.light3DBrightness
                                onMoved: root.light3DBrightness = value
                            }
                            Label {
                                text: root.light3DBrightness.toFixed(2)
                                color: "#eee"
                                font.pixelSize: 12
                                Layout.minimumWidth: 48
                            }
                        }
                    }
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
            Layout.preferredWidth: 162
            Layout.fillHeight: true
            color: "#1e1e1e"
            radius: 8
            border.color: "#333"

            Column {
                anchors.fill: parent
                anchors.margins: 4
                spacing: 4

                Item {
                    width: parent.width
                    height: 40
                    CheckButton {
                        id: morphBtn
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: root.morphMode ? "退出变脸" : "变脸"
                        checked: root.morphMode
                        onClicked: root.morphMode = !root.morphMode
                    }
                    Binding {
                        target: morphBtn
                        property: "checked"
                        value: root.morphMode
                    }
                }

                Item {
                    width: parent.width
                    height: 40
                    CheckButton {
                        id: reliefBtn
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: root.reliefTextureMode ? "退出模型" : "3D模型"
                        checked: root.reliefTextureMode
                        onClicked: {
                            if (root.morphMode)
                                root.morphMode = false
                            root.reliefTextureMode = !root.reliefTextureMode
                        }
                    }
                    Binding {
                        target: reliefBtn
                        property: "checked"
                        value: root.reliefTextureMode
                    }
                }

                Item {
                    width: parent.width
                    height: 40
                    CheckButton {
                        id: light3dBtn
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: root.rotatingLight3DEnabled ? "关灯" : "开灯"
                        checked: root.rotatingLight3DEnabled
                        onClicked: root.rotatingLight3DEnabled = !root.rotatingLight3DEnabled
                    }
                    Binding {
                        target: light3dBtn
                        property: "checked"
                        value: root.rotatingLight3DEnabled
                    }
                }

                Item {
                    width: parent.width - 2
                    height: parent.height - 136
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
                            width: 145
                            height: 100
                            anchors.horizontalCenter: parent.horizontalCenter
                            radius: 6
                            property bool hovered: false
                            color: (!root.morphMode && root.selectedTextureIndex === index) ? "#3a3a4e" : "#2a2a2e"
                            border.color: hovered ? "#888" : (!root.morphMode && root.selectedTextureIndex === index ? "#ffb300" : "#444")
                            border.width: (!root.morphMode && root.selectedTextureIndex === index) ? 2 : 1

                            Row {
                                anchors.centerIn: parent
                                spacing: 2
                                Rectangle {
                                    width: 67
                                    height: 89
                                    radius: 2
                                    color: "transparent"
                                    border.color: root.morphMode && root.selectedLeftIndex === index ? "#ffb300" : "transparent"
                                    border.width: root.morphMode && root.selectedLeftIndex === index ? 2 : 0
                                    Image {
                                        anchors.centerIn: parent
                                        width: 65
                                        height: 87
                                        source: modelData.photoL
                                        fillMode: Image.PreserveAspectFit
                                        sourceSize.width: 65
                                        sourceSize.height: 87
                                        asynchronous: true
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        visible: root.morphMode
                                        onClicked: root.selectedLeftIndex = index
                                    }
                                }
                                Rectangle {
                                    width: 67
                                    height: 89
                                    radius: 2
                                    color: "transparent"
                                    border.color: root.morphMode && root.selectedRightIndex === index ? "#ffb300" : "transparent"
                                    border.width: root.morphMode && root.selectedRightIndex === index ? 2 : 0
                                    Image {
                                        anchors.centerIn: parent
                                        width: 65
                                        height: 87
                                        source: modelData.photoR
                                        fillMode: Image.PreserveAspectFit
                                        sourceSize.width: 65
                                        sourceSize.height: 87
                                        asynchronous: true
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        visible: root.morphMode
                                        onClicked: root.selectedRightIndex = index
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                hoverEnabled: true
                                visible: !root.morphMode
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
