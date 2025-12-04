import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCharts

Dialog {
    width: 640
    height: 480
    visible: true
    title: "Qt 6.9.3 Chart Example"

    ChartView
    {
    }

    // ChartView {
    //     anchors.fill: parent
    //     antialiasing: true

    //     // 定义 X 轴
    //     CategoryAxis {
    //         id: axisX
    //         // Qt 6.9.3 下使用 append() 添加类别
    //         Component.onCompleted: {
    //             append("4/7")
    //             append("4/15")
    //             append("4/21")
    //             append("4/29")
    //             append("5/6")
    //         }
    //     }

    //     // 定义 Y 轴
    //     ValueAxis {
    //         id: axisY
    //         min: 0
    //         max: 100
    //     }

    //     // 柱状序列
    //     BarSeries {
    //         id: barSeries
    //         axisX: axisX
    //         axisY: axisY

    //         BarSet {
    //             label: "毛孔"
    //             values: [57, 66, 54, 53, 51]
    //         }
    //         BarSet {
    //             label: "纹理"
    //             values: [42, 50, 61, 48, 55]
    //         }
    //     }

    //     legend.visible: true
    //     legend.alignment: Qt.AlignBottom
    // }
}
