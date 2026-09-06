import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootDatePicker
    property date selectedDate: new Date()
    width: 250
    height: 40
    signal dateSelected(date d)

    function handleDateSelected(d) {
        dateField.text = Qt.formatDate(d, "yyyy-MM-dd")
    }

    RowLayout {
        anchors.fill: parent
        spacing: 6

        TextField {
            id: dateField
            Layout.fillWidth: true
            readOnly: true
            text: Qt.formatDate(rootDatePicker.selectedDate, "yyyy-MM-dd")

        }

        Button {
            text: "📅"
            onClicked: calendarPopup.open()
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                // 不需要 onClicked，Button 本身已经处理
                enabled: false   // 光标显示用，事件传递给 Button
            }
        }
    }

    Popup {
        id: calendarPopup
        x: 0
        y: rootDatePicker.height + 4
        modal: true
        focus: true
        padding: 8
        background: Rectangle {
            color: "#fff"
            border.color: "#aaa"
            radius: 4
        }

        onOpened: calendarView.updateDays()

        ColumnLayout {
            spacing: 6

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter

                spacing: 6

                Button {
                    text: "<<"
                    onClicked: {
                        calendarView.year--
                        calendarView.updateDays()
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        // 不需要 onClicked，Button 本身已经处理
                        enabled: false   // 光标显示用，事件传递给 Button
                    }
                }
                Button {
                    text: "<"
                    onClicked: {
                        calendarView.month = (calendarView.month + 11) % 12
                        if (calendarView.month === 11) calendarView.year--
                        calendarView.updateDays()
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        // 不需要 onClicked，Button 本身已经处理
                        enabled: false   // 光标显示用，事件传递给 Button
                    }

                }
                Label {
                    text: appTranslator.translateText("%1年 %2月").arg(calendarView.year).arg(calendarView.month + 1)
                    Layout.alignment: Qt.AlignHCenter
                }
                Button {
                    text: ">"
                    onClicked: {
                        calendarView.month = (calendarView.month + 1) % 12
                        if (calendarView.month === 0) calendarView.year++
                        calendarView.updateDays()
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        // 不需要 onClicked，Button 本身已经处理
                        enabled: false   // 光标显示用，事件传递给 Button
                    }
                }
                Button {
                    text: ">>"
                    onClicked: {
                        calendarView.year++
                        calendarView.updateDays()
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        // 不需要 onClicked，Button 本身已经处理
                        enabled: false   // 光标显示用，事件传递给 Button
                    }
                }
            }

            GridLayout {
                id: calendarGrid
                columns: 7
                rowSpacing: 4
                columnSpacing: 4

                Repeater {
                    model: [appTranslator.translateText("日"), appTranslator.translateText("一"), appTranslator.translateText("二"), appTranslator.translateText("三"), appTranslator.translateText("四"), appTranslator.translateText("五"), appTranslator.translateText("六")]
                    delegate: Label {
                        text: modelData
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                Repeater {
                    model: calendarView.days
                    delegate: Button {
                        text: modelData.day
                        visible: modelData.visible
                        checkable: true
                        checked: Boolean(modelData.selected)
                        onClicked: {
                            rootDatePicker.selectedDate = modelData.date
                            rootDatePicker.dateSelected(modelData.date) // 发信号
                            calendarPopup.close()
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            // 不需要 onClicked，Button 本身已经处理
                            enabled: false   // 光标显示用，事件传递给 Button
                        }
                    }
                }
            }

        }
    }

    QtObject {
        id: calendarView
        property int year: (new Date()).getFullYear()
        property int month: (new Date()).getMonth()
        property var days: []

        Component.onCompleted: updateDays()

        function updateDays() {
            var first = new Date(year, month, 1)
            var firstDay = first.getDay()
            var last = new Date(year, month + 1, 0)
            var total = last.getDate()
            var list = []

            for (var i = 0; i < firstDay; i++)
                list.push({ visible: false, selected:false })

            for (var d = 1; d <= total; d++) {
                var dateObj = new Date(year, month, d)
                list.push({
                    visible: true,
                    day: d,
                    date: dateObj,
                    selected: Boolean(Qt.formatDate(rootDatePicker.selectedDate, "yyyy-MM-dd") === Qt.formatDate(dateObj, "yyyy-MM-dd"))
                })
            }
            days = list
        }

    }
}
