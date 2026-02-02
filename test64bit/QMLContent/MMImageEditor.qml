import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Basic
import MyCustomComponents 1.0
import "components"

Item {
    id: root
    property alias source: internalEditor.source
    property alias penColor: internalEditor.penColor
    property alias penWidth: internalEditor.penWidth

    enum EditMode { None, View, Line, Circle, Eraser, ShowSmooth, EditSmooth }
    property int currentMode: MMImageEditor.EditMode.None
    property real minScale: 1.0
    property var points: []
    // 用于手动拖拽的坐标记录
    property point lastMousePos: Qt.point(0, 0)
    property int selectedItemIx: -1   // 存储当前正在编辑的 Item 的索引 (C++ 数组索引)
    property int selectedPointIdx: -1  // 存储当前正在拖拽的点的索引

    function init(ix,dir) { internalEditor.init(ix,dir); }

    function fitToWindow() {
        if (internalEditor.implicitWidth > 0 && internalEditor.implicitHeight > 0) {
            let scaleX = root.width / internalEditor.implicitWidth
            let scaleY = root.height / internalEditor.implicitHeight
            // 初始缩放比例，使图片完整显示在窗口内
            minScale = Math.min(scaleX, scaleY)
            container.scale = minScale

            // 居中显示
            flick.contentX = Math.max(0, (container.width * minScale - flick.width) / 2)
            flick.contentY = Math.max(0, (container.height * minScale - flick.height) / 2)
        }
    }

    function applyZoomCentered(factor) {
        let oldScale = container.scale
        let newScale = Math.max(minScale, Math.min(oldScale * factor, 1.0))
        if (Math.abs(newScale - oldScale) < 0.0001) return

        let centerXInImage = (flick.contentX + flick.width / 2) / oldScale
        let centerYInImage = (flick.contentY + flick.height / 2) / oldScale

        container.scale = newScale
        flick.contentX = centerXInImage * newScale - flick.width / 2
        flick.contentY = centerYInImage * newScale - flick.height / 2
    }

    function applyZoomAtMouse(factor, mouseX, mouseY) {
        let oldScale = container.scale
        let newScale = Math.max(minScale, Math.min(oldScale * factor, 4.0))
        if (Math.abs(newScale - oldScale) < 0.0001) return

        // 1. 记录缩放前的状态
        // 关键：暂时关闭 interactive 避免 Flickable 内部逻辑干扰坐标补偿
        let wasInteractive = flick.interactive
        flick.interactive = false

        // 2. 计算鼠标点在原始图像（scale=1.0）时的逻辑坐标
        let logicX = (flick.contentX + mouseX) / oldScale
        let logicY = (flick.contentY + mouseY) / oldScale

        // 3. 应用新缩放
        container.scale = newScale

        // 4. 强制补偿 contentX/Y
        // 计算缩放后，该逻辑点在新的 content 尺寸下应该在的位置
        let targetX = logicX * newScale - mouseX
        let targetY = logicY * newScale - mouseY

        // 5. 手动进行边界限定，防止“露白”
        flick.contentX = Math.max(0, Math.min(targetX, flick.contentWidth - flick.width))
        flick.contentY = Math.max(0, Math.min(targetY, flick.contentHeight - flick.height))

        // 6. 恢复交互状态
        flick.interactive = wasInteractive
    }

    function calculateCircle(p1, p2, p3) {
        let x1 = p1.x, y1 = p1.y, x2 = p2.x, y2 = p2.y, x3 = p3.x, y3 = p3.y;
        let denom = 2 * (x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2);
        if (Math.abs(denom) < 0.001) return null;
        let h = ((x1**2 + y1**2) * (y2 - y3) + (x2**2 + y2**2) * (y3 - y1) + (x3**2 + y3**2) * (y1 - y2)) / denom;
        let k = ((x1**2 + y1**2) * (x3 - x2) + (x2**2 + y2**2) * (x1 - x3) + (x3**2 + y3**2) * (x2 - x1)) / denom;
        let r = Math.sqrt((x1 - h)**2 + (y1 - k)**2);
        return { x: h, y: k, r: r };
    }


    // --- 1. 静态感应区 ---
    // 放在底层，不随工具栏移动，避免因为 UI 元素移动导致的 containsMouse 状态震荡
    // MouseArea {
    //     id: mouseDetector
    //     anchors.top: parent.top
    //     anchors.horizontalCenter: parent.horizontalCenter
    //     width: toolbarRect.width + 100 // 比工具栏稍宽
    //     height: 60 // 覆盖工具栏滑出的整个区域
    //     hoverEnabled: true
    //     onPressed: (mouse) => mouse.accepted = (root.currentMode !== MMImageEditor.EditMode.None)
    //     z: 99 // 确保在顶层
    // }
    // 优化后的感应逻辑：使用专门的静态感应区，不随工具栏移动
    //readonly property bool isHeaderHovered: mouseDetector.containsMouse //|| toolbarMA.containsMouse

    // --- 2. 悬浮工具栏 ---
    Item {
        id: headerContainer
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: 400
        height: 100 // 感应高度
        z: 999

        // 静态感应逻辑
        MouseArea {
            id: staticSensor
            anchors.fill: parent
            hoverEnabled: true
            // 拦截所有进入此区域的点击，防止点在按钮间隙时画线
            onPressed: (mouse) => mouse.accepted = true
        }

        Rectangle {
            id: toolbarRect
            width: 532; height: 56
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 15
            color: "#F21A1A1E"
            radius: 12; border.color: "#444"

            // 只有鼠标进入感应区才显示，不使用 topMargin 动画避免抖动
            visible: staticSensor.containsMouse

            // MouseArea {
            //     id: toolbarMA
            //     anchors.fill: parent
            //     hoverEnabled: true
            // }

            ButtonGroup { id: toolGroup }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 15; spacing: 8

                component ToolBtn : Button {
                    property int mode: 0
                    property string btnLabel: ""
                    Layout.fillHeight: true; Layout.preferredWidth: 60
                    checkable: true
                    ButtonGroup.group: toolGroup
                    checked: root.currentMode === mode
                    onClicked: root.currentMode = mode
                    hoverEnabled: false
                    background: Rectangle {
                        color: parent.checked ? "#2200CCFF" : (parent.hovered ? "#11FFFFFF" : "transparent")
                        radius: 8
                    }
                    contentItem: Column {
                        spacing: 2
                        Item {
                            width: 24; height: 24; anchors.horizontalCenter: parent.horizontalCenter
                            ModeIcon {
                                mode: parent.parent.parent.mode
                                iconColor: parent.parent.parent.checked ? "#00CCFF" : "white"
                            }
                        }
                        Text {
                            text: parent.parent.btnLabel
                            font.pixelSize: 10; color: parent.parent.checked ? "#00CCFF" : "#888"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }

                ToolBtn { mode: MMImageEditor.EditMode.View; btnLabel: "平移" }
                ToolBtn { mode: MMImageEditor.EditMode.Line; btnLabel: "测量" }
                ToolBtn { mode: MMImageEditor.EditMode.Circle; btnLabel: "画圆" }
                ToolBtn { mode: MMImageEditor.EditMode.Eraser; btnLabel: "橡皮擦" }
                ToolBtn { mode: MMImageEditor.EditMode.ShowSmooth; btnLabel: "显示轮廓" ; onClicked: internalEditor.shapesVisible = true }
                ToolBtn { id: editBtn;  mode: MMImageEditor.EditMode.EditSmooth; btnLabel: "精修轮廓"; onClicked:  internalEditor.setAllItemsEditMode(true) }
                // 重置按钮：点击后进入 None 模式（不显示图形）
                ToolBtn { mode: MMImageEditor.EditMode.None; btnLabel: "隐藏" }
            }
        }
    }

    Flickable {
        id: flick
        anchors.fill: parent
        contentWidth: container.width * container.scale
        contentHeight: container.height * container.scale
        clip: true
        // 只有平移模式可拖拽
        //interactive: root.currentMode === MMImageEditor.EditMode.View
        interactive: false
        boundsBehavior: Flickable.StopAtBounds

        Item {
            id: container
            width: internalEditor.implicitWidth
            height: internalEditor.implicitHeight
            transformOrigin: Item.TopLeft

            ImageEditor {
                id: internalEditor
                anchors.fill: parent
                // 图像始终显示，但图形绘制通过 internalEditor 内部逻辑控制
                // 假设 C++ 端有此属性控制 paint 函数里的图形绘制循环
                // 如果 C++ 端没写该属性，可以先用 opacity 模拟：
                // opacity: root.currentMode === MMImageEditor.EditMode.None ? 0.99 : 1.0
                // 但最好在 C++ 中通过 setShapesVisible(bool) 控制
                shapesVisible: root.currentMode !== MMImageEditor.EditMode.None
                onImplicitWidthChanged: Qt.callLater(root.fitToWindow)
                onImplicitHeightChanged: Qt.callLater(root.fitToWindow)
            }

            // 预览层（Canvas）
            Canvas {
                id: previewCanvas
                anchors.fill: parent
                // 隐藏模式下预览层也该关闭
                visible: root.currentMode === MMImageEditor.EditMode.Line || root.currentMode === MMImageEditor.EditMode.Circle
                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);
                    if (points.length === 0) return;
                    ctx.strokeStyle = root.penColor;
                    ctx.lineWidth = root.penWidth / container.scale; // 保持线宽视觉一致
                    ctx.beginPath();
                    let p1 = points[0];
                    let mouseP = Qt.point(interactionArea.mouseX, interactionArea.mouseY);
                    if (root.currentMode === MMImageEditor.EditMode.Line) {
                        ctx.moveTo(p1.x, p1.y); ctx.lineTo(mouseP.x, mouseP.y);
                    } else if (root.currentMode === MMImageEditor.EditMode.Circle) {
                        if (points.length === 1) {
                            ctx.setLineDash([5, 5]); ctx.moveTo(p1.x, p1.y); ctx.lineTo(mouseP.x, mouseP.y);
                        } else if (points.length === 2) {
                            ctx.setLineDash([]); let circle = calculateCircle(p1, points[1], mouseP);
                            if (circle) ctx.arc(circle.x, circle.y, circle.r, 0, Math.PI * 2);
                        }
                    }
                    ctx.stroke();
                }
            }

            // MouseArea {
            //     id: drawMouseArea
            //     anchors.fill: parent
            //     hoverEnabled: true
            //     // 隐藏和查看模式下不可绘图
            //     enabled: root.currentMode !== MMImageEditor.EditMode.View && root.currentMode !== MMImageEditor.EditMode.None
            //     cursorShape: root.currentMode === MMImageEditor.EditMode.Eraser ? Qt.PointingHandCursor : Qt.CrossCursor
            //     onClicked: (mouse) => {
            //         if (root.currentMode === MMImageEditor.EditMode.Eraser) {
            //             internalEditor.eraseAt(mouse.x, mouse.y);
            //         } else {
            //             points.push(Qt.point(mouse.x, mouse.y));
            //             if (root.currentMode === MMImageEditor.EditMode.Line && points.length === 2) {
            //                 internalEditor.addLine(points[0].x, points[0].y, points[1].x, points[1].y);
            //                 points = [];
            //             } else if (root.currentMode === MMImageEditor.EditMode.Circle && points.length === 3) {
            //                 internalEditor.addCircle(points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y);
            //                 points = [];
            //             }
            //         }
            //         previewCanvas.requestPaint();
            //     }
            //     onPositionChanged: (mouse) => { if (points.length > 0) previewCanvas.requestPaint(); }
            // }
            // --- 核心：手动交互层 ---
            MouseArea {
                id: interactionArea
                anchors.fill: parent
                hoverEnabled: true
                // 只有处于“非隐藏”模式才启用
                enabled: root.currentMode !== MMImageEditor.EditMode.None

                cursorShape: {
                    if (root.currentMode === MMImageEditor.EditMode.View) return Qt.OpenHandCursor;
                    if (root.currentMode === MMImageEditor.EditMode.Eraser) return Qt.PointingHandCursor;
                    return Qt.CrossCursor;
                }

                onPressed: (mouse) => {
                    if (root.currentMode === MMImageEditor.EditMode.EditSmooth) {
                       // 1. 调用 C++ 检测是否点中了某个点的“黄色方块”
                       // 注意：这里传入的是图片像素坐标
                       let hit = internalEditor.hitTestPoint(mouse.x, mouse.y);

                       if (hit.pointIndex !== -1) {
                           // 记录当前操作的 Item 索引和点索引
                           root.selectedItemIx = hit.internalIdx;
                           root.selectedPointIdx = hit.pointIndex;

                           // 此时可以给选中的这个 Item 特殊颜色反馈（可选）
                           internalEditor.setEditState(root.selectedItemIx, true);
                       }
                    }
                    else if (root.currentMode === MMImageEditor.EditMode.View) {
                        // 记录拖拽起始点（这是屏幕坐标）
                        root.lastMousePos = Qt.point(mouse.x * container.scale, mouse.y * container.scale);
                    }
                }

                onPositionChanged: (mouse) => {
                   if (root.currentMode === MMImageEditor.EditMode.EditSmooth && root.selectedPointIdx !== -1) {
                       // 2. 实时拖拽：将新坐标同步回 C++ Item
                       internalEditor.updatePoint(root.selectedItemIx, root.selectedPointIdx, mouse.x, mouse.y);
                       // 触发 C++ 重绘
                       internalEditor.update();
                    }
                    else if (root.currentMode === MMImageEditor.EditMode.View && interactionArea.pressed) {
                        // 计算偏移量
                        let currentPos = Qt.point(mouse.x * container.scale, mouse.y * container.scale);
                        let dx = currentPos.x - root.lastMousePos.x;
                        let dy = currentPos.y - root.lastMousePos.y;

                        // 手动更新 content 坐标
                        flick.contentX = Math.max(0, Math.min(flick.contentX - dx, flick.contentWidth - flick.width));
                        flick.contentY = Math.max(0, Math.min(flick.contentY - dy, flick.contentHeight - flick.height));

                        root.lastMousePos = currentPos;
                    } else if (points.length > 0) {
                        previewCanvas.requestPaint();
                    }
                }

                onClicked: (mouse) => {
                    if (root.currentMode === MMImageEditor.EditMode.Eraser) {
                        internalEditor.eraseAt(mouse.x, mouse.y);
                    } else if (root.currentMode === MMImageEditor.EditMode.Line || root.currentMode === MMImageEditor.EditMode.Circle) {
                        // ... 执行添加逻辑 ...
                        points.push(Qt.point(mouse.x, mouse.y));
                        if (root.currentMode === MMImageEditor.EditMode.Line && points.length === 2) {
                            internalEditor.addLine(points[0].x, points[0].y, points[1].x, points[1].y);
                            points = [];
                        } else if (root.currentMode === MMImageEditor.EditMode.Circle && points.length === 3) {
                            internalEditor.addCircle(points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y);
                            points = [];
                        }
                    }
                    previewCanvas.requestPaint();
                }
                onReleased: (mouse) => {
                    if (root.currentMode === MMImageEditor.EditMode.EditSmooth && root.selectedPointIdx !== -1) {
                        // 抬起鼠标时同步到数据库
                        //internalEditor.syncItemToDb(root.selectedItemIx);

                        // 重置选中状态，但保持 EditSmooth 模式（依然显示所有方块）
                        root.selectedPointIdx = -1;
                    }
                }
            }
        }
    }

    // WheelHandler {
    //     acceptedModifiers: Qt.ControlModifier // 修正：按住 Ctrl 滚动才缩放
    //     onWheel: (event) => {
    //         let factor = event.angleDelta.y > 0 ? 1.1 : 0.9;
    //         applyZoomCentered(factor);
    //     }
    // }

    // Ctrl + 滚轮缩放
    WheelHandler {
        acceptedModifiers: Qt.ControlModifier
        target: flick
        onWheel: (event) => {
            let factor = event.angleDelta.y > 0 ? 1.1 : 0.9;
            applyZoomCentered(factor);
            // 修复错误：增加对 event.point 的安全性检查
            // 如果 event.point 存在，则使用鼠标位置；否则使用窗口中心 (width/2, height/2)
            // let mX = (event.point && event.point.position) ? event.point.position.x : root.width / 2;
            // let mY = (event.point && event.point.position) ? event.point.position.y : root.height / 2;

            // applyZoomAtMouse(factor, mX, mY);
        }
    }

    // 小地图修正
    Rectangle {
        id: minimap
        anchors.right: parent.right; anchors.bottom: parent.bottom; anchors.margins: 12
        width: 160; height: internalEditor.implicitWidth > 0 ? width * (internalEditor.implicitHeight / internalEditor.implicitWidth) : 0
        color: "#AA000000"; border.color: "#555"; radius: 4
        visible: container.scale > minScale
        Image { anchors.fill: parent; anchors.margins: 2; source: root.source; fillMode: Image.PreserveAspectFit; opacity: 0.4 }
        Rectangle {
            x: (flick.contentX / flick.contentWidth) * minimap.width
            y: (flick.contentY / flick.contentHeight) * minimap.height
            width: (flick.width / flick.contentWidth) * minimap.width
            height: (flick.height / flick.contentHeight) * minimap.height
            color: "transparent"; border.color: "#ffb300"; border.width: 1
        }
    }

    // Rectangle {
    //     id: minimap
    //     anchors.right: parent.right
    //     anchors.bottom: parent.bottom
    //     anchors.margins: 20
    //     // 只有当缩放比例大于初始比例（即图片被放大）时才显示
    //     visible: container.scale > (minScale + 0.01) && root.currentMode !== MMImageEditor.EditMode.None

    //     width: 180
    //     height: internalEditor.implicitWidth > 0 ? width * (internalEditor.implicitHeight / internalEditor.implicitWidth) : 0
    //     color: "#CC1A1A1E"
    //     border.color: "#444"
    //     radius: 4
    //     z: 10

    //     Image {
    //         anchors.fill: parent
    //         anchors.margins: 2
    //         source: root.source
    //         fillMode: Image.PreserveAspectFit
    //         opacity: 0.5
    //     }

    //     // 小地图红框
    //     Rectangle {
    //         id: minimapRect
    //         // 计算逻辑：
    //         // contentX / contentWidth = 红框左边距 / 小地图宽度
    //         x: (flick.contentX / flick.contentWidth) * parent.width
    //         y: (flick.contentY / flick.contentHeight) * parent.height
    //         width: (flick.width / flick.contentWidth) * parent.width
    //         height: (flick.height / flick.contentHeight) * parent.height

    //         color: "transparent"
    //         border.color: "#00CCFF"
    //         border.width: 1

    //         // 防止边框超出小地图范围
    //         visible: width < parent.width || height < parent.height
    //     }
    // }

}
