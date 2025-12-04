import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts
//import QtGraphs
//import "components"

Item {
    id: customerReport
    //width: 1920
    //height: 1080

    signal loadPage(string page, var params)
    property int customerID: -1
    property int curIndex: 0   // ⭐ 当前展开的缩略图编号


    property var reportLabels:[
                           "毛 孔", "粉 刺", "深层色斑", "浅层色斑",
                           "皱 纹", "敏感度", "褐色斑", "混合彩斑", "综合报告"
                         ]
    property var resDate: [ "2025/4/1",
                            "2025/5/4",
                            "2025/6/7",
                            "2025/7/12",
                            "2025/8/31"
                          ]

    property var resDatas: [{name: "毛 孔",res:[10, 25, 12, 28, 14]},
                            {name: "粉 刺",res:[73, 62, 71, 58, 74]},
                            {name: "深层色斑",res:[30,52, 18, 24,46]},
                            {name: "浅层色斑",res:[43, 66, 31, 42, 71]},
                            {name: "皱 纹",res:[90, 72, 22, 45, 21]},
                            {name: "敏感度",res:[63, 19, 46, 37, 83]},
                            {name: "褐色斑",res:[12, 23, 45, 67, 89]},
                            {name: "混合彩斑",res:[74, 61, 51, 53, 64]},
                            {name: "综合报告",res:[30,55, 42, 88, 74]}
                          ]
    property var mainphotoes: [
        {photoL:"customers/0000001/0000001_01_01_L.jpg", photoR:"customers/0000001/0000001_01_01_R.jpg"},
    ]

    //ListModel {
    //    id: mainphotoes
    //    ListElement { photoL: "customers/0000001/0000001_01_01_L.jp"; photoR: "customers/0000001/0000001_01_01_R.jpg" }
    //}

    property var subphotoes: [
        {photoL:"customers/0000001/0000001_01_01_L.jpg", photoR:"customers/0000001/0000001_01_01_R.jpg"},
        {photoL:"customers/0000001/0000001_01_02_L.jpg", photoR:"customers/0000001/0000001_01_02_R.jpg"},
        {photoL:"customers/0000001/0000001_01_03_L.jpg", photoR:"customers/0000001/0000001_01_03_R.jpg"},
        {photoL:"customers/0000001/0000001_01_04_L.jpg", photoR:"customers/0000001/0000001_01_04_R.jpg"},
        {photoL:"customers/0000001/0000001_01_05_L.jpg", photoR:"customers/0000001/0000001_01_05_R.jpg"},
        {photoL:"customers/0000001/0000001_01_06_L.jpg", photoR:"customers/0000001/0000001_01_06_R.jpg"},
        {photoL:"customers/0000001/0000001_01_07_L.jpg", photoR:"customers/0000001/0000001_01_07_R.jpg"},
        {photoL:"customers/0000001/0000001_01_08_L.jpg", photoR:"customers/0000001/0000001_01_08_R.jpg"},
    ]


    // 单个图例项
    Rectangle {
        anchors.fill: parent
        color: "#0e1f30"     // 深蓝背景

        ColumnLayout {
            anchors.fill: parent
            spacing: 10
            // ================================
            // 1. Header：头像 + 名字 + 标题 + 退出按钮
            // ================================
            RowLayout {
                id: header
                Layout.fillWidth: true
                spacing: 10
                //height: 150

                Rectangle {
                    width: 150; height: 150
                    radius: 10
                    border.color: "#6aaaff"
                    color: "transparent"

                    Image {
                        anchors.fill: parent
                        source: ""       // 用户头像
                        fillMode: Image.PreserveAspectFit
                    }
                }

                Text {
                    id: userName
                    text: "aaa"
                    color: "white"
                    font.pixelSize: 42
                    verticalAlignment: Text.AlignVCenter
                }

                //Rectangle { Layout.fillWidth: true; color: "transparent" } // 空白拉伸

                Text {
                    text: "客户报告"
                    color: "#7dc9ff"
                    font.pixelSize: 55
                }


            }


            // ================================
            // 2. 上方报告选项按钮区（8个）
            // ================================
            RowLayout {
                id: tabButtons
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                Layout.topMargin: 20
                spacing: 10

                property int selectedIndex: 8   // 默认选中 “综合报告”

                Repeater {
                    model:reportLabels

                    delegate: Rectangle {
                        width: 180; height: 70
                        radius: 6
                        border.color: index === tabButtons.selectedIndex ? "#90c8ff" : "#466080"
                        border.width: 2
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: index === tabButtons.selectedIndex ? "#668bb8" : "#345" }
                            GradientStop { position: 1.0; color: index === tabButtons.selectedIndex ? "#446a9c" : "#233" }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: modelData
                            color: "white"
                            font.pixelSize: 30
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                console.log(`viewStack top is ${viewStack.top} width is ${viewStack.width} : height is ${viewStack.height}`);
                                console.log(`totalPage width is ${totalPage.width} : height is ${totalPage.height}`);
                                console.log(`photoArea width is ${photoArea.width} : height is ${photoArea.height}`);
                                console.log(`chart width is ${chart.width} : height is ${chart.height}`);
                                tabButtons.selectedIndex = index
                                if(index===8)
                                {
                                    viewStack.currentIndex = 0
                                }
                                else
                                {

                                    viewStack.currentIndex = 1
                                    chartBar.updatebar()
                                }
                            }
                        }
                    }
                }
            }



            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.topMargin: 20
                Layout.leftMargin: 150
                Layout.rightMargin:150
                spacing: 10

                StackLayout {
                    id: viewStack
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignHCenter
                    anchors.margins: 20
                    currentIndex: 0
                    clip: true

                    // 综合报告
                    ColumnLayout  {
                        id: totalPage
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 4

                        // ================================
                        // 3. 两行图片展示区（8张）
                        // ================================
                        RowLayout {
                            id: photoArea
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignHCenter
                            spacing: 20
                            Repeater {
                                model: subphotoes
                                delegate: Column {
                                    spacing: 10
                                    Rectangle {
                                        width: 184
                                        height: 124
                                        radius: 8
                                        border.color: "#7ec0ff"
                                        color: "transparent"

                                        Image
                                        {
                                            id:subThumbImgL
                                            x:2
                                            y:2
                                            width:90
                                            height:120
                                            source: modelData.photoL
                                            fillMode: Image.PreserveAspectFit
                                            Layout.alignment: Qt.AlignVCenter
                                        }
                                        Image
                                        {
                                            id:subThumbImgR
                                            x:92
                                            y:2
                                            width:90
                                            height:120
                                            source: modelData.photoR
                                            fillMode: Image.PreserveAspectFit
                                            Layout.alignment: Qt.AlignVCenter
                                            anchors.leftMargin: 8
                                        }
                                    }
                                    Text {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        text: ["毛孔","粉刺","深层色斑","浅层色斑","皱纹","敏感","褐色斑","混合彩斑"][index]
                                        color: "white"
                                        font.pixelSize: 28
                                    }
                                }
                            }
                        }

                        // ================================
                        // 4. 折线图区域（用 Image 或 ChartView 都可以）
                        // ================================

                        ChartView {
                            id: chart
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            margins.left: 10
                            margins.right:10
                            margins.top: 10
                            margins.bottom:  10
                            antialiasing: true

                            function updateChart()
                            {
                                s1.clear()
                                s2.clear()
                                s3.clear()
                                s4.clear()
                                s5.clear()
                                s6.clear()
                                s7.clear()
                                s8.clear()

                                // for(var  i=0;i<resDate.length;i++)
                                // {
                                //     axisX.append(resDate[i], i)
                                //     console.log("updateChart resDate[i] is ",resDate[i])
                                // }

                                for(var  i=0;i<resDate.length;i++)
                                {
                                    axisX.append(resDate[i], i)
                                    s1.append(i, resDatas[0].res[i]);
                                    s2.append(i, resDatas[1].res[i]);
                                    s3.append(i, resDatas[2].res[i]);
                                    s4.append(i, resDatas[3].res[i]);
                                    s5.append(i, resDatas[4].res[i]);
                                    s6.append(i, resDatas[5].res[i]);
                                    s7.append(i, resDatas[6].res[i]);
                                    s8.append(i, resDatas[7].res[i]);
                                }
                                axisX.max = resDate.length;
                                console.log("updateChart end count is ",resDate.length)
                            }
                              // X 轴：分类轴
                            CategoryAxis
                            {
                                id: axisX
                                labelsPosition: CategoryAxis.AxisLabelsPositionOnValue
                            }

                              // Y 轴：值轴
                            ValueAxis {
                                  id: axisY
                                  min: 0
                                  max: 100
                            }

                              // ["毛孔","粉刺","深层色斑","浅层色斑","皱纹","敏感","褐色斑","褐色斑"]
                            LineSeries {id: s1;axisX: axisX;axisY: axisY;name: "毛孔"}
                            LineSeries {id: s2;axisX: axisX;axisY: axisY;name: "粉刺"}
                            LineSeries {id: s3;axisX: axisX;axisY: axisY;name: "深层色斑"}
                            LineSeries {id: s4;axisX: axisX;axisY: axisY;name: "浅层色斑"}
                            LineSeries {id: s5;axisX: axisX;axisY: axisY;name: "皱纹"}
                            LineSeries {id: s6;axisX: axisX;axisY: axisY;name: "敏感"}
                            LineSeries {id: s7;axisX: axisX;axisY: axisY;name: "褐色斑"}
                            LineSeries {id: s8;axisX: axisX;axisY: axisY;name: "褐色斑"}
                            legend.visible: true
                            legend.alignment: Qt.AlignBottom

                            Component.onCompleted: updateChart()


                        }

                    }

                    //单项报告
                    Row {
                        spacing: 20
                        //anchors.centerIn: parent
                        Rectangle {
                            id:mainImgL
                            width: (viewStack.height -10)/4*3; height: viewStack.height -10
                            radius: 8; color: "#222"; border.color: "#ffb300"
                            Image { anchors.fill: parent; fillMode: Image.PreserveAspectFit; source: subphotoes[tabButtons.selectedIndex].photoL}
                        }
                        Rectangle {
                            id:mainImgR
                            width: (viewStack.height -10)/4*3; height: viewStack.height -10
                            radius: 8; color: "#222"; border.color: "#ffb300"
                            Image { anchors.fill: parent; fillMode: Image.PreserveAspectFit; source: subphotoes[tabButtons.selectedIndex].photoR }
                        }
                        ChartView {
                            id: chartBar
                            width: parent.width - mainImgL.width *2 - 40
                            height: viewStack.height -10
                            antialiasing: true

                            function updatebar()
                            {
                                barSeries.clear()

                                for(var  i=0;i<resDate.length;i++)
                                {
                                    axisBarX.append(resDate[i],i)
                                    console.log("updatebar resDate[i] is ",resDate[i])
                                }

                                barSeries.append(resDatas[tabButtons.selectedIndex].name,resDatas[tabButtons.selectedIndex].res)
                                axisBarX.max = resDate.length
                                console.log("updatebar name is ",resDatas[tabButtons.selectedIndex].name)
                            }

                           // X 轴：分类轴
                            CategoryAxis
                            {
                                id: axisBarX
                                labelsPosition: CategoryAxis.AxisLabelsPositionOnValue
                            }

                              // Y 轴：值轴
                            ValueAxis {
                                  id: axisBarY
                                  min: 0
                                  max: 100
                            }
                            BarSeries {
                                id: barSeries
                                axisX: axisBarX
                                axisY: axisBarY
                            }


                            legend.visible: true
                            legend.alignment: Qt.AlignBottom

                            Component.onCompleted: updatebar()


                        }
                    }
                }
            }
            // ================================
            // 5. 底部操作按钮区
            // ================================
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 20
                Layout.bottomMargin: 20
                height: 100
                spacing: 50

                Repeater {
                    model: [
                        {text: "二维码", icon: "qr.png" },
                        {text: "邮件", icon: "mail.png" },
                        {text: "彩点显示", icon: "spot.png" },
                        {text: "A4打印", icon: "print.png" },
                        {text: "HOME", icon: "print.png" }
                    ]

                    delegate: Column {
                        spacing: 10
                        Rectangle {
                            width: 85
                            height: 85
                            radius: 10
                            color: "#204060"
                            border.color: "#7cc0ff"

                            Image {
                                anchors.centerIn: parent
                                width: 50
                                height: 50
                                source: modelData.icon
                                fillMode: Image.PreserveAspectFit
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked:
                                {
                                    console.log(`photoArea x:${photoArea.x},y:${photoArea.y},photoArea.width:${photoArea.width},photoArea.height:${photoArea.width}`)
                                    //console.log(`chartRow x:${chartRow.x},y:${chartRow.y},chartRow.width:${chartRow.width},chartRow.height:${chartRow.width}`)
                                    console.log(modelData.text,modelData.index)

                                    if(index===4)
                                    {
                                        loadPage("logo.qml",{})
                                    }
                                }
                            }
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: modelData.text
                            color: "white"
                            font.pixelSize: 24
                        }
                    }
                }
            }



        }
    }

}
