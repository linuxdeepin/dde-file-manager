// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.dtk
import org.deepin.filemanager.gui
import org.deepin.filemanager.declarative

ApplicationWindow {
    id: root

    DWindow.enabled: true
    flags: Qt.Window | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
    height: 600
    minimumHeight: 600
    minimumWidth: 800
    width: 800

    // TODO: 待评估方案
    Containment.onAppletRootObjectChanged: appletItem => {
        if (!appletItem) {
            return;
        }
        if (undefined !== appletItem.widgetType) {
            switch (appletItem.widgetType) {
            case QuickUtils.Sidebar:
                sidebar.target = appletItem;
                break;
            case QuickUtils.Titlebar:
                titlebar.target = appletItem;
                break;
            case QuickUtils.WorkSpace:
                workspace.target = appletItem;
                break;
            case QuickUtils.DetailSpace:
                detailspace.target = appletItem;
                break;
            default:
                return;
            }
            console.warn("Append applet item", appletItem, appletItem.widgetType);
        } else {
            console.warn("Append invalid applet item", appletItem, appletItem.applet);
        }
    }

    // For local module test
    ActionMenu {
    }
    Connections {
        function onWidthChanged(width) {
        }

        enabled: Window.window !== null
        target: Window.window
    }
    RowLayout {
        anchors.fill: parent

        Item {
            id: sidebarProxy

            Layout.preferredHeight: parent.height

            ColumnLayout {
                id: sidebarLayoutContent

                anchors.fill: parent

                // 用于同步标题栏高度占位的区块
                Rectangle {
                    id: titlebarCorner

                    Layout.preferredHeight: titlebar.target ? titlebar.target.topHeaderHeight : titlebar.height
                    Layout.preferredWidth: parent.width
                    color: "lightyellow"
                }
                LayoutItemProxy {
                    id: sidebar

                    Layout.fillHeight: true
                }
            }
            AnimationHSpliter {
                id: spliter

                enableAnimation: sidebar.target !== null
                height: parent.height
                target: sidebarProxy

                // TODO 移动到 Sidebar, 通过事件处理，而不是 QML 隐式传输
                Connections {
                    function onSidebarVisibleNotify(bVisible) {
                        spliter.switchShow = bVisible;
                    }

                    enabled: titlebar.target !== null
                    target: titlebar.target
                }
            }
        }
        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true

            LayoutItemProxy {
                id: titlebar

                onTargetChanged: target => {}
            }
            RowLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 0

                LayoutItemProxy {
                    id: workspace

                }
                LayoutItemProxy {
                    id: detailspace

                }
            }
        }
    }
}
