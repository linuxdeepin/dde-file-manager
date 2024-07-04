// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.deepin.filemanager.gui
import org.deepin.dtk
import org.deepin.dtk.private 1.0 as P

///! 面包屑导航栏
Item {
    id: breadcrumbs

    // 请求弹出同级文件夹列表
    signal requsetQuickJumpMenu(var url, real x, real y)

    ListView {
        id: addressline

        anchors.fill: parent
        model: Containment.crumbModel
        orientation: ListView.Horizontal

        delegate: Button {
            id: addressBtn

            enabled: index !== (ListView.view.count - 1)
            icon.name: model.useIcon ? model.icon : undefined
            leftInset: 0
            padding: 0
            rightInset: 0
            spacing: 0
            text: model.useIcon ? undefined : model.text

            background: P.ButtonPanel {
                id: bkgPanel

                button: addressBtn
                implicitWidth: addressBtn.contentItem.implicitWidth
                visible: addressBtn.enabled && addressBtn.hovered
            }
            indicator: Item {
                implicitHeight: 20
                implicitWidth: 10

                Text {
                    anchors.centerIn: parent
                    font.bold: false
                    font.family: "Noto Sans CJK TC"
                    text: "/"
                    visible: !bkgPanel.visible
                }

                Loader {
                    enabled: bkgPanel.visible

                    sourceComponent: Button {
                        id: popupBtn

                        height: 16
                        icon.height: 10
                        icon.name: "combobox_arrow"
                        icon.width: 10
                        visible: bkgPanel.visible
                        width: 16

                        onClicked: {
                            breadcrumbs.requsetQuickJumpMenu(model.fileUrl, popupBtn.x, popupBtn.y + 20);
                        }

                        anchors {
                            right: parent.right
                            rightMargin: 10
                            verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            onClicked: {
                Containment.currentUrl = model.fileUrl;
            }
        }
    }
}
