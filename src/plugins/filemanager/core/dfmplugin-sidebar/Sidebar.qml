// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import Qt.labs.folderlistmodel 2.6
import org.deepin.filemanager.gui

AppletItem {
    property int widgetType: QuickUtils.Sidebar

    Layout.fillHeight: true
    implicitWidth: 200

    ListView {
        id: example

        anchors.fill: parent
        clip: true
        delegate: fileDelegate
        model: folderModel

        Component {
            id: fileDelegate

            Text {
                text: fileName
            }
        }
        FolderListModel {
            id: folderModel

            folder: "File:///home"
            nameFilters: ["*"]
            showDirs: true
            showDirsFirst: true
        }
    }
    // Text {
    //     color: "lightgray"
    //     font.pointSize: 36
    //     text: "sidebar"
    // }
}
