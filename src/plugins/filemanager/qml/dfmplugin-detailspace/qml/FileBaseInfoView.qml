// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import org.deepin.dtk
import org.deepin.filemanager.gui
import org.deepin.filemanager.detailspace

Item {
    property int fieldWidth: 100
    property int valueWidth: 150

    BaseFileInfoModel {
        id: baseModel

        url: Containment.currentUrl
    }

    ListView {
        anchors.fill: parent
        interactive: false
        model: baseModel

        delegate: Row {
            Text {
                id: field

                elide: Text.ElideMiddle
                font.bold: true
                horizontalAlignment: Text.AlignLeft
                text: model.key
                width: fieldWidth
            }

            Text {
                id: value

                elide: Text.ElideMiddle
                horizontalAlignment: Text.AlignLeft
                text: model.value
                width: valueWidth
            }
        }
    }
}
