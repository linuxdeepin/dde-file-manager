// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import org.dfm.base

ContainmentItem {
    property int widgetType: QuickUtils.DetailSpace

    Layout.fillHeight: true
    implicitWidth: 200

    ColumnLayout {
        anchors.fill: parent

        Rectangle {
            Layout.fillHeight: true
            color: "lightyellow"
            width: 200
        }
    }

    Text {
        color: "lightgray"
        font.pointSize: 36
        text: "Detail Space"
    }
}
