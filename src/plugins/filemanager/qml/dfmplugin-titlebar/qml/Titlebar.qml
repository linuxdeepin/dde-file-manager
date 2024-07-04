// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.filemanager.gui
import org.deepin.dtk as DTK
import org.deepin.dtk.style 1.0 as DS

ContainmentItem {
    id: titlebar

    property int breadcrumbsHeight: 30
    // 是否显示侧边栏
    property bool sidebarVisible: true
    // 顶栏一层的高度
    property alias topHeaderHeight: topHeader.height
    // 控件类型
    property int widgetType: QuickUtils.Titlebar

    // TODO 移动到框架事件后移除
    signal sidebarVisibleNotify(bool bVisible)

    function updateTopLeftLayout() {
        Qt.callLater(sidebarVisibleNotify, sidebarVisible);
    }

    Layout.fillWidth: true
    implicitHeight: DS.Style.titleBar.height + breadcrumbsHeight

    Component.onCompleted: {
        updateTopLeftLayout();
    }
    Window.onWindowChanged: {
        if (Window.window) {
            topLeftCorner.parent = Window.window.contentItem;
            topLeftCorner.x = 0;
            topLeftCorner.y = 0;
        }
        updateTopLeftLayout();
    }
    onSidebarVisibleChanged: {
        updateTopLeftLayout();
    }

    Row {
        id: topLeftCorner

        height: DS.Style.titleBar.height
        leftPadding: 5
        rightPadding: 5

        DTK.IconLabel {
            anchors.verticalCenter: parent.verticalCenter
            height: 36
            width: 36

            icon {
                height: 24
                name: "dde-file-manager"
                width: 24
            }
        }

        DTK.ToolButton {
            anchors.verticalCenter: parent.verticalCenter
            height: 36
            width: 36

            onClicked: {
                if (!switchSidebar.running) {
                    titlebar.sidebarVisible = !titlebar.sidebarVisible;
                }
            }

            icon {
                height: 48
                name: "window_sidebar"
                width: 48
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            id: topHeader

            Layout.preferredHeight: DS.Style.titleBar.height

            RowLayout {
                Layout.fillHeight: true
                Layout.leftMargin: sidebarVisible ? 0 : topLeftCorner.width
                layoutDirection: Qt.LeftToRight

                Behavior on Layout.leftMargin {
                    NumberAnimation {
                        id: switchSidebar

                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                }

                DTK.IconButton {
                    icon.name: "arrow_ordinary_left"
                }

                DTK.IconButton {
                    icon.name: "arrow_ordinary_right"
                }

                TabBar {
                    TabButton {
                        text: qsTr("Home")
                        width: implicitWidth
                    }

                    TabButton {
                        text: qsTr("Discover")
                        width: implicitWidth
                    }

                    TabButton {
                        text: qsTr("Activity")
                        width: implicitWidth
                    }
                }

                DTK.IconButton {
                    icon.name: "button_add"

                    onClicked: {
                        console.warn("--- test", Containment.applets);
                        Applet.currentUrl = "file:///home/uos/Videos/dde-introduction.mp4";
                    }
                }
            }

            Loader {
                Layout.fillWidth: true
                active: null !== Window.window
                height: DS.Style.titleBar.height

                sourceComponent: DTK.TitleBar {
                    title: ""
                    width: parent.width
                }
            }
        }

        RowLayout {
            id: breadcrumbsLayout

            Layout.fillWidth: true
            implicitHeight: breadcrumbsHeight
            spacing: 0

            CrumbBar {
                id: breadcrumbs

                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            OptionButtonBox {
                height: parent.height
            }

            LineSearch {
                id: searchbar

                visibleState: {
                    return titlebar.width > 600;
                }

                onVisibleStateChanged: {
                    Layout.preferredWidth = visibleState ? 200 : 0;
                }
            }
        }
    }
}
