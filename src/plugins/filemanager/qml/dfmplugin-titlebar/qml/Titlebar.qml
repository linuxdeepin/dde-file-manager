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
    // 控件类型
    property int widgetType: QuickUtils.Titlebar

    Layout.fillWidth: true
    implicitHeight: DS.Style.titleBar.height + breadcrumbsHeight

    Window.onWindowChanged: {
        if (Window.window) {
            topLeftCorner.parent = Window.window.contentItem;
            topLeftCorner.x = 0;
            topLeftCorner.y = 0;
        }
    }

    // 左上角的
    Row {
        id: topLeftCorner

        // 是否允许当前执行切换，进行动画时不允许
        property bool enableSwitch: true

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
                Panel.showSidebar = !Panel.showSidebar;
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

            Loader {
                Layout.fillWidth: true
                active: null !== Window.window
                height: DS.Style.titleBar.height

                // TODO: DTK.TitleBar 左侧有默认的间距，不过设计图间距较大，如可行就无需调整
                sourceComponent: DTK.TitleBar {
                    title: ""
                    width: parent.width

                    leftContent: RowLayout {
                        layoutDirection: Qt.LeftToRight

                        Item {
                            // 需要 Titlebar Applet 已追加到 Panel 中
                            Layout.leftMargin: Panel.showSidebar ? 0 : topLeftCorner.width

                            Behavior on Layout.leftMargin {
                                NumberAnimation {
                                    id: switchSidebar

                                    duration: 200
                                    easing.type: Easing.InOutQuad

                                    onRunningChanged: {
                                        topLeftCorner.enableSwitch = !running;
                                    }
                                }
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
                        }
                    }
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
