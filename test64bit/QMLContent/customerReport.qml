import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtCharts

Item {
    id: customerReport
    signal loadPage(string page, var params)

    property string customerID: ""
    property int currentGroupID: 0
    property int curIndex: 0

    property var mainphotoes: []
    property var subphotoes: []

    property var reportLabels: [
        "毛 孔", "粉 刺", "深层色斑", "浅层色斑",
        "皱 纹", "敏感度", "褐色斑", "混合彩斑", "综合报告"
    ]
    property var resDate: [
        "2025/4/1", "2025/5/4", "2025/6/7", "2025/7/12", "2025/8/31"
    ]
    property var resDatas: [
        { name: "毛 孔", res: [10, 25, 12, 28, 14] },
        { name: "粉 刺", res: [73, 62, 71, 58, 74] },
        { name: "深层色斑", res: [30, 52, 18, 24, 46] },
        { name: "浅层色斑", res: [43, 66, 31, 42, 71] },
        { name: "皱 纹", res: [90, 72, 22, 45, 21] },
        { name: "敏感度", res: [63, 19, 46, 37, 83] },
        { name: "褐色斑", res: [12, 23, 45, 67, 89] },
        { name: "混合彩斑", res: [74, 61, 51, 53, 64] },
        { name: "综合报告", res: [30, 55, 42, 88, 74] }
    ]

    // —— 预录 / 诊断 / 产品 ——
    property bool _usePreRecordManager: typeof preRecordManager !== "undefined"
    property bool _useCustomerReportManager: typeof customerReportManager !== "undefined"
    property bool reportDirty: false
    property alias hostProductsModel: productsModel
    property var reportSuggestions: []
    property var selectedOfferings: []
    property int currentReportTier: 1
    property string diagnosticText: ""
    property var tierLabels: ["好", "中", "差"]
    property var tierColors: ["#2e7d32", "#ef8c00", "#c62828"]
    property var photoShortLabels: ["毛孔", "粉刺", "深层色斑", "浅层色斑", "皱纹", "敏感", "褐色斑", "混合彩斑"]
    property var seriesColors: [
        "#ff6b6b", "#4ecdc4", "#45b7d1", "#f9ca24",
        "#a29bfe", "#fd79a8", "#00b894", "#e17055"
    ]

    function ensureSelectedOfferings() {
        if (!selectedOfferings || !Array.isArray(selectedOfferings))
            selectedOfferings = []
        while (selectedOfferings.length < 9)
            selectedOfferings.push([[], [], []])
    }

    function setSelectedOfferingsForReport(rIdx, tIdx, indices, markDirty) {
        ensureSelectedOfferings()
        if (rIdx < 0 || rIdx >= selectedOfferings.length || tIdx < 0 || tIdx > 2)
            return
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
        if (markDirty !== false)
            reportDirty = true
    }

    function getSelectedProductName(reportIdx, tier, slotIndex) {
        ensureSelectedOfferings()
        if (reportIdx < 0 || reportIdx >= selectedOfferings.length)
            return ""
        var arr = selectedOfferings[reportIdx][tier]
        if (!arr || slotIndex < 0 || slotIndex >= arr.length)
            return ""
        var ix = arr[slotIndex]
        if (ix < 0 || ix >= productsModel.count)
            return ""
        return productsModel.get(ix).name || "未命名"
    }

    function formatProductPrice(priceVal) {
        if (priceVal === undefined || priceVal === null || priceVal === "")
            return ""
        if (typeof priceVal === "number")
            return priceVal % 1 === 0 ? String(priceVal) : priceVal.toFixed(2)
        return String(priceVal)
    }

    function normalizeProductPrice(priceVal) {
        if (priceVal === undefined || priceVal === null || priceVal === "")
            return 0
        var n = (typeof priceVal === "number") ? priceVal : Number(priceVal)
        if (isNaN(n) || n < 0)
            return 0
        return n
    }

    function appendProductToModel(p) {
        productsModel.append({
            IX: p.IX || 0,
            name: (p.name !== undefined && p.name !== null) ? String(p.name) : "",
            price: normalizeProductPrice(p.price),
            usage: (p.usage !== undefined && p.usage !== null) ? String(p.usage) : "",
            photoPath: (p.photoPath !== undefined && p.photoPath !== null) ? String(p.photoPath) : ""
        })
    }

    function getSelectedProductPrice(reportIdx, tier, slotIndex) {
        ensureSelectedOfferings()
        if (reportIdx < 0 || reportIdx >= selectedOfferings.length)
            return ""
        var arr = selectedOfferings[reportIdx][tier]
        if (!arr || slotIndex < 0 || slotIndex >= arr.length)
            return ""
        var ix = arr[slotIndex]
        if (ix < 0 || ix >= productsModel.count)
            return ""
        return formatProductPrice(productsModel.get(ix).price)
    }

    function getSelectedProductIx(reportIdx, tier, slotIndex) {
        ensureSelectedOfferings()
        if (reportIdx < 0 || reportIdx >= selectedOfferings.length)
            return -1
        var arr = selectedOfferings[reportIdx][tier]
        if (!arr || slotIndex < 0 || slotIndex >= arr.length)
            return -1
        return arr[slotIndex]
    }

    function latestScore(reportIdx) {
        if (reportIdx < 0 || reportIdx >= resDatas.length)
            return 0
        var arr = resDatas[reportIdx].res
        if (!arr || arr.length === 0)
            return 0
        return arr[arr.length - 1]
    }

    function scoreToTier(score) {
        if (score <= 33)
            return 0
        if (score <= 66)
            return 1
        return 2
    }

    function comprehensiveScore() {
        var sum = 0
        for (var i = 0; i < 8; i++)
            sum += latestScore(i)
        return sum / 8
    }

    function autoTierForReport(reportIdx) {
        if (reportIdx === 8)
            return scoreToTier(comprehensiveScore())
        return scoreToTier(latestScore(reportIdx))
    }

    function memoForTier(reportIdx, tier) {
        if (!reportSuggestions || reportSuggestions.length <= reportIdx)
            return ""
        var rec = reportSuggestions[reportIdx]
        if (tier === 0)
            return rec.goodMemo || ""
        if (tier === 1)
            return rec.mediumMemo || ""
        return rec.badMemo || ""
    }

    function offeringIndicesFromTemplateIxs(ixs) {
        var indices = []
        if (!ixs)
            return indices
        for (var k = 0; k < ixs.length; k++) {
            for (var j = 0; j < productsModel.count; j++) {
                if (productsModel.get(j).IX === ixs[k]) {
                    indices.push(j)
                    break
                }
            }
        }
        return indices
    }

    function getOfferingTemplateIxs(reportIdx, tier) {
        ensureSelectedOfferings()
        var arr = selectedOfferings[reportIdx][tier]
        var ixs = []
        if (!arr)
            return ixs
        for (var k = 0; k < arr.length; k++) {
            var pi = arr[k]
            if (pi >= 0 && pi < productsModel.count) {
                var ix = productsModel.get(pi).IX
                if (ix)
                    ixs.push(ix)
            }
        }
        return ixs
    }

    function applyTierToReport(reportIdx, tier, fromLoad) {
        currentReportTier = tier
        diagnosticText = memoForTier(reportIdx, tier)
        if (!_usePreRecordManager)
            return
        var ixs = preRecordManager.getReportOfferings(reportIdx, tier)
        setSelectedOfferingsForReport(reportIdx, tier, offeringIndicesFromTemplateIxs(ixs), false)
        if (!fromLoad)
            reportDirty = true
    }

    function loadActiveReport() {
        var idx = tabButtons.selectedIndex
        if (_useCustomerReportManager && customerID !== "" && currentGroupID > 0) {
            var data = customerReportManager.loadReport(customerID, currentGroupID, idx)
            if (data.hasSaved) {
                currentReportTier = data.tier
                diagnosticText = data.memo || ""
                setSelectedOfferingsForReport(
                    idx, data.tier, offeringIndicesFromTemplateIxs(data.offeringIxs), false)
                reportDirty = false
                return
            }
        }
        applyTierToReport(idx, autoTierForReport(idx), true)
        reportDirty = false
    }

    function saveActiveReport() {
        if (!_useCustomerReportManager || customerID === "" || currentGroupID <= 0)
            return
        var idx = tabButtons.selectedIndex
        var ixs = getOfferingTemplateIxs(idx, currentReportTier)
        if (customerReportManager.saveReport(
                customerID, currentGroupID, idx, currentReportTier, diagnosticText, ixs))
            reportDirty = false
    }

    function loadPreRecordData() {
        if (!_usePreRecordManager)
            return
        productsModel.clear()
        var products = preRecordManager.getProducts()
        for (var i = 0; i < products.length; i++)
            appendProductToModel(products[i])
        reportSuggestions = preRecordManager.getReportSuggestions()
        ensureSelectedOfferings()
        loadActiveReport()
    }

    ListModel { id: productsModel }

    function updateAllLineCharts() {
        var displaySeries = [s1, s2, s3, s4, s5, s6, s7, s8]
        var printSeries = [ps1, ps2, ps3, ps4, ps5, ps6, ps7, ps8]
        var allSeries = displaySeries.concat(printSeries)
        for (var s = 0; s < allSeries.length; s++)
            allSeries[s].clear()
        for (var i = 0; i < resDate.length; i++) {
            for (var j = 0; j < 8; j++) {
                displaySeries[j].append(i, resDatas[j].res[i])
                printSeries[j].append(i, resDatas[j].res[i])
            }
        }
    }

    function initChartAxes() {
        for (var i = 0; i < resDate.length; i++) {
            axisX.append(resDate[i], i)
            axisPrintX.append(resDate[i], i)
        }
        axisX.max = resDate.length
        axisPrintX.max = resDate.length
    }

    property int printReportIdx: 8

    function reportTitle(idx) {
        if (idx < 0 || idx >= reportLabels.length)
            return "检测报告"
        return String(reportLabels[idx]).replace(/\s/g, "") + "检测报告"
    }

    function openProductPickerForActiveReport() {
        offeringPickerDialog.openFor(tabButtons.selectedIndex, currentReportTier)
    }

    function grabAndPrint(sheet) {
        Qt.callLater(function() {
            sheet.grabToImage(function(result) {
                if (!result || !result.url)
                    return
                var path = applicationDirPath + "/report_print.png"
                if (result.saveToFile(path))
                    printHelper.printImage(path)
            })
        })
    }

    function updatePrintBarChart(reportIdx) {
        printBarSeries.clear()
        if (reportIdx < 0 || reportIdx >= resDatas.length)
            return
        var barSet = printBarSeries.append(
            resDatas[reportIdx].name,
            resDatas[reportIdx].res
        )
        if (barSet && reportIdx < seriesColors.length)
            barSet.color = seriesColors[reportIdx]
    }

    function initPrintBarAxes() {
        for (var i = 0; i < resDate.length; i++)
            axisPrintBarX.append(resDate[i], i)
        axisPrintBarX.max = resDate.length
    }

    function printActiveReport() {
        if (typeof printHelper === "undefined")
            return
        printReportIdx = tabButtons.selectedIndex
        if (printReportIdx === 8) {
            updateAllLineCharts()
            grabAndPrint(reportSheet)
        } else {
            updatePrintBarChart(printReportIdx)
            grabAndPrint(reportSheetSingle)
        }
    }

    function formatToday() {
        var d = new Date()
        return d.getFullYear() + "/" + (d.getMonth() + 1) + "/" + d.getDate()
    }

    Component.onCompleted: {
        if (customerID !== "")
            subphotoes = analyseModule.loadSub(currentGroupID)
        loadPreRecordData()
        initChartAxes()
        initBarChartAxes()
        initPrintBarAxes()
        updateAllLineCharts()
    }

    function initBarChartAxes() {
        for (var i = 0; i < resDate.length; i++)
            axisBarX.append(resDate[i], i)
        axisBarX.max = resDate.length
    }

    Rectangle {
        anchors.fill: parent
        color: "#0e1f30"

        ColumnLayout {
            anchors.fill: parent
            spacing: 8

            RowLayout {
                id: header
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                spacing: 10

                Rectangle {
                    width: 100
                    height: 100
                    radius: 8
                    border.color: "#6aaaff"
                    color: "transparent"
                    Image {
                        anchors.fill: parent
                        anchors.margins: 4
                        fillMode: Image.PreserveAspectFit
                    }
                }

                Text {
                    text: customerID
                    color: "white"
                    font.pixelSize: 36
                    verticalAlignment: Text.AlignVCenter
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: "客户报告"
                    color: "#7dc9ff"
                    font.pixelSize: 46
                }
            }

            RowLayout {
                id: tabButtons
                Layout.fillWidth: true
                Layout.preferredHeight: 70
                Layout.alignment: Qt.AlignCenter
                spacing: 8
                property int selectedIndex: 8

                Repeater {
                    model: reportLabels
                    delegate: Rectangle {
                        width: 170
                        height: 60
                        radius: 6
                        border.color: index === tabButtons.selectedIndex ? "#90c8ff" : "#466080"
                        border.width: 2
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: index === tabButtons.selectedIndex ? "#668bb8" : "#345" }
                            GradientStop { position: 1.0; color: index === tabButtons.selectedIndex ? "#446a9c" : "#233" }
                        }
                        Text {
                            anchors.centerIn: parent
                            text: modelData
                            color: "white"
                            font.pixelSize: 26
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                tabButtons.selectedIndex = index
                                if (index === 8) {
                                    viewStack.currentIndex = 0
                                } else {
                                    viewStack.currentIndex = 1
                                    chartBar.updatebar()
                                }
                                loadActiveReport()
                            }
                        }
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: 40
                Layout.rightMargin: 40

                StackLayout {
                    id: viewStack
                    anchors.fill: parent
                    currentIndex: 0
                    clip: true

                    // —— 综合报告（屏幕显示：大图 + 折线图为主） ——
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 8

                        RowLayout {
                            id: photoArea
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignHCenter
                            spacing: 20

                            Repeater {
                                model: subphotoes
                                delegate: Column {
                                    spacing: 10
                                    Rectangle {
                                        width: 184
                                        height: 124
                                        radius: 8
                                        border.color: "#7ec0ff"
                                        color: "transparent"
                                        Image {
                                            x: 2
                                            y: 2
                                            width: 90
                                            height: 120
                                            source: modelData.photoL
                                            fillMode: Image.PreserveAspectFit
                                        }
                                        Image {
                                            x: 92
                                            y: 2
                                            width: 90
                                            height: 120
                                            source: modelData.photoR
                                            fillMode: Image.PreserveAspectFit
                                        }
                                    }
                                    Text {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        text: index < photoShortLabels.length ? photoShortLabels[index] : ""
                                        color: "white"
                                        font.pixelSize: 28
                                    }
                                }
                            }
                        }

                        ChartView {
                            id: chart
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.minimumHeight: 200
                            margins.left: 10
                            margins.right: 10
                            margins.top: 10
                            margins.bottom: 10
                            antialiasing: true
                            backgroundColor: "#1a3048"
                            legend.visible: true
                            legend.alignment: Qt.AlignBottom
                            legend.labelColor: "#ffffff"

                            CategoryAxis {
                                id: axisX
                                labelsPosition: CategoryAxis.AxisLabelsPositionOnValue
                                labelsColor: "#cccccc"
                            }
                            ValueAxis {
                                id: axisY
                                min: 0
                                max: 100
                                labelsColor: "#cccccc"
                            }

                            LineSeries { id: s1; axisX: axisX; axisY: axisY; name: "毛孔"; width: 2; color: seriesColors[0] }
                            LineSeries { id: s2; axisX: axisX; axisY: axisY; name: "粉刺"; width: 2; color: seriesColors[1] }
                            LineSeries { id: s3; axisX: axisX; axisY: axisY; name: "深层色斑"; width: 2; color: seriesColors[2] }
                            LineSeries { id: s4; axisX: axisX; axisY: axisY; name: "浅层色斑"; width: 2; color: seriesColors[3] }
                            LineSeries { id: s5; axisX: axisX; axisY: axisY; name: "皱纹"; width: 2; color: seriesColors[4] }
                            LineSeries { id: s6; axisX: axisX; axisY: axisY; name: "敏感"; width: 2; color: seriesColors[5] }
                            LineSeries { id: s7; axisX: axisX; axisY: axisY; name: "褐色斑"; width: 2; color: seriesColors[6] }
                            LineSeries { id: s8; axisX: axisX; axisY: axisY; name: "混合彩斑"; width: 2; color: seriesColors[7] }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 34
                            spacing: 10

                            Text {
                                text: "诊断等级"
                                color: "#cccccc"
                                font.pixelSize: 14
                            }
                            Repeater {
                                model: tierLabels
                                delegate: Rectangle {
                                    width: 48
                                    height: 28
                                    radius: 4
                                    color: currentReportTier === index ? tierColors[index] : "#2a3a4a"
                                    border.color: currentReportTier === index ? tierColors[index] : "#555"
                                    Text {
                                        anchors.centerIn: parent
                                        text: modelData
                                        color: "#ffffff"
                                        font.pixelSize: 13
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: applyTierToReport(8, index, false)
                                    }
                                }
                            }
                            Text {
                                text: "综合指数 " + comprehensiveScore().toFixed(0)
                                color: "#999999"
                                font.pixelSize: 12
                            }
                            Item { Layout.fillWidth: true }
                            Rectangle {
                                width: 120
                                height: 28
                                radius: 4
                                color: screenProductBtnMa.pressed ? "#2a4a6a" : "#204060"
                                border.color: "#7cc0ff"
                                Text {
                                    anchors.centerIn: parent
                                    text: "选择产品/服务"
                                    color: "#cce8ff"
                                    font.pixelSize: 12
                                }
                                MouseArea {
                                    id: screenProductBtnMa
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: openProductPickerForActiveReport()
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 128
                            spacing: 16

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 128
                                radius: 6
                                color: "#1a3048"
                                border.color: "#466080"
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 4
                                    Text {
                                        text: "诊断建议（约100字）"
                                        color: "#7dc9ff"
                                        font.pixelSize: 14
                                        font.bold: true
                                    }
                                    TextArea {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 68
                                        text: diagnosticText
                                        wrapMode: TextArea.Wrap
                                        color: "#ffffff"
                                        font.pixelSize: 13
                                        //maximumLength: 100
                                        placeholderText: "预录建议自动填入，可修改"
                                        placeholderTextColor: "#888"
                                        background: Rectangle { color: "#243548"; radius: 4; border.color: "#555" }
                                        onTextChanged: {
                                            if (diagnosticText !== text) {
                                                diagnosticText = text
                                                reportDirty = true
                                            }
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 128
                                radius: 6
                                color: "#1a3048"
                                border.color: "#466080"
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 4
                                    Text {
                                        text: "推荐产品/服务（5–10项）"
                                        color: "#7dc9ff"
                                        font.pixelSize: 14
                                        font.bold: true
                                    }
                                    Item {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 88
                                        clip: true
                                        Flow {
                                            width: parent.width
                                            spacing: 6
                                            Repeater {
                                                model: selectedOfferings[8] && selectedOfferings[8][currentReportTier]
                                                    ? selectedOfferings[8][currentReportTier].length : 0
                                                delegate: Rectangle {
                                                    height: 24
                                                    radius: 4
                                                    color: "#2a4a6a"
                                                    border.color: "#7cc0ff"
                                                    property string priceStr: getSelectedProductPrice(8, currentReportTier, index)
                                                    width: screenProductLabel.implicitWidth + 16
                                                    Text {
                                                        id: screenProductLabel
                                                        anchors.centerIn: parent
                                                        text: {
                                                            var n = getSelectedProductName(8, currentReportTier, index)
                                                            var p = parent.priceStr
                                                            return p !== "" ? (n + " ¥" + p) : n
                                                        }
                                                        color: "#cce8ff"
                                                        font.pixelSize: 12
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    Text {
                                        visible: !(selectedOfferings[8] && selectedOfferings[8][currentReportTier]
                                            && selectedOfferings[8][currentReportTier].length > 0)
                                        text: "未选择"
                                        color: "#888"
                                        font.pixelSize: 12
                                    }
                                }
                            }
                        }
                    }

                    // —— 单项报告 ——
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 8

                        // 上区：照片(3:4 竖版，紧贴边框) + 柱图占满剩余宽度
                        Item {
                            id: singleTopArea
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.minimumHeight: 100

                            readonly property real minChartW: 280
                            readonly property real photoAspect: 3 / 4   // 宽:高，与缩略图 90×120 一致
                            readonly property real photoPad: 4
                            // 两图总宽 ≈ 2×(h×3/4+pad)；布局未完成时用 fallback
                            readonly property real photoH: {
                                var availH = height > 0 ? height : 220
                                var availW = width > 0 ? width : 900
                                // 总宽 ≈ 2×(h×3/4+pad) + minChartW + 间距
                                return Math.min(
                                    availH,
                                    Math.max(72, (availW - minChartW - 24 - 2 * photoPad) / (2 * photoAspect))
                                )
                            }

                            RowLayout {
                                anchors.top: parent.top
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: singleTopArea.photoH
                                spacing: 28

                                Rectangle {
                                    id: mainImgL
                                    Layout.preferredHeight: parent.height
                                    Layout.maximumHeight: parent.height
                                    Layout.preferredWidth: imgL.width + singleTopArea.photoPad
                                    Layout.maximumWidth: imgL.width + singleTopArea.photoPad
                                    radius: 8
                                    color: "#222"
                                    border.color: "#ffb300"
                                    Image {
                                        id: imgL
                                        anchors.centerIn: parent
                                        height: parent.height - 2
                                        width: height * singleTopArea.photoAspect
                                        fillMode: Image.PreserveAspectFit
                                        source: (subphotoes && tabButtons.selectedIndex < subphotoes.length)
                                            ? subphotoes[tabButtons.selectedIndex].photoL : ""
                                    }
                                }
                                Rectangle {
                                    id: mainImgR
                                    Layout.preferredHeight: parent.height
                                    Layout.maximumHeight: parent.height
                                    Layout.preferredWidth: imgR.width + singleTopArea.photoPad
                                    Layout.maximumWidth: imgR.width + singleTopArea.photoPad
                                    radius: 8
                                    color: "#222"
                                    border.color: "#ffb300"
                                    Image {
                                        id: imgR
                                        anchors.centerIn: parent
                                        height: parent.height - 2
                                        width: height * singleTopArea.photoAspect
                                        fillMode: Image.PreserveAspectFit
                                        source: (subphotoes && tabButtons.selectedIndex < subphotoes.length)
                                            ? subphotoes[tabButtons.selectedIndex].photoR : ""
                                    }
                                }
                                ChartView {
                                    id: chartBar
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    Layout.minimumWidth: singleTopArea.minChartW
                                    antialiasing: true

                                    function updatebar() {
                                        barSeries.clear()
                                        var idx = tabButtons.selectedIndex
                                        var barSet = barSeries.append(
                                            resDatas[idx].name,
                                            resDatas[idx].res
                                        )
                                        if (barSet && idx < seriesColors.length)
                                            barSet.color = seriesColors[idx]
                                    }

                                    CategoryAxis {
                                        id: axisBarX
                                        labelsPosition: CategoryAxis.AxisLabelsPositionOnValue
                                        labelsColor: "#cccccc"
                                    }
                                    ValueAxis {
                                        id: axisBarY
                                        min: 0
                                        max: 100
                                        labelsColor: "#cccccc"
                                    }
                                    BarSeries {
                                        id: barSeries
                                        axisX: axisBarX
                                        axisY: axisBarY
                                    }
                                    legend.visible: true
                                    legend.alignment: Qt.AlignBottom
                                    legend.labelColor: "#ffffff"
                                    backgroundColor: "#1a3048"
                                    Component.onCompleted: updatebar()
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            Layout.topMargin: 4
                            spacing: 10
                            Text {
                                text: "诊断等级"
                                color: "#cccccc"
                                font.pixelSize: 14
                            }
                            Repeater {
                                model: tierLabels
                                delegate: Rectangle {
                                    width: 48
                                    height: 28
                                    radius: 4
                                    color: currentReportTier === index ? tierColors[index] : "#2a3a4a"
                                    border.color: currentReportTier === index ? tierColors[index] : "#555"
                                    Text {
                                        anchors.centerIn: parent
                                        text: modelData
                                        color: "#ffffff"
                                        font.pixelSize: 13
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: applyTierToReport(tabButtons.selectedIndex, index, false)
                                    }
                                }
                            }
                            Text {
                                text: "指数 " + latestScore(tabButtons.selectedIndex).toFixed(0)
                                color: "#999999"
                                font.pixelSize: 12
                            }
                            Item { Layout.fillWidth: true }
                            Rectangle {
                                width: 120
                                height: 28
                                radius: 4
                                color: singleProductBtnMa.pressed ? "#2a4a6a" : "#204060"
                                border.color: "#7cc0ff"
                                Text {
                                    anchors.centerIn: parent
                                    text: "选择产品/服务"
                                    color: "#cce8ff"
                                    font.pixelSize: 12
                                }
                                MouseArea {
                                    id: singleProductBtnMa
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: openProductPickerForActiveReport()
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 128
                            spacing: 16

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 128
                                radius: 6
                                color: "#1a3048"
                                border.color: "#466080"
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 4
                                    Text {
                                        text: "诊断建议（约100字）"
                                        color: "#7dc9ff"
                                        font.pixelSize: 14
                                        font.bold: true
                                    }
                                    TextArea {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 68
                                        text: diagnosticText
                                        wrapMode: TextArea.Wrap
                                        color: "#ffffff"
                                        font.pixelSize: 13
                                        placeholderText: "预录建议自动填入，可修改"
                                        placeholderTextColor: "#888"
                                        background: Rectangle { color: "#243548"; radius: 4; border.color: "#555" }
                                        onTextChanged: {
                                            if (diagnosticText !== text) {
                                                diagnosticText = text
                                                reportDirty = true
                                            }
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 128
                                radius: 6
                                color: "#1a3048"
                                border.color: "#466080"
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 4
                                    Text {
                                        text: "推荐产品/服务（5–10项）"
                                        color: "#7dc9ff"
                                        font.pixelSize: 14
                                        font.bold: true
                                    }
                                    Item {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 88
                                        clip: true
                                        Flow {
                                            width: parent.width
                                            spacing: 6
                                            Repeater {
                                                model: {
                                                    var idx = tabButtons.selectedIndex
                                                    return (selectedOfferings[idx] && selectedOfferings[idx][currentReportTier])
                                                        ? selectedOfferings[idx][currentReportTier].length : 0
                                                }
                                                delegate: Rectangle {
                                                    height: 24
                                                    radius: 4
                                                    color: "#2a4a6a"
                                                    border.color: "#7cc0ff"
                                                    property int rIdx: tabButtons.selectedIndex
                                                    property string priceStr: getSelectedProductPrice(rIdx, currentReportTier, index)
                                                    width: singleProductLabel.implicitWidth + 16
                                                    Text {
                                                        id: singleProductLabel
                                                        anchors.centerIn: parent
                                                        text: {
                                                            var n = getSelectedProductName(rIdx, currentReportTier, index)
                                                            var p = parent.priceStr
                                                            return p !== "" ? (n + " ¥" + p) : n
                                                        }
                                                        color: "#cce8ff"
                                                        font.pixelSize: 12
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    Text {
                                        visible: {
                                            var idx = tabButtons.selectedIndex
                                            return !(selectedOfferings[idx] && selectedOfferings[idx][currentReportTier]
                                                && selectedOfferings[idx][currentReportTier].length > 0)
                                        }
                                        text: "未选择"
                                        color: "#888"
                                        font.pixelSize: 12
                                    }
                                }
                            }
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 100
                Layout.alignment: Qt.AlignHCenter
                spacing: 50

                Repeater {
                    model: [
                        { text: "二维码", icon: "qrc:/images/qr.svg" },
                        { text: "邮件", icon: "qrc:/images/mail.svg" },
                        { text: "彩点显示", icon: "qrc:/images/spot.svg" },
                        { text: "A4打印", icon: "qrc:/images/print.svg" },
                        { text: "保存", icon: "" },
                        { text: "HOME", icon: "qrc:/images/exit_icon.svg" }
                    ]

                    delegate: Column {
                        spacing: 8
                        Rectangle {
                            width: 80
                            height: 80
                            radius: 10
                            color: "#204060"
                            border.color: (modelData.text === "保存" && reportDirty) ? "#ffcc66" : "#7cc0ff"
                            border.width: (modelData.text === "保存" && reportDirty) ? 2 : 1
                            Image {
                                visible: modelData.icon !== ""
                                anchors.centerIn: parent
                                width: 46
                                height: 46
                                source: modelData.icon !== "" ? Qt.resolvedUrl(modelData.icon) : ""
                                fillMode: Image.PreserveAspectFit
                            }
                            Text {
                                visible: modelData.icon === ""
                                anchors.centerIn: parent
                                text: "保存"
                                color: reportDirty ? "#ffcc66" : "white"
                                font.pixelSize: 22
                                font.bold: reportDirty
                            }
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (index === 3)
                                        printActiveReport()
                                    else if (index === 4)
                                        saveActiveReport()
                                    else if (index === 5)
                                        loadPage("logo.qml", {})
                                }
                            }
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: modelData.text
                            color: "white"
                            font.pixelSize: 22
                        }
                    }
                }
            }
        }
    }

    // —— A4 打印专用（屏幕外渲染，打印时抓取） ——
    Item {
        id: printHost
        x: -10000
        y: 0
        width: 794
        height: 1123

        Rectangle {
            id: reportSheet
            anchors.fill: parent
            color: "#ffffff"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 6

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    ColumnLayout {
                        spacing: 2
                        Text {
                            text: customerID
                            color: "#222222"
                            font.pixelSize: 20
                            font.bold: true
                        }
                        Text {
                            text: "综合皮肤检测报告"
                            color: "#555555"
                            font.pixelSize: 13
                        }
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: formatToday()
                        color: "#666666"
                        font.pixelSize: 12
                    }
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: "#dddddd" }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Repeater {
                        model: subphotoes
                        delegate: ColumnLayout {
                            spacing: 2
                            Layout.preferredWidth: 90
                            Rectangle {
                                width: 88
                                height: 60
                                radius: 4
                                border.color: "#aaaaaa"
                                color: "#fafafa"
                                Image {
                                    x: 1; y: 1; width: 42; height: 58
                                    source: modelData.photoL
                                    fillMode: Image.PreserveAspectFit
                                }
                                Image {
                                    x: 45; y: 1; width: 42; height: 58
                                    source: modelData.photoR
                                    fillMode: Image.PreserveAspectFit
                                }
                            }
                            Text {
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                text: index < photoShortLabels.length ? photoShortLabels[index] : ""
                                color: "#333333"
                                font.pixelSize: 9
                            }
                        }
                    }
                }

                ChartView {
                    id: printChart
                    Layout.fillWidth: true
                    Layout.preferredHeight: 300
                    margins.left: 4
                    margins.right: 4
                    margins.top: 4
                    margins.bottom: 4
                    antialiasing: true
                    backgroundColor: "#ffffff"
                    legend.visible: true
                    legend.alignment: Qt.AlignBottom
                    legend.labelColor: "#333333"

                    CategoryAxis {
                        id: axisPrintX
                        labelsPosition: CategoryAxis.AxisLabelsPositionOnValue
                        labelsColor: "#444444"
                    }
                    ValueAxis {
                        id: axisPrintY
                        min: 0
                        max: 100
                        labelsColor: "#444444"
                    }

                    LineSeries { id: ps1; axisX: axisPrintX; axisY: axisPrintY; name: "毛孔"; width: 2; color: seriesColors[0] }
                    LineSeries { id: ps2; axisX: axisPrintX; axisY: axisPrintY; name: "粉刺"; width: 2; color: seriesColors[1] }
                    LineSeries { id: ps3; axisX: axisPrintX; axisY: axisPrintY; name: "深层色斑"; width: 2; color: seriesColors[2] }
                    LineSeries { id: ps4; axisX: axisPrintX; axisY: axisPrintY; name: "浅层色斑"; width: 2; color: seriesColors[3] }
                    LineSeries { id: ps5; axisX: axisPrintX; axisY: axisPrintY; name: "皱纹"; width: 2; color: seriesColors[4] }
                    LineSeries { id: ps6; axisX: axisPrintX; axisY: axisPrintY; name: "敏感"; width: 2; color: seriesColors[5] }
                    LineSeries { id: ps7; axisX: axisPrintX; axisY: axisPrintY; name: "褐色斑"; width: 2; color: seriesColors[6] }
                    LineSeries { id: ps8; axisX: axisPrintX; axisY: axisPrintY; name: "混合彩斑"; width: 2; color: seriesColors[7] }
                }

                Text {
                    Layout.fillWidth: true
                    text: "诊断等级：" + tierLabels[currentReportTier]
                        + "　综合指数：" + comprehensiveScore().toFixed(0)
                    color: "#333333"
                    font.pixelSize: 13
                    font.bold: true
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Text {
                        text: "诊断建议"
                        color: "#333333"
                        font.pixelSize: 12
                        font.bold: true
                    }
                    Text {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 48
                        text: diagnosticText
                        wrapMode: Text.Wrap
                        color: "#222222"
                        font.pixelSize: 11
                        maximumLineCount: 4
                        elide: Text.ElideRight
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Text {
                        text: "推荐产品/服务"
                        color: "#333333"
                        font.pixelSize: 12
                        font.bold: true
                    }
                    Flow {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 88
                        spacing: 4
                        Repeater {
                            model: selectedOfferings[printReportIdx] && selectedOfferings[printReportIdx][currentReportTier]
                                ? selectedOfferings[printReportIdx][currentReportTier].length : 0
                            delegate: Rectangle {
                                height: 22
                                radius: 3
                                color: "#eef6ff"
                                border.color: "#aaccee"
                                property string priceStr: getSelectedProductPrice(printReportIdx, currentReportTier, index)
                                width: printProductLabel.implicitWidth + 12
                                Text {
                                    id: printProductLabel
                                    anchors.centerIn: parent
                                    text: {
                                        var n = getSelectedProductName(printReportIdx, currentReportTier, index)
                                        var p = parent.priceStr
                                        return p !== "" ? (n + " ¥" + p) : n
                                    }
                                    color: "#224466"
                                    font.pixelSize: 10
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // —— 单项报告 A4 打印（屏外） ——
    Item {
        id: printHostSingle
        x: -10000
        y: 1200
        width: 794
        height: 1123

        Rectangle {
            id: reportSheetSingle
            anchors.fill: parent
            color: "#ffffff"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    Text {
                        text: customerID
                        color: "#222222"
                        font.pixelSize: 20
                        font.bold: true
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: reportTitle(printReportIdx)
                        color: "#333333"
                        font.pixelSize: 16
                        font.bold: true
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: formatToday()
                        color: "#666666"
                        font.pixelSize: 12
                    }
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: "#dddddd" }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 280
                    spacing: 12

                    RowLayout {
                        Layout.preferredWidth: 280
                        Layout.fillHeight: true
                        spacing: 8
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 4
                            border.color: "#aaaaaa"
                            color: "#fafafa"
                            Image {
                                anchors.fill: parent
                                anchors.margins: 4
                                fillMode: Image.PreserveAspectFit
                                source: (subphotoes && printReportIdx < subphotoes.length)
                                    ? subphotoes[printReportIdx].photoL : ""
                            }
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 4
                            border.color: "#aaaaaa"
                            color: "#fafafa"
                            Image {
                                anchors.fill: parent
                                anchors.margins: 4
                                fillMode: Image.PreserveAspectFit
                                source: (subphotoes && printReportIdx < subphotoes.length)
                                    ? subphotoes[printReportIdx].photoR : ""
                            }
                        }
                    }

                    ChartView {
                        id: printBarChart
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        margins.left: 4
                        margins.right: 4
                        margins.top: 4
                        margins.bottom: 4
                        antialiasing: true
                        backgroundColor: "#ffffff"
                        legend.visible: true
                        legend.alignment: Qt.AlignBottom
                        legend.labelColor: "#333333"

                        CategoryAxis {
                            id: axisPrintBarX
                            labelsPosition: CategoryAxis.AxisLabelsPositionOnValue
                            labelsColor: "#444444"
                        }
                        ValueAxis {
                            id: axisPrintBarY
                            min: 0
                            max: 100
                            labelsColor: "#444444"
                        }
                        BarSeries {
                            id: printBarSeries
                            axisX: axisPrintBarX
                            axisY: axisPrintBarY
                        }
                    }
                }

                Text {
                    Layout.fillWidth: true
                    text: "诊断等级：" + tierLabels[currentReportTier]
                        + "　指数：" + latestScore(printReportIdx).toFixed(0)
                    color: "#333333"
                    font.pixelSize: 13
                    font.bold: true
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 16

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 4
                        Text {
                            text: "诊断建议"
                            color: "#333333"
                            font.pixelSize: 12
                            font.bold: true
                        }
                        Text {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            text: diagnosticText
                            wrapMode: Text.Wrap
                            color: "#222222"
                            font.pixelSize: 11
                            maximumLineCount: 6
                            elide: Text.ElideRight
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 4
                        Text {
                            text: "推荐产品/服务"
                            color: "#333333"
                            font.pixelSize: 12
                            font.bold: true
                        }
                        Flow {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 4
                            Repeater {
                                model: selectedOfferings[printReportIdx] && selectedOfferings[printReportIdx][currentReportTier]
                                    ? selectedOfferings[printReportIdx][currentReportTier].length : 0
                                delegate: Rectangle {
                                    height: 22
                                    radius: 3
                                    color: "#eef6ff"
                                    border.color: "#aaccee"
                                    property string priceStr: getSelectedProductPrice(printReportIdx, currentReportTier, index)
                                    width: printSingleProductLabel.implicitWidth + 12
                                    Text {
                                        id: printSingleProductLabel
                                        anchors.centerIn: parent
                                        text: {
                                            var n = getSelectedProductName(printReportIdx, currentReportTier, index)
                                            var p = parent.priceStr
                                            return p !== "" ? (n + " ¥" + p) : n
                                        }
                                        color: "#224466"
                                        font.pixelSize: 10
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    OfferingPickerDialog {
        id: offeringPickerDialog
        host: customerReport
        preRecordManager: _usePreRecordManager ? preRecordManager : null
        transientParent: customerReport.Window.window
    }
}
