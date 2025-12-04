// LegendItem.qml
import QtQuick

Rectangle {
    id: box
    width: 140
    height: 22
    radius: 4
    color: "#303030"
    border.color: "#999"
    border.width: 1

    property string seriesName: ""
    property color seriesColor: "white"

    Row {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 6

        Rectangle { width: 14; height: 14; radius: 2; color: box.seriesColor }
        Text { text: box.seriesName; color: "white"; font.pixelSize: 14 }
    }
}
