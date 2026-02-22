import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
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
    property int photoVersion: 0


    Connections {
        target: customerModule
        //function onLog(msg) { console.log("[client]", msg) }
    }

    property var customers: customerModule.customers

    Component.onCompleted: {
      customerModule.search("")

    }

    function getPhotoSource(photo, gender, version) {
        // 1. 优先使用用户上传的照片
        if (photo && photo !== "") {

            return "file:///" + applicationDirPath + "/" + photo+ "?v=" + version;
        }

        // 2. 根据性别返回默认图 (假设 1是男, 2是女)
        switch(gender) {
            case 1:  return "images/male.png";
            case 2:  return "images/female.png";
            default: return "images/user_icon.svg";
        }
    }

    function getGenderString(gender) {
        // 2. 根据性别返回默认图 (假设 1是男, 2是女)
        switch(gender) {
            case 1:  return "男";
            case 2:  return "女";
        }
        return "未知";
    }

    function selectAndNav(targetId) {
        if (!targetId) return;

        // 再次确认清空，防止闪烁
        customerListView.currentIndex = -1;

        let globalIndex = -1;
        for (let i = 0; i < customers.length; i++) {
            if (customers[i].id === targetId) {
                globalIndex = i;
                break;
            }
        }

        if (globalIndex !== -1) {
            let targetPage = Math.floor(globalIndex / pageSize) + 1;

            // 切换页面
            currentPage = targetPage;

            // 计算本地索引
            let localIndex = globalIndex % pageSize;

            // 这一步 callLater 非常关键，它等待分页 slice 刷新完成
            Qt.callLater(() => {
                customerListView.currentIndex = localIndex;
                customerListView.positionViewAtIndex(localIndex, ListView.Contain);
            });
        }
    }

    function getRealIndex() {
        if (customerListView.currentIndex < 0) {
            return -1;
        }

        let realIndex =
                (currentPage - 1) * pageSize
                + customerListView.currentIndex;

        if (realIndex < 0 || realIndex >= customers.length) {
            console.log("index 越界", realIndex)
            return -1;
        }
        return realIndex;
    }


    // function onCustomersChanged() {
    //     console.log("customersChanged received")

    //     // 在这里做你想做的事
    //     // 例如：刷新选中、滚动到顶部、更新统计
    //     customerListView.currentIndex = -1
    // }

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
                         customerModule.search(searchInput.currentText)

                        console.log(`customerListView width is ${customerListView.width} height is ${customerListView.height} pageSize is ${pageSize}`)
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
                id: customerListView
                //Layout.fillWidth: true
                //Layout.fillHeight: true
                anchors.fill: parent
                anchors.topMargin: 5
                anchors.leftMargin: 5
                anchors.rightMargin: 5
                anchors.bottomMargin: 5

                clip: true
                spacing: 2

                // currentIndex: -1
                // Component.onCompleted: {
                //         currentIndex = -1
                //     }
                // onModelChanged: {
                //         if (currentIndex === 0 && count > 0) {
                //             currentIndex = -1
                //         }
                // }

                model: customers.slice((currentPage-1)*pageSize, currentPage*pageSize)


                delegate: Rectangle {
                    width: parent.width
                    height: rowHeight
                    radius: 12

                    readonly property bool isActuallySelected: index === customerListView.currentIndex && customerListView.currentIndex !== -1
                    color: isActuallySelected ? "#FFD6D1" : "#FAEAE8"
                    border.color: isActuallySelected ? "#FF6A00" : "#444"
                    border.width: isActuallySelected ? 2 : 1


                    // ✅ 点击任意位置选中
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            customerListView.currentIndex = index
                            selectionAnimation.restart()
                        }
                    }

                    SequentialAnimation on color {
                        id: selectionAnimation
                        running: false
                        ColorAnimation { from: "#FFD6D1"; to: "#FF6A00"; duration: 200 }
                        ColorAnimation { from: "#FF6A00"; to: "#FFD6D1"; duration: 400 }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 20 // 增加列间距，看起来更美观


                        // 1.客户照片
                        ColumnLayout{
                            Layout.preferredWidth: 80 // 强制固定宽度
                            Layout.alignment:  Qt.AlignLeft | Qt.AlignTop // 顶部对齐
                            Layout.maximumWidth: 80  // 限制最大宽度，防止它向右扩散
                            Layout.fillWidth: false   // 绝对不准填满剩余宽度
                            //spacing: 2
                            Rectangle {
                                width: 80
                                height: 100
                                color: "#fff"
                                radius: 6
                                Image {
                                    id:customerphoto
                                    anchors.fill: parent
                                    anchors.margins: 4
                                    source: getPhotoSource(modelData.photo,modelData.gender,root.photoVersion)
                                    fillMode: Image.PreserveAspectFit
                                }
                            }
                            Item { Layout.fillHeight: true }

                        }


                        // 客户信息
                        ColumnLayout {
                            Layout.preferredWidth: 220 // 给客户信息一个足够且固定的宽度
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: false   // 绝对不准填满剩余宽度
                            spacing: 2
                            Label { text: "客户编号: " + modelData.id; color:"#ffb300" }
                            Label { text: "客户姓名: " + modelData.name; color:"#ffb300" }
                            Label { text: "登记时间: " + modelData.date; color:"#ffb300" }
                            Label { text: "性别: " + getGenderString(modelData.gender) + "   生日: " + modelData.birthday; color:"#ffb300" }
                            Label { text: "Email: " + modelData.email; color:"#ffb300" }
                            Label { text: "电话: " + modelData.phone; color:"#ffb300" }
                            Item { Layout.fillHeight: true }
                        }

                        // 最新报告
                        ColumnLayout {
                            Layout.fillWidth: true // 让最后一列占据剩余的所有空间
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            spacing: 10

                            // 关键：只有当存在 reportDate 时，这块区域才显示并占用空间
                            visible: !!modelData.reportDate && modelData.reportDate !== ""

                            // 如果 visible 为 false，Layout 会自动将其占用的宽高设为 0
                            // 但为了确保不留下“空隙”，建议加上这条
                            Layout.preferredHeight: visible ? -1 : 0
                            Layout.fillHeight: visible
                            Label { text: "报告日期: " + modelData.reportDate; color:"#ffb300" }
                            Label { text: "报告摘要: " + modelData.reportSummary; color:"#ffb300" }
                            Label { text: "肌肤皱纹: " + modelData.wrinkle; color:"#ffb300" }
                            Label { text: "肌肤色斑: " + modelData.spot; color:"#ffb300" }
                            Label { text: "肌肤粉刺: " + modelData.acne; color:"#ffb300" }
                            Label { text: "肌肤血红斑: " + modelData.erythema; color:"#ffb300" }
                            Item { Layout.fillHeight: true }
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                    }
                }
                onHeightChanged:
                {
                    pageSize = Math.floor(customerListView.height / rowHeight)
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
                enabled: currentPage * pageSize < customers.length
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
                    customerDialog.setCustomer({
                        id: "#######",
                        photo: "",
                        name: "",
                        date: Qt.formatDate(new Date(), "yyyy-MM-dd"),
                        gender: 0,
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
                enabled: customerListView.currentIndex !== -1
                onClicked:
                {
                    let realIndex = getRealIndex()
                    if(realIndex<0) return

                    customerDialog.setCustomer({
                        ix: customers[realIndex].ix,
                        id: customers[realIndex].id,
                        photo: customers[realIndex].photo,
                        name: customers[realIndex].name,
                        date: customers[realIndex].date,
                        gender: customers[realIndex].gender,
                        birthday: customers[realIndex].dirthday,
                        email: customers[realIndex].email,
                        phone: customers[realIndex].phone
                    })
                    let customerId = customers[realIndex].id
                    console.log("进入详情 customerId =", customerId)
                    customerDialog.show()

                }
            }
            TextButton {
                Layout.preferredWidth: 200
                Layout.preferredHeight: 50
                text: "删除用户"
                enabled: customerListView.currentIndex !== -1
                onClicked: {
                    // 1. 获取选中行的 ID
                    let realIndex = getRealIndex()
                    if(realIndex<0) return

                    // 2. 将 ID 传给对话框
                    deleteConfirmDialog.targetID = customers[realIndex].id

                    // 3. 弹出对话框
                    deleteConfirmDialog.open()
                }
            }


            TextButton {
                Layout.preferredWidth: 200
                Layout.preferredHeight: 50
                text: "进入分析"
                enabled: customerListView.currentIndex !== -1
                onClicked:
                {
                    let realIndex = getRealIndex()
                    if(realIndex<0) return

                    let customerId = customers[realIndex].id
                    console.log("进入分析 customerId =", customerId)

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

            onAccepted: function(customer) {
                //customerListView[customerListView.currentIndex].customerphoto.source =""
                customerListView.currentIndex = -1;
                // 2. 调用 C++ 的方法将对象传回
                // 注意：C++ 的方法名要对应，假设叫 saveCustomer
                let finalId = customerModule.saveCustomer(customer);
                if (finalId) {
                    // 2. 刷新数据（保持当前的搜索关键词，确保数据是最新的）
                    root.photoVersion++;

                    customerModule.search(searchInput.text);

                    // 3. 关键：等待 model 刷新完成后再执行定位
                    // 使用 Qt.callLater 避开 QML 渲染过程中的中间状态
                    Qt.callLater(() => {
                        selectAndNav(finalId);
                    });
                }
            }
        }

        // --- 删除确认对话框 ---
        Dialog {
            id: deleteConfirmDialog
            title: "确认删除"
            anchors.centerIn: parent // 居中于 customerManager
            width: 300
            property string targetID: ""

            modal: true             // 开启模态
            focus: true
            closePolicy: Popup.NoAutoClose // 强制点击按钮关闭

            // 关键：遮罩层，让背景变暗且不可点击
            Overlay.modal: Rectangle {
                color: "#80000000"
            }

            Label {
                text: "您确定要删除 ID 为 " + deleteConfirmDialog.targetID + " 的用户吗？"
                anchors.centerIn: parent
            }

            footer: DialogButtonBox {
                Button {
                    text: "确认"
                    DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                }
                Button {
                    text: "取消"
                    DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                }
                onAccepted: {
                    // 调用你的 C++ 删除逻辑
                    if(customerModule.deleteCustomer(deleteConfirmDialog.targetID)){
                        customerModule.search(searchInput.currentText)
                    }

                }
            }
        }
    }
}
