// ModeIcon.qml (子组件)
import QtQuick
import QtQuick.Shapes

Item {
    id: iconRoot
    property int mode: 0
    property color iconColor: "white"
    width: 24; height: 24

    Shape {
        anchors.fill: parent
        ShapePath {
            strokeColor: iconColor
            strokeWidth: 1.5
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap

            // 根据模式切换路径数据
            PathSvg {
                path: {
                    if (mode === 1) // 平移 (手形)
                        return "M12,2 v4 M8,4 v3 M16,4 v3 M6,9 v6 c0,3 2,5 6,5 s6,-2 6,-5 v-8 a1.5,1.5 0 0 0 -3,0 v4 a1.5,1.5 0 0 0 -3,0 v-1 a1.5,1.5 0 0 0 -3,0 v1 a1.5,1.5 0 0 0 -3,0";
                    if (mode === 2) // 直线
                        return "M4,20 L20,4 M17,4 h3 v3 M4,17 v3 h3";
                    if (mode === 3) // 圆形
                        return "M12,2 a10,10 0 1,0 0.1,0 Z M12,7 v2 M12,15 v2 M7,12 h2 M15,12 h2";
                    if (mode === 4) // 橡皮擦
                        return "M4,10 L10,4 L20,14 L14,20 Z M7,13 L17,3 M14,20 h5";
                    if (mode === 5) // ShowSmooth (显示轮廓 - 侧脸轮廓)
                        // 绘制一个类似脸部侧面的流线型轮廓
                        return "M15,4 C10,4 6,8 6,12 C6,15 8,18 10,20 M10,20 L13,21 M15,4 L17,5 M12,10 A2,2 0 1,0 12,10.1";
                    if (mode === 6) // EditSmooth (精修轮廓 - 路径+锚点)
                        // 绘制一段曲线并在端点和中间绘制小方块(控制点)的视觉暗示
                        return "M4,15 C8,5 16,5 20,15 M3,14 h2 v2 h-2 Z M11,7 h2 v2 h-2 Z M19,14 h2 v2 h-2 Z";

                    return "M6,6 L18,18 M18,6 L6,18"; // 关闭/None
                }
            }
        }
    }
}
