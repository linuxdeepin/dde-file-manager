// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.filemanager.gui

AppletItem {
    Layout.fillHeight: true
    Layout.fillWidth: true

    Connections {
        function onSetViewItem(appletItem) {
            centerItem.target = appletItem;
        }

        target: Containment
    }
    Rectangle {
        anchors.fill: parent
        color: palette.base

        ColumnLayout {
            anchors.fill: parent

            LayoutItemProxy {
                id: topItem

            }
            LayoutItemProxy {
                id: centerItem

            }
            LayoutItemProxy {
                id: bottomItem

            }
        }
    }
}
