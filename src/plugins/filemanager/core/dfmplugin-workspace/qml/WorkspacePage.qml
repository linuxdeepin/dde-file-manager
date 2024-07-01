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

    SplitView {
        id: mainSplitView

        anchors.fill: parent

        handle: Rectangle {
            //     color: SplitHandle.pressed ? "#81e889" : (SplitHandle.hovered ? Qt.lighter("#c2f4c6", 1.1) : "#c2f4c6")
            implicitHeight: 1
            implicitWidth: 1
        }

        Control {
            id: leftContainer

            SplitView.fillHeight: true
            SplitView.fillWidth: true

            contentItem: ViewContainer {
            }
        }
        Control {
            id: rightContainer

            visible: false
        }
    }
}
