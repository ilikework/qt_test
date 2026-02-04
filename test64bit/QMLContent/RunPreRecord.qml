// Standalone launcher: run with "qml RunPreRecord.qml" from QMLContent folder to preview PreRecordDialog only
import QtQuick
import QtQuick.Controls

ApplicationWindow {
    visible: true
    width: 1
    height: 1
    title: "PreRecord Preview"
    Component.onCompleted: {
        var c = Qt.createComponent("PreRecordDialog.qml")
        if (c.status === Component.Loading) {
            c.statusChanged.connect(function() {
        if (c.status === Component.Ready) {
            var dlg = c.createObject(null)
            if (dlg) {
                dlg.closing.connect(Qt.quit)
                dlg.show()
            }
        } else if (c.status === Component.Error) {
                    console.error("Error loading PreRecordDialog:", c.errorString())
                }
            })
        } else if (c.status === Component.Ready) {
            var dlg = c.createObject(null)
            if (dlg) {
                dlg.closing.connect(Qt.quit)
                dlg.show()
            }
        } else {
            console.error("Error loading PreRecordDialog:", c.errorString())
        }
    }
}
