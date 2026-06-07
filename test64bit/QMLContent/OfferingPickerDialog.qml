import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Basic
import "components"

// 选择产品/服务（Offering）— 独立模态窗口，供 PreRecordDialog 等引用
Window {
    id: offeringPickerWin
    modality: Qt.ApplicationModal
    flags: Qt.Dialog | Qt.FramelessWindowHint
    width: 780
    height: 680
    minimumWidth: 640
    minimumHeight: 520
    color: "transparent"
    visible: false

    property var host: null
    property var preRecordManager: null
    property int reportIdx: -1
    property int tier: -1
    property var productsModel: host ? host.hostProductsModel : null

    ListModel { id: filteredProductsModel }
    property string productFilterText: ""
    property var pendingSelection: []

    function photoUrl(path) {
        if (!path || String(path).length === 0)
            return ""
        var s = String(path)
        return s.indexOf("file://") === 0 ? s : "file:///" + s
    }

    function formatPrice(priceVal) {
        if (priceVal === undefined || priceVal === null || priceVal === "")
            return ""
        if (typeof priceVal === "number")
            return priceVal % 1 === 0 ? String(priceVal) : priceVal.toFixed(2)
        return String(priceVal)
    }

    function rebuildFilteredProducts() {
        filteredProductsModel.clear()
        if (!productsModel) return
        var filter = (productFilterText || "").toLowerCase()
        for (var i = 0; i < productsModel.count; i++) {
            var p = productsModel.get(i)
            var name = (p.name || "").toLowerCase()
            var usage = (p.usage || "").toLowerCase()
            var priceStr = formatPrice(p.price)
            if (!filter || name.indexOf(filter) >= 0 || usage.indexOf(filter) >= 0)
                filteredProductsModel.append({
                    productIndex: i,
                    name: p.name || "未命名",
                    priceText: priceStr,
                    usage: p.usage || "",
                    photoPath: p.photoPath || ""
                })
        }
    }

    function isSelected(productIndex) {
        return pendingSelection.indexOf(productIndex) >= 0
    }

    function toggleSelected(productIndex) {
        var i = pendingSelection.indexOf(productIndex)
        var next = pendingSelection.slice()
        if (i >= 0) next.splice(i, 1)
        else next.push(productIndex)
        next.sort(function(a, b) { return a - b })
        pendingSelection = next
    }

    function openFor(rIdx, tIdx) {
        if (!host) return
        reportIdx = rIdx
        tier = tIdx
        productFilterText = ""

        var indices = []
        if (host && host.selectedOfferings && Array.isArray(host.selectedOfferings) &&
            reportIdx >= 0 && reportIdx < host.selectedOfferings.length &&
            tier >= 0 && tier < 3) {
            var selections = host.selectedOfferings[reportIdx][tier]
            if (selections && Array.isArray(selections))
                indices = selections
        }

        pendingSelection = indices.slice ? indices.slice() : indices
        rebuildFilteredProducts()
        show()
    }

    onProductsModelChanged: rebuildFilteredProducts()
    onProductFilterTextChanged: rebuildFilteredProducts()
    onVisibleChanged: {
        if (visible) {
            rebuildFilteredProducts()
            if (transientParent) {
                x = transientParent.x + (transientParent.width - width) / 2
                y = transientParent.y + (transientParent.height - height) / 2
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#252525"
        border.color: "#555"
        radius: 8

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Text {
                text: "选择产品/服务"
                color: "#ffffff"
                font.pixelSize: 18
                font.bold: true
            }
            TextField {
                Layout.fillWidth: true
                placeholderText: "输入名称或功能说明筛选"
                text: productFilterText
                font.pixelSize: 14
                color: "#ffffff"
                placeholderTextColor: "#888"
                background: Rectangle {
                    color: "#333"
                    radius: 4
                    border.color: "#555"
                }
                onTextChanged: productFilterText = text
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                Text {
                    text: {
                        var total = filteredProductsModel.count
                        var sel = 0
                        for (var i = 0; i < filteredProductsModel.count; i++) {
                            var ix = filteredProductsModel.get(i).productIndex
                            if (isSelected(ix)) sel++
                        }
                        "共 " + total + " 条，已选 " + sel + " 条"
                    }
                    color: "#b0b0b0"
                    font.pixelSize: 13
                }
                Item { Layout.fillWidth: true }
                Rectangle {
                    implicitWidth: 64
                    implicitHeight: 28
                    radius: 4
                    property bool hovered: false
                    property bool pressed: false
                    color: pressed ? "#2a3a4a" : (hovered ? "#333" : "#2a2a2a")
                    border.color: hovered ? "#5a9aca" : "#555"
                    Text {
                        anchors.centerIn: parent
                        text: "全选"
                        color: parent.hovered ? "#9ec8ff" : "#b0b0b0"
                        font.pixelSize: 12
                    }
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onContainsMouseChanged: parent.hovered = containsMouse
                        onPressedChanged: parent.pressed = containsMouse && pressed
                        onClicked: {
                            for (var i = 0; i < filteredProductsModel.count; i++) {
                                var ix = filteredProductsModel.get(i).productIndex
                                if (!isSelected(ix)) toggleSelected(ix)
                            }
                        }
                    }
                }
                Rectangle {
                    implicitWidth: 72
                    implicitHeight: 28
                    radius: 4
                    property bool hovered: false
                    property bool pressed: false
                    color: pressed ? "#3a2a2a" : (hovered ? "#442222" : "#2a2a2a")
                    border.color: hovered ? "#8a5555" : "#555"
                    Text {
                        anchors.centerIn: parent
                        text: "取消全选"
                        color: parent.hovered ? "#dd9999" : "#b0b0b0"
                        font.pixelSize: 12
                    }
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onContainsMouseChanged: parent.hovered = containsMouse
                        onPressedChanged: parent.pressed = containsMouse && pressed
                        onClicked: {
                            for (var i = 0; i < filteredProductsModel.count; i++) {
                                var ix = filteredProductsModel.get(i).productIndex
                                if (isSelected(ix)) toggleSelected(ix)
                            }
                        }
                    }
                }
            }
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                contentWidth: availableWidth
                ListView {
                    id: pickerListView
                    model: filteredProductsModel
                    spacing: 6
                    delegate: Rectangle {
                        width: pickerListView.width - 8
                        height: 88
                        radius: 6
                        color: isSelected(model.productIndex) ? "#2a3a4a" : "#2c2c2c"
                        border.color: isSelected(model.productIndex) ? "#5a9aca" : "#444"
                        border.width: isSelected(model.productIndex) ? 2 : 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 10

                            Rectangle {
                                width: 22
                                height: 22
                                radius: 3
                                color: isSelected(model.productIndex) ? "#00aaff" : "#333"
                                border.color: isSelected(model.productIndex) ? "#00aaff" : "#555"
                                Layout.alignment: Qt.AlignTop
                                Layout.topMargin: 4
                                Rectangle {
                                    visible: isSelected(model.productIndex)
                                    anchors.centerIn: parent
                                    width: 10
                                    height: 10
                                    radius: 2
                                    color: "#ffffff"
                                }
                            }

                            Rectangle {
                                width: 72
                                height: 72
                                radius: 4
                                color: "#1a1a1a"
                                border.color: "#555"
                                Layout.alignment: Qt.AlignVCenter
                                Image {
                                    anchors.fill: parent
                                    anchors.margins: 2
                                    fillMode: Image.PreserveAspectFit
                                    source: photoUrl(model.photoPath)
                                    visible: !!(model.photoPath && String(model.photoPath).length > 0)
                                }
                                Text {
                                    anchors.centerIn: parent
                                    text: "无图"
                                    color: "#666"
                                    font.pixelSize: 11
                                    visible: !(model.photoPath && String(model.photoPath).length > 0)
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                spacing: 4

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8
                                    Text {
                                        text: model.name || "未命名"
                                        color: "#ffffff"
                                        font.pixelSize: 15
                                        font.bold: true
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }
                                    Text {
                                        text: model.priceText !== "" ? ("¥" + model.priceText) : "价格未填"
                                        color: model.priceText !== "" ? "#7ec0ff" : "#888"
                                        font.pixelSize: 14
                                        font.bold: true
                                    }
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: model.usage || "（无功能说明）"
                                    color: model.usage ? "#b0b0b0" : "#666"
                                    font.pixelSize: 12
                                    wrapMode: Text.WordWrap
                                    maximumLineCount: 2
                                    elide: Text.ElideRight
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: toggleSelected(model.productIndex)
                        }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
                spacing: 12
                TextButton {
                    text: "取消"
                    implicitWidth: 80
                    implicitHeight: 36
                    onClicked: offeringPickerWin.close()
                }
                TextButton {
                    text: "确定"
                    implicitWidth: 80
                    implicitHeight: 36
                    onClicked: {
                        if (host && typeof host.setSelectedOfferingsForReport === "function")
                            host.setSelectedOfferingsForReport(reportIdx, tier, pendingSelection.slice(), true)
                        offeringPickerWin.close()
                    }
                }
            }
        }
    }
}
