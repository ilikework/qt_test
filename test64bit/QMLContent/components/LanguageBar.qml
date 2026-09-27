import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic as Basic
import QtQuick.Layouts

RowLayout {
    id: root
    spacing: 8

    readonly property int _i18nRev: appTranslator ? appTranslator.revision : 0

    Image {
        source: "qrc:/images/language_icon.svg"
        width: 22
        height: 22
        fillMode: Image.PreserveAspectFit
        Layout.alignment: Qt.AlignVCenter
    }

    Basic.ComboBox {
        id: langBox
        implicitWidth: 200
        Layout.preferredHeight: 32
        Layout.alignment: Qt.AlignVCenter
        model: langModel
        textRole: "label"
        valueRole: "code"

        background: Rectangle {
            radius: 6
            color: "#2a2a2a"
            border.color: langBox.activeFocus ? "#7cc0ff" : "#555555"
            border.width: 1
        }

        contentItem: Text {
            text: langBox.displayText
            color: "#eeeeee"
            font.pixelSize: 13
            verticalAlignment: Text.AlignVCenter
            leftPadding: 10
            elide: Text.ElideRight
        }

        delegate: Basic.ItemDelegate {
            width: langBox.width
            height: 30
            contentItem: Text {
                text: model.label
                color: highlighted ? "#ffffff" : "#dddddd"
                font.pixelSize: 13
                verticalAlignment: Text.AlignVCenter
                leftPadding: 10
            }
            background: Rectangle {
                color: highlighted ? "#3a5f8a" : "transparent"
            }
        }

        popup: Basic.Popup {
            y: langBox.height + 2
            width: langBox.width
            implicitHeight: contentItem.implicitHeight
            padding: 4

            contentItem: ListView {
                clip: true
                implicitHeight: Math.min(contentHeight, 240)
                model: langBox.popup.visible ? langBox.delegateModel : null
                currentIndex: langBox.highlightedIndex
                ScrollIndicator.vertical: ScrollIndicator { }
            }

            background: Rectangle {
                color: "#2a2a2a"
                border.color: "#555555"
                radius: 6
            }
        }

        onActivated: {
            if (!appTranslator)
                return
            var row = langModel.get(currentIndex)
            if (row)
                appTranslator.setLanguage(row.code)
        }

        function syncIndex() {
            if (!appTranslator)
                return
            var pref = appTranslator.languagePreference
            for (var i = 0; i < langModel.count; ++i) {
                if (langModel.get(i).code === pref) {
                    currentIndex = i
                    return
                }
            }
        }

        Component.onCompleted: syncIndex()
    }

    ListModel {
        id: langModel

        function rebuild() {
            var _ = root._i18nRev
            clear()
            if (!appTranslator)
                return
            var codes = appTranslator.languageOptions()
            for (var i = 0; i < codes.length; ++i) {
                var code = codes[i]
                var label = code === "auto"
                        ? appTranslator.translateText("自动（跟随系统）")
                        : appTranslator.languageDisplayName(code)
                append({ code: code, label: label })
            }
        }
    }

    Component.onCompleted: langModel.rebuild()

    Connections {
        target: appTranslator
        function onLanguageChanged() {
            langModel.rebuild()
            langBox.syncIndex()
        }
    }
}
