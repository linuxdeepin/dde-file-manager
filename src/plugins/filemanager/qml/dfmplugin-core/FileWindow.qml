// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.dtk
import org.deepin.dtk.style 1.0 as DS
import org.deepin.filemanager.gui
import org.deepin.filemanager.declarative

ApplicationWindow {
    id: root

    // 布局Item
    property alias detailview: detailviewProxy.target
    // 右侧布局的内容的最小宽度，用于自动隐藏侧边栏
    property real rightContentMinimumWidth: 600
    property alias sidebar: sidebarProxy.target
    property alias titlebar: titlebarProxy.target
    property alias workspace: workspaceProxy.target

    function adpatSidebarOnce() {
        showSidebarAction.enabled = true;
    }

    DWindow.enabled: true
    flags: Qt.Window | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
    height: 600
    minimumHeight: 600
    minimumWidth: 800
    width: 800

    // TODO: 待评估方案
    Panel.onAppletRootObjectChanged: appletItem => {
        if (!appletItem) {
            return;
        }
        if (undefined !== appletItem.widgetType) {
            switch (appletItem.widgetType) {
            case QuickUtils.Sidebar:
                sidebar = appletItem;
                break;
            case QuickUtils.Titlebar:
                titlebar = appletItem;
                break;
            case QuickUtils.WorkSpace:
                workspace = appletItem;
                break;
            case QuickUtils.DetailSpace:
                detailview = appletItem;
                break;
            default:
                return;
            }
            console.warn("Append applet item", appletItem, appletItem.widgetType);
        } else {
            console.warn("Append invalid applet item", appletItem, appletItem.applet);
        }
    }
    Panel.onSidebarStateChanged: (width, manualHide, autoHide) => {
        spliter.target.visible = Panel.showSidebar;
        spliter.setTargetWidth(width);
        // 状态标识在界面完成更新后设置
        spliter.manualHideSidebar = manualHide;
        spliter.autoHideSidebar = autoHide;
    }
    onClosing: {
        // 关闭前缓存当前侧边栏状态
        Panel.setSidebarState(spliter.showWidth, spliter.manualHideSidebar, spliter.autoHideSidebar);
    }

    RowLayout {
        anchors.fill: parent

        Item {
            id: leftContent

            Layout.preferredHeight: parent.height

            ColumnLayout {
                id: sidebarLayoutContent

                anchors.fill: parent

                LayoutItemProxy {
                    id: sidebarProxy

                    Layout.fillHeight: true
                    // TODO 明确设计图后，高度属性移动到 QuickUtils 统一管理，不关联组件
                    Layout.topMargin: DS.Style.titleBar.height
                }
            }
            AnimationHSpliter {
                id: spliter

                // 自动隐藏侧边栏标识
                property bool autoHideSidebar: false
                property real constMaxWidth: 600
                // 手动操作的隐藏侧边栏
                property bool manualHideSidebar: false

                // 切换手动隐藏状态
                function switchMaunualHide(bVisible) {
                    if (transitionsRunning)
                        return;
                    manualHideSidebar = !bVisible;
                    autoHideSidebar = false;

                    // 特殊处理，当展开时窗口的大小过小时，会调整窗口大小
                    if (expand) {
                        if (showWidth > maximumWidth) {
                            changeWindowWidth.to = root.width + showWidth - maximumWidth;
                            changeWindowWidth.start();
                        }
                    }
                }

                enableAnimation: root.sidebar !== null
                expand: Panel.showSidebar
                height: parent.height
                // 当前允许的宽度
                maximumWidth: Math.min(root.width - root.rightContentMinimumWidth, constMaxWidth)
                minimumWidth: 100
                target: leftContent

                onExpandChanged: {
                    switchMaunualHide(expand);
                }
                onMaximumWidthChanged: {
                    // 手动隐藏的状态优先
                    if (manualHideSidebar) {
                        return;
                    }

                    // 超过最小的显示范围，自动隐藏
                    if (showWidth > maximumWidth) {
                        Panel.showSidebar = false;
                        // Note: 需要触发变更后更新状态
                        autoHideSidebar = true;
                        manualHideSidebar = false;
                    } else if (autoHideSidebar) {
                        Panel.showSidebar = true;
                        autoHideSidebar = false;
                    }
                }

                NumberAnimation {
                    id: changeWindowWidth

                    duration: 200
                    easing.type: Easing.InOutQuad
                    property: "width"
                    target: root
                }
            }
        }
        ColumnLayout {
            id: rightContent

            Layout.fillHeight: true
            Layout.fillWidth: true

            LayoutItemProxy {
                id: titlebarProxy

            }
            RowLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 0

                LayoutItemProxy {
                    id: workspaceProxy

                }
                LayoutItemProxy {
                    id: detailviewProxy

                }
            }
        }
    }
}
