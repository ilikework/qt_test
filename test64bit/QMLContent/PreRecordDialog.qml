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
    /// 暴露给子组件（如 OfferingPickerDialog）：子组件里 host.productsModel 取不到 id，需用 host 上的属性
    property alias hostProductsModel: productsModel
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
        preRecordDirty = true
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

    /// 设置某报告某档的已选产品（indices 为 productsModel 的索引）。markDirty：是否记为未保存（true=用户点确定，false=打开 picker 时从 DB 初始化）
    function setSelectedOfferingsForReport(rIdx, tIdx, indices, markDirty) {
        ensureSelectedOfferings()
        if (rIdx < 0 || rIdx >= selectedOfferings.length || tIdx < 0 || tIdx > 2) return
        var next = []
        for (var r = 0; r < selectedOfferings.length; r++) {
            var row = selectedOfferings[r]
            next[r] = [
                (row && row[0] ? row[0].slice() : []),
                (row && row[1] ? row[1].slice() : []),
                (row && row[2] ? row[2].slice() : [])
            ]
        }
        next[rIdx][tIdx] = (indices && indices.slice) ? indices.slice() : (indices || [])
        selectedOfferings = next
        if (markDirty) preRecordDirty = true
    }

    function openProductPicker(rIdx, tIdx) {
        offeringPickerDialog.openFor(rIdx, tIdx)
    }

    property bool _usePreRecordManager: typeof preRecordManager !== "undefined"
    /// 暴露给子组件用，避免子组件内 preRecordManager 绑定自引用导致 binding loop
    property var productsManager: typeof preRecordManager !== "undefined" ? preRecordManager : null
    /// 是否有未保存的修改（产品/报告建议/报告关联产品）；无修改时保存按钮仅关闭，不写库
    property bool preRecordDirty: false
    /// 为 true 时报告建议 onTextChanged 不置 dirty（避免首次加载时绑定更新触发保存确认）
    property bool _suppressDirtyFromLoad: false

    function buildProductsList() {
        var list = []
        for (var i = 0; i < productsModel.count; i++) {
            var p = productsModel.get(i)
            var priceVal = p.price
            var priceNum = (priceVal !== undefined && priceVal !== null && priceVal !== "")
                ? (typeof priceVal === "number" ? priceVal : Number(priceVal))
                : 0
            if (isNaN(priceNum) || priceNum < 0) priceNum = 0
            list.push({
                IX: p.IX || 0,
                name: (p.name !== undefined && p.name !== null) ? String(p.name) : "",
                price: priceNum,
                usage: (p.usage !== undefined && p.usage !== null) ? String(p.usage) : "",
                photoPath: (p.photoPath !== undefined && p.photoPath !== null) ? String(p.photoPath) : ""
            })
        }
        return list
    }

    function loadFromPreRecordManager() {
        if (!_usePreRecordManager) return
        _suppressDirtyFromLoad = true
        preRecordDirty = false
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
        Qt.callLater(function() { _suppressDirtyFromLoad = false })
    }

    function isPriceValid(text) {
        if (text === null || text === undefined) return false
        var s = String(text).trim()
        if (s === "") return true
        var n = Number(s)
        return !isNaN(n) && n >= 0
    }

    /// 保存前校验：产品名称不能为空。返回 { ok: bool, emptyAt: number? }
    function checkProductNamesNonEmpty() {
        for (var i = 0; i < productsModel.count; i++) {
            var name = String(productsModel.get(i).name || "").trim()
            if (name === "") return { ok: false, emptyAt: i }
        }
        return { ok: true, emptyAt: -1 }
    }
    /// 保存前校验：每条产品必须填写价格（可为 0，不可为空）。返回 { ok: bool, emptyAt: number? }
    function checkProductPricesFilled() {
        for (var i = 0; i < productsModel.count; i++) {
            var p = productsModel.get(i).price
            if (p === undefined || p === null || p === "") return { ok: false, emptyAt: i }
            var s = String(p).trim()
            if (s === "") return { ok: false, emptyAt: i }
            var n = Number(s)
            if (isNaN(n) || n < 0) return { ok: false, emptyAt: i }
        }
        return { ok: true, emptyAt: -1 }
    }
    /// 保存前校验：产品名称不能重复（非空名称）。返回 { ok: bool, duplicateName: string? }
    function checkProductNamesUnique() {
        var seen = {}
        for (var i = 0; i < productsModel.count; i++) {
            var name = String(productsModel.get(i).name || "").trim()
            if (name === "") continue
            if (seen[name]) return { ok: false, duplicateName: name }
            seen[name] = true
        }
        return { ok: true, duplicateName: null }
    }

    /// 统一保存到数据库：产品名称非空+不重复校验 → 保存产品 → 保存各报告 memo → 保存各报告关联产品（T_Report_Offerings_Template）
    function doSaveToDb() {
        if (!_usePreRecordManager) return true
        var emptyCheck = checkProductNamesNonEmpty()
        if (!emptyCheck.ok) {
            saveErrorDialog.boxMessage = "请填写产品名称（第 " + (emptyCheck.emptyAt + 1) + " 条名称为空）"
            saveErrorDialog.open()
            return false
        }
        var namesCheck = checkProductNamesUnique()
        if (!namesCheck.ok) {
            saveErrorDialog.boxMessage = "产品名称不能重复：「" + (namesCheck.duplicateName || "") + "」"
            saveErrorDialog.open()
            return false
        }
        var priceCheck = checkProductPricesFilled()
        if (!priceCheck.ok) {
            saveErrorDialog.boxMessage = "请填写价格（第 " + (priceCheck.emptyAt + 1) + " 条价格为空，0 为有效值）"
            saveErrorDialog.open()
            return false
        }
        ensureSelectedOfferings()
        if (!preRecordManager.saveProducts(buildProductsList())) return false
        preRecordDirty = false
        for (var r = 0; r < 9; r++) {
            if (r < reportModel.count) {
                var rec = reportModel.get(r)
                preRecordManager.setReportMemo(r, 0, rec.goodMemo || "")
                preRecordManager.setReportMemo(r, 1, rec.mediumMemo || "")
                preRecordManager.setReportMemo(r, 2, rec.badMemo || "")
            }
        }
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
        return true
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
                    onClicked: {
                        if (tabRow.currentIndex === 1) return
                        if (!preRecordDirty) { tabRow.currentIndex = 1; return }
                        savePromptDialog.open()
                    }
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
                            preRecordDirty = true
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
                                                    if (typeof index !== "undefined" && index >= 0 && index < productsModel.count && String(model.name) !== text) {
                                                        productsModel.setProperty(index, "name", text)
                                                        preRecordDirty = true
                                                    }
                                                }
                                                onEditingFinished: {
                                                    if (typeof index !== "undefined" && index >= 0 && index < productsModel.count && String(model.name) !== text) {
                                                        productsModel.setProperty(index, "name", text)
                                                        preRecordDirty = true
                                                    }
                                                }
                                            }
                                            TextField {
                                                id: priceField
                                                Layout.fillWidth: true
                                                placeholderText: "价格（必填，数字≥0，可为0）"
                                                text: (typeof model.price === "number") ? model.price : (model.price !== undefined && model.price !== null && model.price !== "" ? String(model.price) : "")
                                                font.pixelSize: 14
                                                color: "#ffffff"
                                                placeholderTextColor: "#888"
                                                validator: DoubleValidator { bottom: 0; decimals: 2; locale: "C" }
                                                background: Rectangle {
                                                    color: priceField.activeFocus && !isPriceValid(priceField.text) ? "#442222" : "#333"
                                                    radius: 4
                                                    border.color: priceField.activeFocus && !isPriceValid(priceField.text) ? "#aa5555" : "#555"
                                                }
                                                onTextChanged: {
                                                    if (typeof index === "undefined" || index < 0 || index >= productsModel.count) return
                                                    var t = text.trim()
                                                    if (t === "") {
                                                        productsModel.setProperty(index, "price", "")
                                                        preRecordDirty = true
                                                        return
                                                    }
                                                    if (isPriceValid(t)) {
                                                        var num = Number(t)
                                                        var cur = model.price
                                                        var curNum = (typeof cur === "number") ? cur : (cur === "" || cur === undefined || cur === null ? NaN : Number(cur))
                                                        if (curNum !== num) {
                                                            productsModel.setProperty(index, "price", num)
                                                            preRecordDirty = true
                                                        }
                                                    }
                                                }
                                                onEditingFinished: {
                                                    if (typeof index === "undefined" || index < 0 || index >= productsModel.count) return
                                                    var t = text.trim()
                                                    if (!isPriceValid(t) && t !== "") {
                                                        var prev = productsModel.get(index).price
                                                        productsModel.setProperty(index, "price", prev !== undefined && prev !== null && prev !== "" ? prev : "")
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
                                                    if (typeof index !== "undefined" && index >= 0 && index < productsModel.count && String(model.usage || "") !== text) {
                                                        productsModel.setProperty(index, "usage", text)
                                                        preRecordDirty = true
                                                    }
                                                }
                                                onEditingFinished: {
                                                    if (typeof index !== "undefined" && index >= 0 && index < productsModel.count && String(model.usage || "") !== text) {
                                                        productsModel.setProperty(index, "usage", text)
                                                        preRecordDirty = true
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
                                                    preRecordDirty = true
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
                                                            if (!_suppressDirtyFromLoad) preRecordDirty = true
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
                                                                property int rIdx: (parent && parent.parent && parent.parent.parent && typeof parent.parent.parent.rIdx === "number") ? parent.parent.parent.rIdx : 0
                                                                Rectangle {
                                                                    width: 120
                                                                    height: 22
                                                                    radius: 4
                                                                    color: "#3a3a3a"
                                                                    border.color: "#555"
                                                                    Text {
                                                                        text: getSelectedProductName(rIdx, 0, index)
                                                                        color: "#ffffff"
                                                                        font.pixelSize: 12
                                                                        anchors.verticalCenter: parent.verticalCenter
                                                                        anchors.left: parent.left
                                                                        anchors.right: parent.right
                                                                        anchors.leftMargin: 6
                                                                        anchors.rightMargin: 20
                                                                        elide: Text.ElideRight
                                                                    }
                                                                    TextButton {
                                                                        width: 16
                                                                        height: 16
                                                                        anchors.right: parent.right
                                                                        anchors.verticalCenter: parent.verticalCenter
                                                                        anchors.rightMargin: 3
                                                                        text: "×"
                                                                        font.pixelSize: 11
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
                                                            if (!_suppressDirtyFromLoad) preRecordDirty = true
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
                                                                property int rIdx: (parent && parent.parent && parent.parent.parent && typeof parent.parent.parent.rIdx === "number") ? parent.parent.parent.rIdx : 0
                                                                Rectangle {
                                                                    width: 120
                                                                    height: 22
                                                                    radius: 4
                                                                    color: "#3a3a3a"
                                                                    border.color: "#555"
                                                                    Text {
                                                                        text: getSelectedProductName(rIdx, 1, index)
                                                                        color: "#ffffff"
                                                                        font.pixelSize: 12
                                                                        anchors.verticalCenter: parent.verticalCenter
                                                                        anchors.left: parent.left
                                                                        anchors.right: parent.right
                                                                        anchors.leftMargin: 6
                                                                        anchors.rightMargin: 20
                                                                        elide: Text.ElideRight
                                                                    }
                                                                    TextButton {
                                                                        width: 16
                                                                        height: 16
                                                                        anchors.right: parent.right
                                                                        anchors.verticalCenter: parent.verticalCenter
                                                                        anchors.rightMargin: 3
                                                                        text: "×"
                                                                        font.pixelSize: 11
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
                                                            if (!_suppressDirtyFromLoad) preRecordDirty = true
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
                                                                property int rIdx: (parent && parent.parent && parent.parent.parent && typeof parent.parent.parent.rIdx === "number") ? parent.parent.parent.rIdx : 0
                                                                Rectangle {
                                                                    width: 120
                                                                    height: 22
                                                                    radius: 4
                                                                    color: "#3a3a3a"
                                                                    border.color: "#555"
                                                                    Text {
                                                                        text: getSelectedProductName(rIdx, 2, index)
                                                                        color: "#ffffff"
                                                                        font.pixelSize: 12
                                                                        anchors.verticalCenter: parent.verticalCenter
                                                                        anchors.left: parent.left
                                                                        anchors.right: parent.right
                                                                        anchors.leftMargin: 6
                                                                        anchors.rightMargin: 20
                                                                        elide: Text.ElideRight
                                                                    }
                                                                    TextButton {
                                                                        width: 16
                                                                        height: 16
                                                                        anchors.right: parent.right
                                                                        anchors.verticalCenter: parent.verticalCenter
                                                                        anchors.rightMargin: 3
                                                                        text: "×"
                                                                        font.pixelSize: 11
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
                onClicked: {
                    if (!preRecordDirty) { close(); return }
                    if (doSaveToDb()) close()
                }
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
        preRecordManager: preRecordWin.productsManager
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
                preRecordDirty = true
            }
        }
    }

    MessageBox {
        id: saveErrorDialog
        transientParent: preRecordWin
        boxTitle: "保存失败"
        boxMessage: ""
    }

    Dialog {
        id: savePromptDialog
        title: "保存到数据库"
        modal: true
        anchors.centerIn: parent
        width: 320
        contentItem: Item {
            width: 280
            height: 40
            Text { text: "是否保存当前修改到数据库？"; color: "#fff"; wrapMode: Text.WordWrap; anchors.fill: parent }
        }
        background: Rectangle { color: "#2c2c2c"; border.color: "#555"; radius: 8 }
        footer: Row {
            spacing: 12
            TextButton { text: "保存"; onClicked: { if (doSaveToDb()) tabRow.currentIndex = 1; savePromptDialog.close() } }
            TextButton { text: "不保存"; onClicked: { tabRow.currentIndex = 1; savePromptDialog.close() } }
        }
    }
}
