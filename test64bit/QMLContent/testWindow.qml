import QtQuick
import QtQuick.Window
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.AssetUtils

Window {
    width: 800
    height: 600
    visible: true
    title: "Toyplane Viewer"



    View3D {
        id:view3d
        anchors.fill: parent

        PerspectiveCamera { id: camera; position: Qt.vector3d(0,0,30) }
        //DirectionalLight { eulerRotation: Qt.vector3d(-45,45,0) }
        DirectionalLight {
            id: sunLight
            // Set rotation using Euler angles (degrees)
            eulerRotation: Qt.vector3d(-45, 60, 0)
            color: "white"
            //intensity: 1.0
        }
        Node { id: originNode; position: Qt.vector3d(0,0,0) }


        Node { id: modelContainer; parent: originNode; scale: Qt.vector3d(10,10,10)
            RuntimeLoader { source: "./customers/0000001/0000001_01.obj" }
        }

        // Orbit controller
        OrbitCameraController {
            camera: camera
            origin: originNode
        }

        // Light pivot: rotates around model
        Node {
            id: lightPivot
            position: Qt.vector3d(0,0,0)

            DirectionalLight {
                id: rotatingLight
                eulerRotation: Qt.vector3d(-45,0,0)
                color: "white"
                //intensity: 1.0
            }

            // Animate the pivot's rotation
            NumberAnimation on eulerRotation.y {
                from: 0
                to: 360
                duration: 10000
                loops: Animation.Infinite
                running: true
            }
        }
    }


}
