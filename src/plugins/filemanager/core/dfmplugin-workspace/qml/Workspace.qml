// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.filemanager.gui

ContainmentItem {
    property int widgetType: QuickUtils.WorkSpace

    Layout.fillHeight: true
    Layout.fillWidth: true

    Connections {
        function onPushNewPage(appletItem) {
            workspacePageStack.push(appletItem);
        }

        target: Containment
    }
    Rectangle {
        anchors.fill: parent
        color: palette.base

        StackView {
            id: workspacePageStack

            anchors.fill: parent
        }
    }
}
