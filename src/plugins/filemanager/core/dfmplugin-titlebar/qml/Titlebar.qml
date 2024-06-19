// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import org.dfm.base
import org.deepin.dtk
import org.deepin.dtk.style 1.0 as DS

ContainmentItem {
    id: titlebar

    property int breadcrumbsHeight: 30
    // 控件类型
    property int widgetType: QuickUtils.Titlebar

    Layout.fillWidth: true
    implicitHeight: DS.Style.titleBar.height + breadcrumbsHeight

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: DS.Style.titleBar.height

            RowLayout {
                Layout.fillHeight: true
                layoutDirection: Qt.LeftToRight

                CheckBox {
                    id: _internal_check

                }

                IconButton {
                    icon.name: "arrow_ordinary_left"
                }

                IconButton {
                    icon.name: "arrow_ordinary_right"
                }

                TabBar {
                    Layout.fillWidth: true

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

                IconButton {
                    icon.name: "button_add"

                    onClicked: {
                        console.warn("--- test", Containment.applets);
                        Applet.currentUrl = "file:///home/uos/Downloads/GammaRay/build/bin";
                    }
                }
            }

            Loader {
                Layout.fillWidth: true
                active: Window.window
                height: DS.Style.titleBar.height

                sourceComponent: TitleBar {
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

            Rectangle {
                id: tools

                Layout.fillHeight: true
                color: "yellow"
                width: 100
            }

            LineSearch {
                id: searchbar

                Layout.fillHeight: true
                visible: titlebar.width > 500
            }
        }
    }
}
