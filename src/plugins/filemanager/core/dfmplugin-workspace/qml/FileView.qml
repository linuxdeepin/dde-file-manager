// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.filemanager.gui
import org.deepin.filemanager.workspace

AppletItem {
    Layout.fillHeight: true
    Layout.fillWidth: true

    Connections {
        function onCurrentUrlChanged(url) {
            listview.model.setRootUrl(url);
        }

        target: Containment
    }
    ScrollView {
        anchors.fill: parent
        clip: true

        ListView {
            id: listview

            delegate: ListItemDelegate {
                autoExclusive: false
            }
            model: FileItemModel {
            }
        }
    }
}
