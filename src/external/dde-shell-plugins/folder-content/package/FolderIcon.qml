// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import org.deepin.dtk 1.0 as D

Rectangle {
    id: root
    required property var presentation
    readonly property var contents: presentation ? presentation.previewIcons : []
    readonly property bool empty: contents.length === 0
    radius: height / 6
    // The translucent plate only frames the preview grid; an empty folder just
    // shows the folder glyph on the bare dock background.
    color: root.empty ? "transparent" : Qt.rgba(255 / 255.0, 255 / 255.0, 255 / 255.0, 0.2)

    D.BoxShadow {
        anchors.fill: parent
        z: -1
        shadowColor: Qt.rgba(0, 0, 0, 0.1)
        shadowBlur: 10
        shadowOffsetX: 0
        shadowOffsetY: 0
        cornerRadius: root.radius
        spread: 0
        hollow: true
        visible: contents.length !== 0
    }

    // Empty folders still have to read as a folder rather than a blank square.
    D.DciIcon {
        objectName: "folderPlaceholder"
        anchors.centerIn: parent
        width: root.width
        height: width
        sourceSize: Qt.size(width, height)
        name: "folder"
        visible: root.empty
    }

    Item {
        anchors.fill: parent
        anchors.margins: root.width / 8
        Repeater {
            model: 4
            D.DciIcon {
                required property int index
                objectName: "folderPreview" + index
                width: (root.width * 3 / 4 - root.width / 12) / 2
                height: width
                name: index < root.contents.length ? root.contents[index].iconName : ""
                visible: index < root.contents.length
                sourceSize: Qt.size(width, height)
                // Invisible slots still participate in the fixed 2x2 geometry.
                x: (index % 2) * (width + root.width / 12)
                y: Math.floor(index / 2) * (height + root.width / 12)
            }
        }
    }
}
