import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Basic
import "components"
import "."

Item {
    id: root
    visible: true
    signal loadPage(string page, var params)

    property int pageSize: 0
    property int rowHeight: 120
    property int currentPage: 1

    // 模拟数据
    property var users: [
        {id:"0000001", photo:"images/male.png", name:"Wang", date:"2025-11-02", gender:"男", birthday:"1995-08-15", email:"wang@example.com", phone:"13800138000",
         reportDate:"2025-11-12", reportSummary:"肌肤年龄，28岁", wrinkle:"12%", spot:"8%", acne:"5%", erythema:"3%"},
        {id:"0000002", photo:"images/female.png", name:"Li", date:"2025-10-15", gender:"女", birthday:"2000-03-22", email:"li@example.com", phone:"13900139000",
         reportDate:"2025-10-20", reportSummary:"肌肤年龄，24岁", wrinkle:"10%", spot:"6%", acne:"2%", erythema:"1%"},
        {id:"0000003", photo:"images/male.png", name:"Zhang", date:"2025-09-21", gender:"男", birthday:"1985-12-01", email:"zhang@example.com", phone:"13700137000",
         reportDate:"2025-09-25", reportSummary:"肌肤年龄，41岁", wrinkle:"18%", spot:"12%", acne:"7%", erythema:"4%"},
        {id:"0000004", photo:"images/female.png", name:"Sun", date:"2025-08-30", gender:"女", birthday:"1990-06-10", email:"sun@example.com", phone:"13600136000",
         reportDate:"2025-09-02", reportSummary:"肌肤年龄，36岁", wrinkle:"14%", spot:"9%", acne:"4%", erythema:"2%"},
        {id:"0000005", photo:"images/male.png", name:"Zhao", date:"2025-07-18", gender:"男", birthday:"1996-11-20", email:"zhao@example.com", phone:"13500135000",
         reportDate:"2025-07-20", reportSummary:"肌肤年龄，29岁", wrinkle:"9%", spot:"5%", acne:"1%", erythema:"0%"},
        {id:"0000006", photo:"images/male.png", name:"Wang", date:"2025-11-02", gender:"男", birthday:"1995-08-15", email:"wang@example.com", phone:"13800138000",
         reportDate:"2025-11-12", reportSummary:"肌肤年龄，28岁", wrinkle:"12%", spot:"8%", acne:"5%", erythema:"3%"},
        {id:"0000007", photo:"images/female.png", name:"Li", date:"2025-10-15", gender:"女", birthday:"2000-03-22", email:"li@example.com", phone:"13900139000",
         reportDate:"2025-10-20", reportSummary:"肌肤年龄，24岁", wrinkle:"10%", spot:"6%", acne:"2%", erythema:"1%"},
        {id:"0000008", photo:"images/male.png", name:"Zhang", date:"2025-09-21", gender:"男", birthday:"1985-12-01", email:"zhang@example.com", phone:"13700137000",
         reportDate:"2025-09-25", reportSummary:"肌肤年龄，41岁", wrinkle:"18%", spot:"12%", acne:"7%", erythema:"4%"},
        {id:"0000009", photo:"images/female.png", name:"Sun", date:"2025-08-30", gender:"女", birthday:"1990-06-10", email:"sun@example.com", phone:"13600136000",
         reportDate:"2025-09-02", reportSummary:"肌肤年龄，36岁", wrinkle:"14%", spot:"9%", acne:"4%", erythema:"2%"},
        {id:"0000010", photo:"images/male.png", name:"Zhao", date:"2025-07-18", gender:"男", birthday:"1996-11-20", email:"zhao@example.com", phone:"13500135000",
         reportDate:"2025-07-20", reportSummary:"肌肤年龄，29岁", wrinkle:"9%", spot:"5%", acne:"1%", erythema:"0%"},
        {id:"0000011", photo:"images/male.png", name:"Wang", date:"2025-11-02", gender:"男", birthday:"1995-08-15", email:"wang@example.com", phone:"13800138000",
         reportDate:"2025-11-12", reportSummary:"肌肤年龄，28岁", wrinkle:"12%", spot:"8%", acne:"5%", erythema:"3%"},
        {id:"0000012", photo:"images/female.png", name:"Li", date:"2025-10-15", gender:"女", birthday:"2000-03-22", email:"li@example.com", phone:"13900139000",
         reportDate:"2025-10-20", reportSummary:"肌肤年龄，24岁", wrinkle:"10%", spot:"6%", acne:"2%", erythema:"1%"},
        {id:"0000013", photo:"images/male.png", name:"Zhang", date:"2025-09-21", gender:"男", birthday:"1985-12-01", email:"zhang@example.com", phone:"13700137000",
         reportDate:"2025-09-25", reportSummary:"肌肤年龄，41岁", wrinkle:"18%", spot:"12%", acne:"7%", erythema:"4%"},
        {id:"0000014", photo:"images/female.png", name:"Sun", date:"2025-08-30", gender:"女", birthday:"1990-06-10", email:"sun@example.com", phone:"13600136000",
         reportDate:"2025-09-02", reportSummary:"肌肤年龄，36岁", wrinkle:"14%", spot:"9%", acne:"4%", erythema:"2%"},
        {id:"0000015", photo:"images/male.png", name:"Zhao", date:"2025-07-18", gender:"男", birthday:"1996-11-20", email:"zhao@example.com", phone:"13500135000",
         reportDate:"2025-07-20", reportSummary:"肌肤年龄，29岁", wrinkle:"9%", spot:"5%", acne:"1%", erythema:"0%"}
    ]
    // property var users: []

    ColumnLayout  {
        anchors.fill: parent
        //anchors.top: parent.top
        anchors.topMargin: 20
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.bottomMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 12
        //width: parent.width * 0.9
        //height:parent.height
        // 标题
        Label {
            text: "客户管理"
            height: 30
            font.pixelSize: 36
            font.bold: true
            color: "#fff"
        }

        // // 搜索栏
        RowLayout {
            id:searchRow
            height: 24
            spacing: 12
            Label {
                text: "搜索方式:"; color:"#ffd"
                font.pixelSize: 24
                font.bold: true
            }
            ComboBox {
                id: searchType
                width: 580
                font.pixelSize: 24
                font.bold: true
                model: ["客户编号","客户姓名","客户电话"]

                // 重写显示当前选中项的内容
                contentItem: Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.right: parent.right
                    leftPadding: 10
                    rightPadding: 10
                    text: searchType.displayText
                    font.pixelSize: 24
                    font.bold: true
                    elide: Text.ElideNone   // 不省略
                    horizontalAlignment: Text.AlignLeft
                }

                // 下拉列表
                delegate: ItemDelegate {
                    width: searchType.width
                    text: modelData
                    font.pixelSize: 24
                }
            }

            TextField {
                id: searchInput
                font.pixelSize: 24
                font.bold: true
                placeholderText: "请输入搜索内容"
                width: 580
                Layout.preferredWidth: 580    // 告诉布局使用此宽度
                Layout.maximumWidth: 580
                Layout.minimumWidth: 580
            }

            Button {
                text: "🔍"
                font.pixelSize: 24
                background: Rectangle { color:"#444"; radius:6 }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        console.log("搜索:", searchType.currentText, searchInput.text)
                        console.log(`userListView width is ${userListView.width} height is ${userListView.height} pageSize is ${pageSize}`)
                    }
                }
            }
        }

        // 客户表格
        Rectangle {
            id: listContainer
            width: parent.width
            //anchors.horizontalCenter: parent.horizontalCenter
            height: parent.height - 30 - 24 - 40 -100 -40   // 顶部 bar 50 + 底部按钮 60 + margin 3

            color: "transparent"      // 背景透明
            border.color: "#444"      // 边框颜色
            border.width: 2           // 边框宽度
            radius: 6                 // 可选，圆角

            ListView {
                id: userListView
                //Layout.fillWidth: true
                //Layout.fillHeight: true
                anchors.fill: parent
                anchors.topMargin: 5
                anchors.leftMargin: 5
                anchors.rightMargin: 5
                anchors.bottomMargin: 5

                clip: true
                spacing: 2

                model: users.slice((currentPage-1)*pageSize, currentPage*pageSize)


                delegate: Rectangle {
                    width: parent.width
                    height: rowHeight
                    radius: 12

                    // ✅ 是否选中当前项
                    property bool selected: ListView.isCurrentItem

                    // ✅ 选中样式（颜色/边框/阴影你随便调）
                    color: selected ? "#FFD6D1" : "#FAEAE8"
                    border.color: selected ? "#FF6A00" : "#444"
                    border.width: selected ? 2 : 1

                    // ✅ 点击任意位置选中
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: userListView.currentIndex = index
                    }


                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 4

                        // 客户照片
                        Rectangle {
                            width: 80
                            height: 100
                            color: "#fff"
                            radius: 6
                            Image {
                                anchors.fill: parent
                                anchors.margins: 4
                                source: modelData.photo
                                fillMode: Image.PreserveAspectFit
                            }
                        }

                        // 客户信息
                        ColumnLayout {
                            spacing: 2
                            Label { text: "客户编号: " + modelData.id; color:"#ffb300" }
                            Label { text: "客户姓名: " + modelData.name; color:"#ffb300" }
                            Label { text: "登记时间: " + modelData.date; color:"#ffb300" }
                            Label { text: "性别: " + modelData.gender + "   生日: " + modelData.birthday; color:"#ffb300" }
                            Label { text: "Email: " + modelData.email; color:"#ffb300" }
                            Label { text: "电话: " + modelData.phone; color:"#ffb300" }
                        }

                        // 最新报告
                        ColumnLayout {
                            spacing: 2
                            Label { text: "报告日期: " + modelData.reportDate; color:"#ffb300" }
                            Label { text: "报告摘要: " + modelData.reportSummary; color:"#ffb300" }
                            Label { text: "肌肤皱纹: " + modelData.wrinkle; color:"#ffb300" }
                            Label { text: "肌肤色斑: " + modelData.spot; color:"#ffb300" }
                            Label { text: "肌肤粉刺: " + modelData.acne; color:"#ffb300" }
                            Label { text: "肌肤血红斑: " + modelData.erythema; color:"#ffb300" }
                        }

                        // // 功能按钮
                        // ColumnLayout {
                        //     spacing: 4
                        //     TextButton
                        //     {
                        //         width: 100
                        //         height: 25
                        //         font.bold: false          // 可选，加粗
                        //         text: "进入详情"
                        //         onClicked:
                        //         {
                        //             console.log("进入详情", modelData.id)
                        //             //customerAnalyse2.showFullScreen()
                        //             loadPage("customerAnalyse.qml", { customerID: modelData.id })
                        //             //loadPage("customerAnalyse.qml", { customerID: "0000001" })
                        //         }
                        //     }
                        //     TextButton
                        //     {
                        //         width: 100
                        //         height: 25
                        //         font.bold: false          // 可选，加粗
                        //         text: "编辑信息"
                        //         onClicked: console.log("编辑信息", modelData.id)
                        //     }
                        // }
                    }
                }
                onHeightChanged:
                {
                    console.log("onHeightChanged in")
                    pageSize = Math.floor(userListView.height / rowHeight)
                }
            }
        }
        // 分页按钮
        RowLayout {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter   // 代替 anchors.horizontalCenter
            Layout.fillWidth: true                                // 如果想横向撑满
            height: 40
            spacing: 10
            TextButton {
                text: "< 上一页"
                enabled: currentPage > 1
                onClicked: currentPage--
            }
            TextButton {
                text: "下一页 >"
                enabled: currentPage * pageSize < users.length
                onClicked: currentPage++
            }
        }

        RowLayout {
            id:funButtons
            height: 100
            spacing: 6
            // 新增按钮
            TextButton {
                Layout.preferredWidth: 200
                Layout.preferredHeight: 50
                text: "新增用户"

                onClicked: {
                    customerDialog.setUser({
                        id: "自动生成",
                        photo: "images/user_icon.svg",
                        name: "",
                        date: Qt.formatDate(new Date(), "yyyy-MM-dd"),
                        gender: "",
                        birthday: "",
                        email: "",
                        phone: ""
                    })
                    customerDialog.show()
                }
            }

            TextButton {
                Layout.preferredWidth: 200
                Layout.preferredHeight: 50
                text: "编辑信息"
                onClicked:
                {
                    if (userListView.currentIndex < 0) {
                        console.log("未选择任何客户")
                        return
                    }

                    let realIndex =
                            (currentPage - 1) * pageSize
                            + userListView.currentIndex

                    if (realIndex < 0 || realIndex >= users.length) {
                        console.log("index 越界", realIndex)
                        return
                    }

                    customerDialog.setUser({
                        id: users[realIndex].id,
                        photo: users[realIndex].photo,
                        name: users[realIndex].name,
                        date: users[realIndex].date,
                        gender: users[realIndex].gender,
                        birthday: users[realIndex].dirthday,
                        email: users[realIndex].email,
                        phone: users[realIndex].phone
                    })
                    let customerId = users[realIndex].id
                    console.log("进入详情 customerId =", customerId)
                    customerDialog.open()

                }
            }
            TextButton {
                Layout.preferredWidth: 200
                Layout.preferredHeight: 50
                text: "删除用户"

                onClicked: {

                }
            }


            TextButton {
                Layout.preferredWidth: 200
                Layout.preferredHeight: 50
                text: "进入详情"
                onClicked:
                {
                    if (userListView.currentIndex < 0) {
                        console.log("未选择任何客户")
                        return
                    }

                    let realIndex =
                            (currentPage - 1) * pageSize
                            + userListView.currentIndex

                    if (realIndex < 0 || realIndex >= users.length) {
                        console.log("index 越界", realIndex)
                        return
                    }

                    let customerId = users[realIndex].id
                    console.log("进入详情 customerId =", customerId)

                    loadPage("customerAnalyse.qml", {
                        customerID: customerId
                    })
                }
            }

            TextButton {
                text: "回到Home"
                Layout.preferredWidth: 200
                Layout.preferredHeight: 50

                onClicked:
                {
                    console.log("回到Home")
                    loadPage("logo.qml",{})
                }
            }
        }

        // CustomerAnalyse2
        // {
        //     id:customerAnalyse2
        // }

        CustomerEditDialog {
            id: customerDialog

            onAccepted: {
                console.log("保存:", userName)
            }
        }
    }
}
