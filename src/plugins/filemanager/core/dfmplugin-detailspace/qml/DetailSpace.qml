// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import org.deepin.dtk
import org.dfm.base

ContainmentItem {
    id: detailspace

    property int widgetType: QuickUtils.DetailSpace

    Layout.fillHeight: true
    implicitWidth: 250

    Containment.onAppletRootObjectChanged: appletItem => {
        itemModel.append(appletItem);
    }

    Rectangle {
        anchors.fill: parent
        border.color: "blue"
        border.width: 1
        color: "transparent"
    }

    ColumnLayout {
        anchors.fill: parent

        Text {
            color: "lightgray"
            font.pointSize: 36
            text: "Detail Space"
        }

        Loader {
            Layout.fillHeight: true
            Layout.fillWidth: true
            active: detailspace.visible

            sourceComponent: FileBaseInfoView {
                id: infoView

                anchors.fill: parent
            }
        }
    }
}
