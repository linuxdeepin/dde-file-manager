// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.dtk
import org.deepin.filemanager.gui

RowLayout {
    property int iconSize: 28

    ButtonGroup {
        buttons: viewMode.children
        exclusive: true
    }

    Row {
        id: viewMode

        ToolButton {
            id: iconView

            checkable: true

            icon {
                height: iconSize
                name: "unchecked"
                width: iconSize
            }
        }

        ToolButton {
            id: listView

            checkable: true

            icon {
                height: iconSize
                name: "unchecked"
                width: iconSize
            }
        }

        ToolButton {
            id: treeView

            checkable: true

            icon {
                height: iconSize
                name: "unchecked"
                width: iconSize
            }
        }
    }

    ToolButton {
        id: detail

        checkable: true

        Component.onCompleted: {
            checked = Containment.showDetail;
        }
        onCheckedChanged: {
            Containment.showDetail = checked;
        }

        icon {
            height: iconSize
            name: "splitscreen_right"
            width: iconSize
        }
    }
}
