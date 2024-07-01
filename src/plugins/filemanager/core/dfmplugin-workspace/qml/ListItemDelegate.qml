// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.dtk
import org.deepin.filemanager.gui

ItemDelegate {
    id: listItemDelegate

    backgroundVisible: index % 2 === 0
    hoverEnabled: true
    icon.name: fileIcon
    text: fileDisplayName
    width: ListView.view.width

    content: RowLayout {
        Label {
            text: fileLastModifiedTime
        }
        Label {
            text: fileSize
        }
        Label {
            text: fileMimeType
        }
    }

    MouseArea {
        acceptedButtons: Qt.LeftButton
        anchors.fill: parent

        // onClicked: {
        //     console.warn(index, " item clicked.");
        //     listview.model.openIndex(index);
        //     console.warn(Containment.getWinId());
        // }

        onDoubleClicked: {
            listview.model.openIndex(Containment.getWinId(), index);
        }
    }
}
