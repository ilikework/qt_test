import QtQuick
//import testQML

Window {
    id: root
    visible: true
    //width: 800
    //height: 600
    visibility: "FullScreen"
    color: "#18181b"

    // 临时保存参数
    property var loadParams: null

    Loader {
        id: pageLoader
        anchors.fill: parent
        source: "logo.qml"

        onLoaded: {
            console.log("onLoaded:", item)

            // if (loadParams && item) {
            //     for (var key in loadParams) {
            //         console.log("assign:", key, "=", loadParams[key])
            //         item[key] = loadParams[key]
            //     }
            // }

            // loadParams = null
        }
    }

    // 当前子页面调用 loadPage()
    Connections {
        target: pageLoader.item

        function onLoadPage(page, params) {
            console.log("onLoadPage", page, params)
            root.loadParams = params || {}   // 保存参数
            pageLoader.setSource(page, params)
            //pageLoader.source = page         // 加载新页面
        }
    }
}

// Window {
//     id: root
//     visible: true
//     visibility: "FullScreen"
//     color: "#18181b"
//     //Qt.application.style: "Material"


//     Loader {
//         id: pageLoader
//         anchors.fill: parent
//         source: "logo.qml"    // 初始显示 a1
//         //source: "testWindow2.qml"    // 初始显示 a1
//     }

//     Connections {
//         target: pageLoader.item  // item 必须存在，并且有 loadPage signal
//         function onLoadPage(page, params) {
//             console.log(`onLoadPage ${page} ${params}` )

//             pageLoader.source = page
//             // 参数传递可以在 onLoaded 里做
//             Connections: {
//                 //target: pageLoader
//                 function onLoaded() {
//                     console.log(` ${pageLoader.item} ` )
//                     if (params && pageLoader.item) {
//                         for (var key in params) {
//                             pageLoader.item[key] = params[key]
//                             console.log(` pageLoader.item[key]=${pageLoader.item[key]} params[key]` )
//                         }
//                     }
//                 }
//             }
//         }
//     }
// }
