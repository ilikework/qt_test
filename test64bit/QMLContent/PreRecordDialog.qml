import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "components"

Window {
    id: preRecordWin
    width: 700
    height: 620
    flags: Qt.FramelessWindowHint
    modality: Qt.WindowModal

    property var reportLabels: [
        "毛 孔", "粉 刺", "深层色斑", "浅层色斑",
        "皱 纹", "敏感度", "褐色斑", "混合彩斑", "综合报告"
    ]
    property var productList: []

    // 好/中/差：好≤30% 中 30–70% 差>70%（供后续自动分析归类用）
    function loadData() {
        var suggestions = preRecordManager.getReportSuggestions()
        suggestionModel.clear()
        for (var s = 0; s < 9; s++) {
            var item = suggestions[s]
            if (item && typeof item === "object") {
                suggestionModel.append({
                    label: reportLabels[s],
                    good: item.good || "",
                    medium: item.medium || "",
                    bad: item.bad || ""
                })
            } else {
                suggestionModel.append({ label: reportLabels[s], good: "", medium: "", bad: "" })
            }
        }
        productList = preRecordManager.getProducts()
        if (!productList || !Array.isArray(productList)) productList = []
        productsModel.clear()
        for (var i = 0; i < productList.length; i++) {
            var p = productList[i]
            productsModel.append({
                IX: p.IX || 0,
                name: p.name || "",
                photoPath: p.photoPath || "",
                price: p.price || "",
                usage: p.usage || ""
            })
        }
    }

    function collectSuggestions() {
        var list = []
        for (var i = 0; i < suggestionModel.count; i++) {
            var r = suggestionModel.get(i)
            list.push({ good: r.good || "", medium: r.medium || "", bad: r.bad || "" })
        }
        return list
    }

    function collectProducts() {
        var list = []
        for (var k = 0; k < productsModel.count; k++) {
            var r = productsModel.get(k)
            list.push({
                IX: r.IX || 0,
                name: r.name,
                photoPath: r.photoPath,
                price: r.price,
                usage: r.usage
            })
        }
        return list
    }

    function doSave() {
        preRecordManager.setReportSuggestions(collectSuggestions())
        preRecordManager.setProducts(collectProducts())
        if (preRecordManager.save())
            close()
    }

    ListModel { id: suggestionModel }

    Rectangle {
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

        TabBar {
            id: tabBar
            anchors.top: titleText.bottom
            anchors.topMargin: 12
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(parent.width - 40, 400)

            background: Rectangle { color: "#2c2c2c"; radius: 4 }
            contentItem: RowLayout { spacing: 0 }

            TabButton {
                text: "分量报告建议"
                Layout.fillWidth: true
                contentItem: Text {
                    text: parent.text
                    color: parent.checked ? "#00aaff" : "#aaa"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.checked ? "#2c2c2c" : "transparent"
                    radius: 4
                }
            }
            TabButton {
                text: "产品/服务"
                contentItem: Text {
                    text: parent.text
                    color: parent.checked ? "#00aaff" : "#aaa"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.checked ? "#2c2c2c" : "transparent"
                    radius: 4
                }
            }
        }

        StackLayout {
            anchors.top: tabBar.bottom
            anchors.topMargin: 12
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: bottomRow.top
            anchors.margins: 16
            currentIndex: tabBar.currentIndex

            Item {
                ScrollView {
                    anchors.fill: parent
                    clip: true
                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                    contentWidth: availableWidth

                    ColumnLayout {
                        width: preRecordWin.width - 80
                        spacing: 10
                        Repeater {
                            model: suggestionModel
                            delegate: ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4
                                Text {
                                    text: model.label + "（好≤30% / 中 30–70% / 差>70%）"
                                    color: "#b0b0b0"
                                    font.pixelSize: 14
                                }
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text { text: "好"; color: "#7ec0ff"; font.pixelSize: 12 }
                                        TextArea {
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 52
                                            placeholderText: "建议文字（好）"
                                            text: model.good
                                            font.pixelSize: 13
                                            color: "#eee"
                                            background: Rectangle { color: "#2c2c2c"; border.color: "#555"; radius: 4 }
                                            onTextChanged: suggestionModel.setProperty(index, "good", text)
                                        }
                                    }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text { text: "中"; color: "#b0b0b0"; font.pixelSize: 12 }
                                        TextArea {
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 52
                                            placeholderText: "建议文字（中）"
                                            text: model.medium
                                            font.pixelSize: 13
                                            color: "#eee"
                                            background: Rectangle { color: "#2c2c2c"; border.color: "#555"; radius: 4 }
                                            onTextChanged: suggestionModel.setProperty(index, "medium", text)
                                        }
                                    }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text { text: "差"; color: "#cc6666"; font.pixelSize: 12 }
                                        TextArea {
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 52
                                            placeholderText: "建议文字（差）"
                                            text: model.bad
                                            font.pixelSize: 13
                                            color: "#eee"
                                            background: Rectangle { color: "#2c2c2c"; border.color: "#555"; radius: 4 }
                                            onTextChanged: suggestionModel.setProperty(index, "bad", text)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Item {
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8
                    Button {
                        Layout.alignment: Qt.AlignRight
                        text: "添加产品/服务"
                        font.pixelSize: 14
                        background: Rectangle {
                            radius: 8
                            color: parent.pressed ? "#0066aa" : (parent.hovered ? "#0088cc" : "#00aaff")
                        }
                        contentItem: Text { text: parent.text; color: "white"; font: parent.font; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                        onClicked: {
                            productsModel.append({ IX: 0, name: "", photoPath: "", price: "", usage: "" })
                        }
                    }
                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                        ListView {
                            id: productListView
                            model: ListModel { id: productsModel }
                            spacing: 8
                            delegate: Rectangle {
                                width: productListView.width - 20
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
                                                source: model.photoPath ? (model.photoPath.toString().indexOf("file://") === 0 ? model.photoPath : "file:///" + model.photoPath) : ""
                                                visible: model.photoPath !== ""
                                            }
                                            Text {
                                                anchors.centerIn: parent
                                                text: "无图"
                                                color: "#666"
                                                font.pixelSize: 12
                                                visible: !productImg.visible
                                            }
                                            MouseArea {
                                                anchors.fill: parent
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
                                                color: "#eee"
                                                background: Rectangle { color: "#333"; radius: 4; border.color: "#555" }
                                                onTextChanged: productsModel.setProperty(index, "name", text)
                                            }
                                            TextField {
                                                Layout.fillWidth: true
                                                placeholderText: "价格"
                                                text: model.price
                                                font.pixelSize: 14
                                                color: "#eee"
                                                background: Rectangle { color: "#333"; radius: 4; border.color: "#555" }
                                                onTextChanged: productsModel.setProperty(index, "price", text)
                                            }
                                            TextArea {
                                                Layout.fillWidth: true
                                                Layout.preferredHeight: 48
                                                placeholderText: "使用说明"
                                                text: model.usage
                                                font.pixelSize: 13
                                                color: "#eee"
                                                background: Rectangle { color: "#333"; radius: 4; border.color: "#555" }
                                                onTextChanged: productsModel.setProperty(index, "usage", text)
                                            }
                                        }
                                        Button {
                                            text: "删除"
                                            font.pixelSize: 12
                                            Layout.alignment: Qt.AlignTop
                                            background: Rectangle {
                                                radius: 6
                                                color: parent.pressed ? "#882222" : (parent.hovered ? "#aa3333" : "#663333")
                                            }
                                            contentItem: Text { text: parent.text; color: "white"; font: parent.font }
                                            onClicked: productsModel.remove(index)
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
                onClicked: doSave()
            }
            TextButton {
                text: "返回"
                width: 90
                onClicked: close()
            }
        }
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
            }
        }
    }
}
