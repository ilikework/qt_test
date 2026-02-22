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
    width: Math.min(520, 520)
    height: Math.min(560, 560)
    minimumWidth: 400
    minimumHeight: 400
    color: "transparent"
    visible: false

    // 由调用方设置后 show()
    property var host: null
    /// 若设置，打开时从此 manager 的 getProducts() 拉取 T_Offerings_Template 数据并填入 host，确保列表来自数据库
    property var preRecordManager: null
    property int reportIdx: -1
    property int tier: -1
    property var productsModel: host ? host.hostProductsModel : null

    ListModel { id: filteredProductsModel }
    property string productFilterText: ""
    /// 弹窗内本地勾选状态，仅点「确定」时写回 host，避免选择时立刻出现在报告预录
    property var pendingSelection: []

    function rebuildFilteredProducts() {
        filteredProductsModel.clear()
        if (!productsModel) return
        var filter = (productFilterText || "").toLowerCase()
        for (var i = 0; i < productsModel.count; i++) {
            var p = productsModel.get(i)
            var name = (p.name || "").toLowerCase()
            var priceVal = p.price
            var priceStr = (priceVal !== undefined && priceVal !== null) ? (typeof priceVal === "number" ? String(priceVal) : String(priceVal)) : ""
            if (!filter || name.indexOf(filter) >= 0)
                filteredProductsModel.append({ productIndex: i, name: p.name || "未命名", price: priceStr })
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
        next.sort(function(a,b) { return a - b })
        pendingSelection = next
    }

    function openFor(rIdx, tIdx) {
        if (!host) return
        reportIdx = rIdx
        tier = tIdx
        productFilterText = ""

        // 1) 确保 host 的产品列表是最新的。
        //    注意：loadFromPreRecordManager 会加载所有数据，包括产品和报告关联。
        //    这可以确保在打开选择器之前，host 的数据是最新的。
        //    如果 host 已经有未保存的修改，则不应执行此操作。
        //    PreRecordDialog 的逻辑是 onVisibleChanged 时加载，这里不再需要重复加载。

        // 2) 从 host (PreRecordDialog) 的暂存数据中初始化勾选状态
        var indices = []
        if (host && host.selectedOfferings && Array.isArray(host.selectedOfferings) &&
            reportIdx >= 0 && reportIdx < host.selectedOfferings.length &&
            tier >= 0 && tier < 3) {
            var selections = host.selectedOfferings[reportIdx][tier]
            if (selections && Array.isArray(selections)) {
                indices = selections
            }
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
                id: productFilterField
                Layout.fillWidth: true
                placeholderText: "输入名称筛选（支持大量产品时快速查找）"
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
                    spacing: 2
                    delegate: CheckBox {
                        width: pickerListView.width - 8
                        checked: model ? isSelected(model.productIndex) : false
                        text: model ? (model.name || "未命名") : ""
                        font.pixelSize: 14
                        palette.text: "#ffffff"
                        contentItem: Text {
                            text: model ? (model.name || "未命名") : ""
                            color: "#ffffff"
                            font: parent.font
                            leftPadding: parent.indicator.width + parent.spacing
                            verticalAlignment: Text.AlignVCenter
                        }
                        indicator: Rectangle {
                            implicitWidth: 20
                            implicitHeight: 20
                            x: parent.leftPadding
                            y: (parent.height - height) / 2
                            radius: 3
                            color: parent.checked ? "#00aaff" : "#333"
                            border.color: parent.checked ? "#00aaff" : "#555"
                            Rectangle {
                                visible: parent.parent.checked
                                anchors.centerIn: parent
                                width: 10
                                height: 10
                                radius: 2
                                color: "#ffffff"
                            }
                        }
                        onClicked: if (model) toggleSelected(model.productIndex)
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
