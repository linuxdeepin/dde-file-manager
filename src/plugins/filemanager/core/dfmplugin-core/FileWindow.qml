// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts
import org.dfm.base
import org.dfm.declarative
import org.deepin.dtk

ApplicationWindow {
    id: root

    DWindow.enabled: false
    flags: Qt.Window | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
    height: 600
    width: 800

    // TODO: 待评估方案
    Containment.onAppletRootObjectChanged: appletItem => {
        if (!appletItem) {
            return;
        }
        if (undefined !== appletItem.widgetType) {
            var control;
            switch (appletItem.widgetType) {
            case QuickUtils.Sidebar:
                sidebar.contentItem = appletItem;
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

    SplitView {
        anchors.fill: parent

        Control {
            id: sidebar

            contentItem: Rectangle {
                width: 200
            }
        }

        ColumnLayout {
            SplitView.fillHeight: true
            SplitView.fillWidth: true

            LayoutItemProxy {
                id: titlebar

            }

            RowLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true

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
