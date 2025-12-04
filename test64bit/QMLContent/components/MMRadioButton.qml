import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
// MyRadioButton.qml

RadioButton {
    id: radioOption

    font.pixelSize: 20

    Material.accent: Material.Indigo
    contentItem: Label {
        text: radioOption.text
        // Change the text color
        color: "white"
        font: radioOption.font
        // Align the text within the content area
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        leftPadding: parent.indicator.width + parent.spacing
    }
    MouseArea {
        id: radioMouseArea
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor // Set the cursor shape
        onClicked:
        {
            radioOption.checked=true
        }
    }

}


// RadioButton {
//     id: radioOption

//     font.pixelSize: 20
//     //Material.accent: Material.Indigo
//     indicator: Rectangle {
//         id: ind
//         width: 16
//         height: 16
//         radius: 8
//         border.color: "white"
//         color: "transparent"
//         anchors.verticalCenter: parent.verticalCenter   // ⭐ 关键：垂直居中
//     }

//     // Customize the appearance
//     contentItem: Label {
//         text: radioOption.text
//         // Change the text color
//         color: "white"
//         font: radioOption.font
//         // Align the text within the content area
//         verticalAlignment: Text.AlignVCenter
//         horizontalAlignment: Text.AlignLeft
//         leftPadding: parent.indicator.width + parent.spacing
//     }
//     MouseArea {
//         id: radioMouseArea
//         anchors.fill: parent
//         cursorShape: Qt.PointingHandCursor // Set the cursor shape
//         onClicked:
//         {
//             radioOption.checked=true
//         }
//     }
//     onCheckedChanged:
//     {
//         if (checked)
//             ind.color= "Indigo"
//         else
//             ind.color= "transparent"
//     }
// }

