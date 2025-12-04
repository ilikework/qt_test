import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

CheckBox {
    id: control
    font.pixelSize: 20
    bottomPadding: 0
    //color:"green"
    Material.accent: Material.Indigo
   //  indicator: Rectangle
   //  {
   //        implicitWidth: 16
   //        implicitHeight: 16
   //        radius: 3
   //        border.color: control.checked ? "blue" : "gray"
   //        border.width: 1
   //        anchors.verticalCenter: parent.verticalCenter
   //  }
   //  // The actual check mark/fill
   // Rectangle {
   //     width: 12
   //     height: 12
   //     radius: 2
   //     color: "blue" // The color when checked
   //     anchors.centerIn: parent

   //     // Use opacity and a Behavior for a smooth visual transition
   //     opacity: control.checked ? 1.0 : 0.0
   //     Behavior on opacity {
   //         // Optional: Add a smooth animation when checking/unchecking
   //         NumberAnimation { duration: 100 }
   //     }
   // }
    contentItem: Text {
        text: control.text
        color: "white"
        font: control.font
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + control.spacing
    }
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked:
        {
            control.checked= !control.checked
        }
    }


}
