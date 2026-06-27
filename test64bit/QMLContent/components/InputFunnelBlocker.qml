import QtQuick

/// 全窗口鼠标/滚轮漏斗：置于 Window 最顶层，active 时吞掉一切指针事件。
/// 禁止光标由 MM3DManager::setRunning 通过 QGuiApplication::setOverrideCursor 设置。
Item {
    id: root
    anchors.fill: parent
    visible: active
    enabled: active
    z: 2147483647

    property bool active: false

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.AllButtons
        preventStealing: true
        onWheel: function (wheel) { wheel.accepted = true }
    }
}
