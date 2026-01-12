import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls.Material
import "components"

Window {
    //visible: true
    id:dlg
    width: 800
    height: 550
    modality: Qt.ApplicationModal
    flags: Qt.FramelessWindowHint
    property int baseFontSize: 20

    signal accepted(var customer)
    signal canceled()

    QtObject {
        id: customer
        property int ix: -1
        property string id: ""
        property string photo: ""
        property bool photo_update: false
        property string name: ""
        property string date: ""
        property int gender: 0
        property string birthday: ""
        property string email: ""
        property string phone: ""
    }

    function getPhotoSource(photo, gender) {
        // 1. 优先使用用户上传的照片
        if (photo && photo !== "") {
            if (photo.startsWith("file:///") || photo.startsWith("qrc:/") || photo.startsWith("http")) {
                    return photo
            }
            // 假设 path 是相对路径 "customer/ID/photo.jpg"
            // Qt.resolvedUrl(".") 会定位到当前 QML 所在的目录
            return "file:///" + applicationDirPath + "/" + photo;
        }

        // 2. 根据性别返回默认图 (假设 1是男, 2是女)
        switch(gender) {
            case 1:  return "images/male.png";
            case 2:  return "images/female.png";
            default: return "images/user_icon.svg";
        }
    }
    function setCustomer(data) {
        if (!data) return
        customer.ix       = data.ix       || -1
        customer.id       = data.id       || ""
        customer.photo    = data.photo    || ""
        customer.name     = data.name     || ""
        customer.date     = data.date     || ""
        customer.gender   = data.gender   || 0
        customer.birthday = data.birthday || ""
        customer.email    = data.email    || ""
        customer.phone    = data.phone    || ""
        customer.photo_update = false

        // 手动更新组件状态，而不是靠自动绑定
        if (customer.date.length > 0) {
            createDate.selectedDate = Date.fromLocaleString(Qt.locale(), customer.date, "yyyy-MM-dd")
        } else {
            createDate.selectedDate = new Date()
        }
        if (customer.birthday.length > 0) {
            birthday.selectedDate = Date.fromLocaleString(Qt.locale(), customer.birthday, "yyyy-MM-dd")
        } else {
            birthday.selectedDate = new Date(2000, 0, 1)
        }

        customerPhoto.source = getPhotoSource(customer.photo,customer.gender)
    }



    function validate() {
            // 1. 检查必填项（非空）
            if (nameInput.text.trim() === "") {
                errorLabel.text = "⚠️ 姓名不能为空"
                nameInput.forceActiveFocus() // 让错误的输入框获得焦点
                return false
            }
            if(genderBox.currentIndex!==1 && genderBox.currentIndex!==2 ) {
                errorLabel.text = "⚠️ 请选择性别"
                genderBox.forceActiveFocus()
                return false
            }

            // 2. 检查手机号格式（简单正则：1开头且为11位数字）
            let phoneRegex = /^1[3-9]\d{9}$/
            if (!phoneRegex.test(phoneInput.text)) {
                errorLabel.text = "⚠️ 手机号格式错误"
                phoneInput.forceActiveFocus()
                return false
            }

            // 3. 检查邮箱格式（如果有输入的话）
            if (emailInput.text.trim() !== "") {
                let emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/
                if (!emailRegex.test(emailInput.text)) {
                    errorLabel.text = "⚠️ 邮箱格式不正确！"
                    emailInput.forceActiveFocus()
                    return false
                }
            }
            errorLabel.text = "" // 清空错误
            return true // 全部通过
        }

    Rectangle {
        anchors.fill: parent
        color: "#1c1c1c"

        ScrollView {
            anchors.fill: parent

            Rectangle {
                width: 600
                anchors.horizontalCenter: parent.horizontalCenter
                color: "#222226"
                radius: 16
                anchors.topMargin: 48
                anchors.top: parent.top
                //padding: 32

                ColumnLayout {
                    width: parent.width
                    spacing: 24

                    // 标题
                    Text {
                        text: "客户信息填写"
                        font.pixelSize: 22
                        font.bold: true
                        color: "#ffffff"
                        Layout.alignment: Qt.AlignHCenter
                        Rectangle {
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            width: parent.width
                            height: 1
                            color: "#444444"
                        }
                    }

                    RowLayout {
                        spacing: 32
                        //anchors.horizontalCenter: parent.horizontalCenter

                        // 左侧照片
                        ColumnLayout {
                            spacing: 12
                            width: 150

                            Rectangle {
                                width: 150
                                height: 200
                                radius: 8
                                color: "#ffffff"
                                border.color: "#444444"
                                Image {
                                    id: customerPhoto
                                    anchors.fill: parent
                                    anchors.margins: 0
                                    fillMode: Image.PreserveAspectFit
                                    //source: getPhotoSource(customer.photo, genderBox.currentIndex)
                                }
                            }

                            FileDialog {
                                id: fileDialog
                                title: "选择照片"
                                nameFilters: ["Images (*.png *.jpg *.jpeg *.bmp)"]
                                onAccepted: {
                                    customerPhoto.source = fileDialog.selectedFile
                                    customer.photo = fileDialog.selectedFile.toString()
                                    customer.photo_update = true
                                }
                            }
                            RowLayout {
                                Layout.alignment: Qt.AlignHCenter
                                spacing: 0
                                TextButton {
                                    text: "选择照片"
                                    onClicked: fileDialog.open()
                                }
                            }
                        }

                        // 右侧表单
                        ColumnLayout {
                            spacing: 16
                            Layout.fillWidth: true

                            // 客户编号
                            RowLayout {
                                spacing: 8
                                Label { text: "客户编号"; font.pixelSize: 14; color: "#ffb300"; font.bold: true; Layout.preferredWidth: 90; horizontalAlignment: Text.AlignRight}
                                TextField {
                                    text: customer.id
                                    readOnly: true
                                    Layout.fillWidth: true
                                    background: Rectangle { color: "#777777"; radius: 6; border.color: "#444444"}
                                }
                            }

                            // 客户姓名
                            RowLayout {
                                spacing: 8
                                Label { text: "客户姓名"; font.pixelSize: 14; color: "#ffb300"; font.bold: true; Layout.preferredWidth: 90 ; horizontalAlignment: Text.AlignRight}
                                TextField
                                {
                                    id:nameInput
                                    placeholderText: "请输入客户姓名"
                                    text: customer.name
                                    Layout.fillWidth: true
                                    onTextEdited: customer.name = text
                                    background: Rectangle { color: "#fbeeee"; radius: 6; border.color: "#444444"}
                                }
                            }

                            // 登记日
                            RowLayout {
                                spacing: 8
                                Label { text: "登记日"; font.pixelSize: 14; color: "#ffb300"; font.bold: true; Layout.preferredWidth: 90; horizontalAlignment: Text.AlignRight }
                                DatePicker {
                                    id: createDate
                                    Layout.fillWidth: true
                                    onSelectedDateChanged: customer.date =Qt.formatDate(selectedDate, "yyyy-MM-dd")
                                }

                            }

                            // 性别 + 生日
                            RowLayout {
                                spacing: 8

                                Label { text: "性别"; font.pixelSize: 14; color: "#ffb300"; font.bold: true; Layout.preferredWidth: 90; horizontalAlignment: Text.AlignRight }
                                ComboBox {
                                    id:genderBox
                                    model: ["请选择", "男", "女"]
                                    Layout.preferredWidth: 100
                                    //  文字显示（下拉框未展开时）
                                    contentItem: Text {
                                        text: genderBox.displayText
                                        color: "#000000"          // 黑色文字
                                        font.pixelSize: 14
                                        verticalAlignment: Text.AlignVCenter
                                        elide: Text.ElideRight
                                        leftPadding: 8
                                    }

                                    //  背景
                                    background: Rectangle {
                                        radius: 6
                                        color: "#FFFFFF"          // 白色背景
                                        border.color: "#444444"
                                        border.width: 1
                                    }
                                    currentIndex: customer.gender
                                    onActivated: (index) => { customer.gender = index }
                                }

                                Label { text: "生日"; font.pixelSize: 14; color: "#ffb300"; font.bold: true; Layout.preferredWidth: 70; horizontalAlignment: Text.AlignRight }
                                DatePicker {
                                    id: birthday
                                    Layout.fillWidth: true
                                    onSelectedDateChanged: customer.birthday =Qt.formatDate(selectedDate, "yyyy-MM-dd")

                                }
                            }

                            // Email
                            RowLayout {
                                spacing: 8
                                Label { text: "Email"; font.pixelSize: 14; color: "#ffb300"; font.bold: true; Layout.preferredWidth: 90; horizontalAlignment: Text.AlignRight }
                                TextField {
                                    id:emailInput
                                    placeholderText: "请输入Email"
                                    Layout.fillWidth: true
                                    background: Rectangle { color: "#fbeeee"; radius: 6; border.color: "#444444"}
                                    text: customer.email
                                    onTextEdited: customer.email = text
                                }
                            }

                            // 电话
                            RowLayout {
                                spacing: 8
                                Label { text: "手机号"; font.pixelSize: 14; color: "#ffb300"; font.bold: true; Layout.preferredWidth: 90; horizontalAlignment: Text.AlignRight }
                                TextField {
                                    id:phoneInput
                                    placeholderText: "请输入手机号"
                                    Layout.fillWidth: true
                                    background: Rectangle { color: "#fbeeee"; radius: 6; border.color: "#444444"}
                                    text: customer.phone
                                    onTextEdited: customer.phone = text
                                }
                            }

                            // 按钮
                            RowLayout {
                                spacing: 28
                                Layout.alignment: Qt.AlignRight
                                TextButton { text: "保存"; width: 200; height: 50
                                    onClicked: {
                                        if (!validate()) return
                                        dlg.accepted(customer)
                                        dlg.close()
                                    }
                                }
                                TextButton { text: "取消"; width: 200; height: 50;

                                    onClicked: {
                                        dlg.canceled()
                                        dlg.close()
                                    }
                                }
                            }
                            // 统一的错误提示区域
                            Label {
                                id: errorLabel
                                text: ""
                                color: "red"
                                visible: text !== ""
                                Layout.alignment: Qt.AlignHCenter
                            }

                        } // ColumnLayout 右侧
                    } // RowLayout
                } // ColumnLayout
            } // Rectangle container
        } // ScrollView
    } // Rectangle background
}
