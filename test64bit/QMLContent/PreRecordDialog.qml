import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls.Basic
import "components"


// 预录界面原型（不依赖 components，可单独 qml 运行）（仅 QML + mock 数据，不依赖 C++/数据库）
// 模块1：产品/服务预录（照片、名字、价格、功能）
// 模块2：报告预录（9 种报告 × 好/中/差，每档：建议 + 多选产品/服务）
Window {
    id: preRecordWin
    width: 1600
    height: 1000
    flags: Qt.FramelessWindowHint

    // 作为子组件时默认不显示，由外部调用 show() 打开；单独 run PreRecordDialog 时 onCompleted 会设为 true

    property var reportLabels: [
        "毛 孔", "粉 刺", "深层色斑", "浅层色斑",
        "皱 纹", "敏感度", "褐色斑", "混合彩斑", "综合报告"
    ]
    // 9 个报告 × 3 档，每档选中的产品索引 [index, index, ...]
    property var selectedOfferings: []

    ListModel { id: productsModel }
    ListModel { id: reportModel }

    function ensureSelectedOfferings() {
        if (!selectedOfferings || !Array.isArray(selectedOfferings))
            selectedOfferings = []
        while (selectedOfferings.length < 9) {
            selectedOfferings.push([[], [], []])
        }
    }
    function reportLabel(i) { return (reportModel.count > i) ? reportModel.get(i).label : "" }
    function reportGoodMemo(i) { return (reportModel.count > i) ? reportModel.get(i).goodMemo : "" }
    function reportMediumMemo(i) { return (reportModel.count > i) ? reportModel.get(i).mediumMemo : "" }
    function reportBadMemo(i) { return (reportModel.count > i) ? reportModel.get(i).badMemo : "" }
    function isProductSelected(reportIdx, tier, productIdx) {
        ensureSelectedOfferings()
        if (reportIdx < 0 || reportIdx >= selectedOfferings.length) return false
        var row = selectedOfferings[reportIdx]
        if (!row || tier < 0 || tier > 2) return false
        var arr = row[tier]
        return arr && arr.indexOf(productIdx) >= 0
    }
    function toggleProductSelected(reportIdx, tier, productIdx) {
        ensureSelectedOfferings()
        if (reportIdx < 0 || reportIdx >= selectedOfferings.length || tier < 0 || tier > 2) return
        var next = []
        for (var r = 0; r < selectedOfferings.length; r++) {
            var row = selectedOfferings[r]
            next[r] = [
                (row && row[0] ? row[0].slice() : []),
                (row && row[1] ? row[1].slice() : []),
                (row && row[2] ? row[2].slice() : [])
            ]
        }
        var arr = next[reportIdx][tier]
        var i = arr.indexOf(productIdx)
        if (i >= 0) arr.splice(i, 1)
        else arr.push(productIdx)
        selectedOfferings = next
        syncReportOfferingsToDb()
    }
    function removeProductSelected(reportIdx, tier, productIdx) {
        ensureSelectedOfferings()
        if (reportIdx < 0 || reportIdx >= selectedOfferings.length || tier < 0 || tier > 2) return
        var next = []
        for (var r = 0; r < selectedOfferings.length; r++) {
            var row = selectedOfferings[r]
            next[r] = [
                (row && row[0] ? row[0].slice() : []),
                (row && row[1] ? row[1].slice() : []),
                (row && row[2] ? row[2].slice() : [])
            ]
        }
        var arr = next[reportIdx][tier]
        var i = arr.indexOf(productIdx)
        if (i >= 0) arr.splice(i, 1)
        selectedOfferings = next
        syncReportOfferingsToDb()
    }
    function getSelectedProductName(reportIdx, tier, slotIndex) {
        ensureSelectedOfferings()
        if (reportIdx < 0 || reportIdx >= selectedOfferings.length) return ""
        var arr = selectedOfferings[reportIdx][tier]
        if (!arr || slotIndex < 0 || slotIndex >= arr.length) return ""
        var ix = arr[slotIndex]
        if (ix < 0 || ix >= productsModel.count) return ""
        return productsModel.get(ix).name || "未命名"
    }
    function getSelectedProductIx(reportIdx, tier, slotIndex) {
        ensureSelectedOfferings()
        if (reportIdx < 0 || reportIdx >= selectedOfferings.length) return -1
        var arr = selectedOfferings[reportIdx][tier]
        if (!arr || slotIndex < 0 || slotIndex >= arr.length) return -1
        return arr[slotIndex]
    }

    function openProductPicker(rIdx, tIdx) {
        offeringPickerDialog.openFor(rIdx, tIdx)
    }

    property bool _usePreRecordManager: typeof preRecordManager !== "undefined"

    function buildProductsList() {
        var list = []
        for (var i = 0; i < productsModel.count; i++) {
            var p = productsModel.get(i)
            list.push({
                IX: p.IX || 0,
                name: p.name || "",
                price: p.price || "",
                usage: p.usage || "",
                photoPath: p.photoPath || ""
            })
        }
        return list
    }

    function loadFromPreRecordManager() {
        if (!_usePreRecordManager) return
        var products = preRecordManager.getProducts()
        productsModel.clear()
        for (var i = 0; i < products.length; i++)
            productsModel.append(products[i])
        var suggestions = preRecordManager.getReportSuggestions()
        reportModel.clear()
        for (var s = 0; s < suggestions.length; s++)
            reportModel.append(suggestions[s])
        ensureSelectedOfferings()
        for (var r = 0; r < 9; r++) {
            for (var t = 0; t < 3; t++) {
                var ixs = preRecordManager.getReportOfferings(r, t)
                var indices = []
                for (var k = 0; k < ixs.length; k++) {
                    for (var j = 0; j < productsModel.count; j++) {
                        if (productsModel.get(j).IX === ixs[k]) {
                            indices.push(j)
                            break
                        }
                    }
                }
                selectedOfferings[r][t] = indices
            }
        }
        selectedOfferings = selectedOfferings
    }

    function reloadProductsOnly() {
        if (!_usePreRecordManager) return
        var products = preRecordManager.getProducts()
        productsModel.clear()
        for (var i = 0; i < products.length; i++)
            productsModel.append(products[i])
    }

    function saveProductsToDb() {
        if (!_usePreRecordManager) return
        ensureSelectedOfferings()
        var savedIxs = []
        for (var r = 0; r < 9; r++) {
            savedIxs[r] = [[], [], []]
            for (var t = 0; t < 3; t++) {
                var arr = selectedOfferings[r] && selectedOfferings[r][t] ? selectedOfferings[r][t] : []
                for (var k = 0; k < arr.length; k++) {
                    var idx = arr[k]
                    if (idx >= 0 && idx < productsModel.count) {
                        var ix = productsModel.get(idx).IX
                        if (ix) savedIxs[r][t].push(ix)
                    }
                }
            }
        }
        if (!preRecordManager.saveProducts(buildProductsList())) return
        reloadProductsOnly()
        for (var r = 0; r < 9; r++) {
            for (var t = 0; t < 3; t++) {
                var indices = []
                for (var k = 0; k < savedIxs[r][t].length; k++) {
                    var ix = savedIxs[r][t][k]
                    for (var j = 0; j < productsModel.count; j++) {
                        if (productsModel.get(j).IX === ix) {
                            indices.push(j)
                            break
                        }
                    }
                }
                selectedOfferings[r][t] = indices
            }
        }
        selectedOfferings = selectedOfferings
    }

    function syncReportOfferingsToDb() {
        if (!_usePreRecordManager) return
        if (!preRecordManager.saveProducts(buildProductsList())) return
        for (var r = 0; r < 9; r++) {
            for (var t = 0; t < 3; t++) {
                var arr = selectedOfferings[r] && selectedOfferings[r][t] ? selectedOfferings[r][t] : []
                var ixs = []
                for (var k = 0; k < arr.length; k++) {
                    var idx = arr[k]
                    if (idx >= 0 && idx < productsModel.count) {
                        var ix = productsModel.get(idx).IX
                        if (ix) ixs.push(ix)
                    }
                }
                preRecordManager.setReportOfferings(r, t, ixs)
            }
        }
    }

    onVisibleChanged: {
        if (!visible) return
        ensureSelectedOfferings()
        if (_usePreRecordManager) {
            loadFromPreRecordManager()
            return
        }
        if (productsModel.count === 0) {
            productsModel.append({ name: "示例精华液", price: "298", usage: "每日早晚使用，配合按摩。" })
            productsModel.append({ name: "示例面膜", price: "88", usage: "每周 2–3 次，敷 15 分钟。" })
        }
        if (reportModel.count === 0) {
            for (var s = 0; s < 9; s++) {
                reportModel.append({
                    label: reportLabels[s],
                    goodMemo: "",
                    mediumMemo: "",
                    badMemo: ""
                })
            }
        }
    }

    Rectangle {
        id: mainContent
        anchors.fill: parent
        color: "#1c1c1c"

        Text {
            id: titleText
            text: "预录设置"
            font.pixelSize: 24
            color: "white"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 16
        }

        Row {
            id: tabRow
            anchors.top: titleText.bottom
            anchors.topMargin: 12
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 0
            property int currentIndex: 0
            Rectangle {
                width: 200
                height: 40
                color: tabRow.currentIndex === 0 ? "#2c2c2c" : "#1c1c1c"
                border.color: "#555"
                radius: 4
                Text {
                    anchors.centerIn: parent
                    text: "1. 产品/服务预录"
                    color: tabRow.currentIndex === 0 ? "#00aaff" : "#e0e0e0"
                    font.pixelSize: 15
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: tabRow.currentIndex = 0
                }
            }
            Rectangle {
                width: 200
                height: 40
                color: tabRow.currentIndex === 1 ? "#2c2c2c" : "#1c1c1c"
                border.color: "#555"
                radius: 4
                Text {
                    anchors.centerIn: parent
                    text: "2. 报告预录"
                    color: tabRow.currentIndex === 1 ? "#00aaff" : "#e0e0e0"
                    font.pixelSize: 15
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: tabRow.currentIndex = 1
                }
            }
        }

        StackLayout {
            anchors.top: tabRow.bottom
            anchors.topMargin: 12
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: bottomRow.top
            anchors.margins: 16
            currentIndex: tabRow.currentIndex

            // ========== 模块1：产品/服务预录 ==========
            Item {
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8
                    Text {
                        text: "维护产品/服务目录，供报告预录时选用。可上传照片、填写名称、价格、功能说明。"
                        color: "#e0e0e0"
                        font.pixelSize: 13
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }
                    TextButton {
                        Layout.alignment: Qt.AlignRight
                        text: "添加"
                        onClicked: {
                                                        productsModel.append({ name: "", price: "", usage: "", photoPath: "" })
                                                        saveProductsToDb()
                                                    }
                    }
                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                        ListView {
                            id: productListView
                            model: productsModel
                            spacing: 8
                            delegate: Rectangle {
                                width: productListView.width - 8
                                height: productRow.height + 16
                                color: "#2c2c2c"
                                radius: 6
                                border.color: "#555"
                                ColumnLayout {
                                    id: productRow
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.top: parent.top
                                    anchors.margins: 8
                                    spacing: 6
                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 10
                                        Rectangle {
                                            width: 72
                                            height: 72
                                            radius: 4
                                            color: "#1a1a1a"
                                            Layout.alignment: Qt.AlignTop
                                            Image {
                                                id: productImg
                                                anchors.fill: parent
                                                anchors.margins: 2
                                                fillMode: Image.PreserveAspectFit
                                                source: model.photoPath ? (String(model.photoPath).indexOf("file://") === 0 ? model.photoPath : "file:///" + model.photoPath) : ""
                                                visible: !!(model.photoPath && String(model.photoPath).length > 0)
                                            }
                                            Text {
                                                anchors.centerIn: parent
                                                text: "点击选图"
                                                color: "#b0b0b0"
                                                font.pixelSize: 11
                                                visible: !productImg.visible
                                            }
                                            MouseArea {
                                                anchors.fill: parent
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: {
                                                    productPhotoDialog.currentIndex = index
                                                    productPhotoDialog.open()
                                                }
                                            }
                                        }
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 4
                                            TextField {
                                                Layout.fillWidth: true
                                                placeholderText: "名称"
                                                text: model.name
                                                font.pixelSize: 14
                                                color: "#ffffff"
                                                placeholderTextColor: "#888"
                                                background: Rectangle {
                                                    color: "#333"
                                                    radius: 4
                                                    border.color: "#555"
                                                }
                                                onTextChanged: {
                                                    if (index >= 0 && index < productsModel.count && String(model.name) !== text) {
                                                        productsModel.setProperty(index, "name", text)
                                                        saveProductsToDb()
                                                    }
                                                }
                                            }
                                            TextField {
                                                Layout.fillWidth: true
                                                placeholderText: "价格"
                                                text: model.price
                                                font.pixelSize: 14
                                                color: "#ffffff"
                                                placeholderTextColor: "#888"
                                                background: Rectangle {
                                                    color: "#333"
                                                    radius: 4
                                                    border.color: "#555"
                                                }
                                                onTextChanged: {
                                                    if (index >= 0 && index < productsModel.count && String(model.price) !== text) {
                                                        productsModel.setProperty(index, "price", text)
                                                        saveProductsToDb()
                                                    }
                                                }
                                            }
                                            TextArea {
                                                Layout.fillWidth: true
                                                Layout.preferredHeight: 44
                                                placeholderText: "功能说明"
                                                text: model.usage
                                                font.pixelSize: 13
                                                color: "#ffffff"
                                                placeholderTextColor: "#888"
                                                background: Rectangle {
                                                    color: "#333"
                                                    radius: 4
                                                    border.color: "#555"
                                                }
                                                onTextChanged: {
                                                    if (index >= 0 && index < productsModel.count) {
                                                        productsModel.setProperty(index, "usage", text)
                                                        saveProductsToDb()
                                                    }
                                                }
                                            }
                                        }
                                        TextButton {
                                            text: "删除"
                                            Layout.alignment: Qt.AlignTop
                                            implicitWidth: 80
                                            implicitHeight: 32
                                            onClicked: {
                                                if (index >= 0 && index < productsModel.count) {
                                                    productsModel.remove(index)
                                                    saveProductsToDb()
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ========== 模块2：报告预录（Tab 按报告类型，每页一种，尽量无滚动条）==========
            Item {
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4
                    Text {
                        text: "每种报告分 好 / 中 / 差 三档，每档填写建议并勾选推荐产品/服务。请切换下方 Tab 选择报告类型。"
                        color: "#e0e0e0"
                        font.pixelSize: 13
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }
                    TabBar {
                        id: reportTabBar
                        Layout.fillWidth: true
                        Layout.topMargin: 2
                        currentIndex: reportStack.currentIndex
                        onCurrentIndexChanged: reportStack.currentIndex = currentIndex
                        Repeater {
                            model: 9
                            TabButton {
                                text: reportLabels[index]
                                font.pixelSize: 17
                                palette.button: reportTabBar.currentIndex === index ? "#2c2c2c" : "#1c1c1c"
                                palette.buttonText: reportTabBar.currentIndex === index ? "#00aaff" : "#e0e0e0"
                                contentItem: Item {
                                    implicitWidth: label.implicitWidth + 20
                                    implicitHeight: label.implicitHeight + 12
                                    Text {
                                        id: label
                                        text: parent.parent.text
                                        font: parent.parent.font
                                        color: parent.parent.palette.buttonText
                                        anchors.centerIn: parent
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        acceptedButtons: Qt.NoButton
                                    }
                                }
                            }
                        }
                    }
                    StackLayout {
                        id: reportStack
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        currentIndex: reportTabBar.currentIndex
                        Repeater {
                            model: 9
                            Item {
                                property int reportIdx: index
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                Rectangle {
                                    anchors.fill: parent
                                    color: "#252525"
                                    radius: 8
                                    border.color: "#444"
                                    ColumnLayout {
                                        anchors.top: parent.top
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        anchors.margins: 16
                                        spacing: 6
                                        Text {
                                            text: reportLabel(reportIdx)
                                            color: "#ffffff"
                                            font.pixelSize: 20
                                            font.bold: true
                                            Layout.fillWidth: true
                                        }
                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 20
                                            // 好
                                            ColumnLayout {
                                                Layout.fillWidth: true
                                                Layout.minimumWidth: 160
                                                spacing: 6
                                                Text { text: "好"; color: "#7ec0ff"; font.pixelSize: 15; Layout.fillWidth: true }
                                                TextArea {
                                                    Layout.fillWidth: true
                                                    Layout.preferredHeight: 240
                                                    placeholderText: "建议（约可显示 300 字）"
                                                    text: reportGoodMemo(reportIdx)
                                                    font.pixelSize: 14
                                                    color: "#ffffff"
                                                    placeholderTextColor: "#888"
                                                    background: Rectangle { color: "#2c2c2c"; border.color: "#555"; radius: 4 }
                                                    onTextChanged: {
                                                        if (reportIdx < reportModel.count) {
                                                            reportModel.setProperty(reportIdx, "goodMemo", text)
                                                            if (_usePreRecordManager) preRecordManager.setReportMemo(reportIdx, 0, text)
                                                        }
                                                    }
                                                }
                                                Rectangle {
                                                    Layout.alignment: Qt.AlignLeft
                                                    implicitWidth: 120
                                                    implicitHeight: 30
                                                    radius: 4
                                                    property bool hovered: false
                                                    property bool pressed: false
                                                    color: pressed ? "#2a3a4a" : (hovered ? "#333" : "#2a2a2a")
                                                    border.color: hovered ? "#5a9aca" : "#555"
                                                    Text {
                                                        anchors.centerIn: parent
                                                        text: "选择产品/服务"
                                                        color: parent.hovered ? "#9ec8ff" : "#b0b0b0"
                                                        font.pixelSize: 12
                                                    }
                                                    MouseArea {
                                                        anchors.fill: parent
                                                        hoverEnabled: true
                                                        cursorShape: Qt.PointingHandCursor
                                                        onContainsMouseChanged: parent.hovered = containsMouse
                                                        onPressedChanged: parent.pressed = containsMouse && pressed
                                                        onClicked: openProductPicker(reportIdx, 0)
                                                    }
                                                }
                                                Text { text: "已选产品"; color: "#e0e0e0"; font.pixelSize: 12; Layout.fillWidth: true }
                                                Item {
                                                    property int rIdx: (reportIdx >= 0) ? reportIdx : 0
                                                    Layout.fillWidth: true
                                                    Layout.preferredHeight: 68
                                                    Flow {
                                                        anchors.fill: parent
                                                        spacing: 6
                                                        Repeater {
                                                            model: selectedOfferings[parent.parent.rIdx] && selectedOfferings[parent.parent.rIdx][0] ? selectedOfferings[parent.parent.rIdx][0].length : 0
                                                            delegate: Row {
                                                                spacing: 2
                                                                property int rIdx: parent.parent.parent.rIdx
                                                                Rectangle {
                                                                    width: (children[0] ? children[0].implicitWidth : 0) + 28
                                                                    height: 24
                                                                    radius: 4
                                                                    color: "#3a3a3a"
                                                                    border.color: "#555"
                                                                    Text { text: getSelectedProductName(rIdx, 0, index); color: "#ffffff"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 5 }
                                                                    TextButton {
                                                                        width: 24
                                                                        height: 24
                                                                        anchors.right: parent.right
                                                                        anchors.verticalCenter: parent.verticalCenter
                                                                        anchors.rightMargin: 2
                                                                        text: "×"
                                                                        onClicked: removeProductSelected(rIdx, 0, getSelectedProductIx(rIdx, 0, index))
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            // 中
                                            ColumnLayout {
                                                Layout.fillWidth: true
                                                Layout.minimumWidth: 160
                                                spacing: 6
                                                Text { text: "中"; color: "#e0e0e0"; font.pixelSize: 15; Layout.fillWidth: true }
                                                TextArea {
                                                    Layout.fillWidth: true
                                                    Layout.preferredHeight: 240
                                                    placeholderText: "建议（约可显示 300 字）"
                                                    text: reportMediumMemo(reportIdx)
                                                    font.pixelSize: 14
                                                    color: "#ffffff"
                                                    placeholderTextColor: "#888"
                                                    background: Rectangle { color: "#2c2c2c"; border.color: "#555"; radius: 4 }
                                                    onTextChanged: {
                                                        if (reportIdx < reportModel.count) {
                                                            reportModel.setProperty(reportIdx, "mediumMemo", text)
                                                            if (_usePreRecordManager) preRecordManager.setReportMemo(reportIdx, 1, text)
                                                        }
                                                    }
                                                }
                                                Rectangle {
                                                    Layout.alignment: Qt.AlignLeft
                                                    implicitWidth: 120
                                                    implicitHeight: 30
                                                    radius: 4
                                                    property bool hovered: false
                                                    property bool pressed: false
                                                    color: pressed ? "#2a3a4a" : (hovered ? "#333" : "#2a2a2a")
                                                    border.color: hovered ? "#5a9aca" : "#555"
                                                    Text {
                                                        anchors.centerIn: parent
                                                        text: "选择产品/服务"
                                                        color: parent.hovered ? "#9ec8ff" : "#b0b0b0"
                                                        font.pixelSize: 12
                                                    }
                                                    MouseArea {
                                                        anchors.fill: parent
                                                        hoverEnabled: true
                                                        cursorShape: Qt.PointingHandCursor
                                                        onContainsMouseChanged: parent.hovered = containsMouse
                                                        onPressedChanged: parent.pressed = containsMouse && pressed
                                                        onClicked: openProductPicker(reportIdx, 1)
                                                    }
                                                }
                                                Text { text: "已选产品"; color: "#e0e0e0"; font.pixelSize: 12; Layout.fillWidth: true }
                                                Item {
                                                    property int rIdx: (reportIdx >= 0) ? reportIdx : 0
                                                    Layout.fillWidth: true
                                                    Layout.preferredHeight: 68
                                                    Flow {
                                                        anchors.fill: parent
                                                        spacing: 6
                                                        Repeater {
                                                            model: selectedOfferings[parent.parent.rIdx] && selectedOfferings[parent.parent.rIdx][1] ? selectedOfferings[parent.parent.rIdx][1].length : 0
                                                            delegate: Row {
                                                                spacing: 2
                                                                property int rIdx: parent.parent.parent.rIdx
                                                                Rectangle {
                                                                    width: (children[0] ? children[0].implicitWidth : 0) + 28
                                                                    height: 24
                                                                    radius: 4
                                                                    color: "#3a3a3a"
                                                                    border.color: "#555"
                                                                    Text { text: getSelectedProductName(rIdx, 1, index); color: "#ffffff"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 5 }
                                                                    TextButton {
                                                                        width: 24
                                                                        height: 24
                                                                        anchors.right: parent.right
                                                                        anchors.verticalCenter: parent.verticalCenter
                                                                        anchors.rightMargin: 2
                                                                        text: "×"
                                                                        onClicked: removeProductSelected(rIdx, 1, getSelectedProductIx(rIdx, 1, index))
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            // 差
                                            ColumnLayout {
                                                Layout.fillWidth: true
                                                Layout.minimumWidth: 160
                                                spacing: 6
                                                Text { text: "差"; color: "#ff8888"; font.pixelSize: 15; Layout.fillWidth: true }
                                                TextArea {
                                                    Layout.fillWidth: true
                                                    Layout.preferredHeight: 240
                                                    placeholderText: "建议（约可显示 300 字）"
                                                    text: reportBadMemo(reportIdx)
                                                    font.pixelSize: 14
                                                    color: "#ffffff"
                                                    placeholderTextColor: "#888"
                                                    background: Rectangle { color: "#2c2c2c"; border.color: "#555"; radius: 4 }
                                                    onTextChanged: {
                                                        if (reportIdx < reportModel.count) {
                                                            reportModel.setProperty(reportIdx, "badMemo", text)
                                                            if (_usePreRecordManager) preRecordManager.setReportMemo(reportIdx, 2, text)
                                                        }
                                                    }
                                                }
                                                Rectangle {
                                                    Layout.alignment: Qt.AlignLeft
                                                    implicitWidth: 120
                                                    implicitHeight: 30
                                                    radius: 4
                                                    property bool hovered: false
                                                    property bool pressed: false
                                                    color: pressed ? "#2a3a4a" : (hovered ? "#333" : "#2a2a2a")
                                                    border.color: hovered ? "#5a9aca" : "#555"
                                                    Text {
                                                        anchors.centerIn: parent
                                                        text: "选择产品/服务"
                                                        color: parent.hovered ? "#9ec8ff" : "#b0b0b0"
                                                        font.pixelSize: 12
                                                    }
                                                    MouseArea {
                                                        anchors.fill: parent
                                                        hoverEnabled: true
                                                        cursorShape: Qt.PointingHandCursor
                                                        onContainsMouseChanged: parent.hovered = containsMouse
                                                        onPressedChanged: parent.pressed = containsMouse && pressed
                                                        onClicked: openProductPicker(reportIdx, 2)
                                                    }
                                                }
                                                Text { text: "已选产品"; color: "#e0e0e0"; font.pixelSize: 12; Layout.fillWidth: true }
                                                Item {
                                                    property int rIdx: (reportIdx >= 0) ? reportIdx : 0
                                                    Layout.fillWidth: true
                                                    Layout.preferredHeight: 68
                                                    Flow {
                                                        anchors.fill: parent
                                                        spacing: 6
                                                        Repeater {
                                                            model: selectedOfferings[parent.parent.rIdx] && selectedOfferings[parent.parent.rIdx][2] ? selectedOfferings[parent.parent.rIdx][2].length : 0
                                                            delegate: Row {
                                                                spacing: 2
                                                                property int rIdx: parent.parent.parent.rIdx
                                                                Rectangle {
                                                                    width: (children[0] ? children[0].implicitWidth : 0) + 28
                                                                    height: 24
                                                                    radius: 4
                                                                    color: "#3a3a3a"
                                                                    border.color: "#555"
                                                                    Text { text: getSelectedProductName(rIdx, 2, index); color: "#ffffff"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 5 }
                                                                    TextButton {
                                                                        width: 24
                                                                        height: 24
                                                                        anchors.right: parent.right
                                                                        anchors.verticalCenter: parent.verticalCenter
                                                                        anchors.rightMargin: 2
                                                                        text: "×"
                                                                        onClicked: removeProductSelected(rIdx, 2, getSelectedProductIx(rIdx, 2, index))
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        Row {
            id: bottomRow
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 24
            TextButton {
                text: "保存"
                width: 90
                height: 40
                onClicked: close()
            }
            TextButton {
                text: "返回"
                width: 90
                height: 40
                onClicked: close()
            }
        }
    }

    OfferingPickerDialog {
        id: offeringPickerDialog
        host: preRecordWin
        transientParent: preRecordWin
    }

    FileDialog {
        id: productPhotoDialog
        property int currentIndex: -1
        title: "选择产品/服务图片"
        fileMode: FileDialog.OpenFile
        nameFilters: ["图片 (*.png *.jpg *.jpeg *.bmp)"]
        onAccepted: {
            if (currentIndex >= 0 && currentIndex < productsModel.count) {
                var path = selectedFile.toString()
                if (path.indexOf("file:///") === 0) path = path.substring(8)
                productsModel.setProperty(currentIndex, "photoPath", path)
                saveProductsToDb()
            }
        }
    }
}
