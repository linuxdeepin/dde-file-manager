// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.dfm.base

ContainmentItem {
    property int widgetType: QuickUtils.WorkSpace

    Layout.fillHeight: true
    Layout.fillWidth: true

    Component.onCompleted:
    // do init Workspace
    {
    }

    SplitView {
        id: workspaceMainSpliteContainer

        anchors.fill: parent

        Rectangle {
            id: tempViewRect

            SplitView.fillWidth: true
            SplitView.minimumWidth: 200
            color: "lightblue"

            Text {
                anchors.centerIn: parent
                color: "black"
                font.pointSize: 36
                text: "Workspace"
            }
        }
    }
}
