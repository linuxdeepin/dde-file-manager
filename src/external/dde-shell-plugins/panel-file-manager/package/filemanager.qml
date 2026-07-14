// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1 as LP
import Qt5Compat.GraphicalEffects

import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D
import org.deepin.ds.dock 1.0

AppletItem {
    id: root

    property bool useColumnLayout: Panel.position % 2
    property int dockOrder: 17
    property bool shouldVisible: true

    // --- Dimension constants ---
    readonly property int hoverBgRadius: 12
    readonly property int gridCellWidth: 90
    readonly property int gridCellHeight: 100
    readonly property int gridThumbSize: 52
    readonly property int gridItemRadius: 6
    readonly property int gridTopMargin: 6
    readonly property int gridTextHPadding: 4
    readonly property real gridTextFontSize: 11
    readonly property int listRowHeight: 36
    readonly property int listThumbSize: 24
    readonly property int listItemRadius: 4
    readonly property int listRowHPadding: 8
    readonly property int listTextMarginRight: 40
    readonly property real listTextFontSize: 12
    readonly property int popupWidth: 480
    readonly property int popupHeight: 400
    readonly property int popupPadding: 8
    readonly property int popupSpacing: 6
    readonly property int toolbarBtnWidth: 32
    readonly property int toolbarBtnHeight: 28
    readonly property int toolbarSpacing: 4
    readonly property real toolbarArrowFontSize: 16
    readonly property real badgeBorderWidth: 0.8
    readonly property int dropBorderWidth: 2
    readonly property int dropRadius: 8
    readonly property int dropInnerMargin: 2
    readonly property int dropInnerRadius: 6
    readonly property int gridItemMargin: 2
    readonly property int listItemMarginH: 2
    readonly property real gridHighlightAlpha: 0.08
    readonly property real listHighlightAlpha: 0.08
    readonly property real gridHoverAlpha: 0.06
    readonly property real listHoverAlpha: 0.06
    readonly property real hoverAlpha: 0.15
    readonly property real pressAlpha: 0.25

    implicitWidth: useColumnLayout ? Panel.rootObject.dockSize : Panel.rootObject.dockItemMaxSize * 0.8
    implicitHeight: useColumnLayout ? Panel.rootObject.dockItemMaxSize * 0.8 : Panel.rootObject.dockSize

    // --- Hover / Press background ---
    property real canvasSize: Math.min(root.width, root.height) * 0.75
    property real hoverSize: canvasSize * 1.4

    Rectangle {
        id: hoverBg
        anchors.centerIn: parent
        width: hoverSize
        height: hoverSize
        radius: hoverBgRadius
        color: "transparent"

        Behavior on color { ColorAnimation { duration: 120 } }

        states: State {
            when: mouseHandler.containsMouse
            PropertyChanges {
                hoverBg.color: Qt.rgba(1.0, 1.0, 1.0, hoverAlpha)
            }
        }
    }

    Rectangle {
        id: pressBg
        anchors.centerIn: parent
        width: hoverSize
        height: hoverSize
        radius: hoverBgRadius
        color: "transparent"

        Behavior on color { ColorAnimation { duration: 80 } }

        states: State {
            when: mouseHandler.pressed
            PropertyChanges {
                pressBg.color: Qt.rgba(1.0, 1.0, 1.0, pressAlpha)
            }
        }
    }

    // --- Tooltip ---
    PanelToolTip {
        id: toolTip
        text: qsTr("Files: %1").arg(Applet.folderCount)
        toolTipX: DockPanelPositioner.x
        toolTipY: DockPanelPositioner.y
    }

    // --- Dock icon: PinnedItemIcon ---
    Item {
        anchors.centerIn: parent
        width: canvasSize
        height: canvasSize

        PinnedItemIcon {
            id: dockIcon
            anchors.fill: parent
            iconSize: parent.width
            iconName: "folder"
            previewIcons: Applet.previewIconNames
            colorTheme: Dock.Dark
        }

        // Count badge when total entries > 4
        Rectangle {
            id: countBadge
            visible: Applet.directoryModel && Applet.directoryModel.totalCount > 4
            width: badgeR * 2
            height: badgeR * 2
            radius: badgeR
            x: parent.width - badgeR * 2 - 2
            y: badgeR

            property real badgeR: parent.width * 0.14

            color: D.DTK.makeColor(D.Color.Highlight)
            border.width: badgeBorderWidth
            border.color: Qt.rgba(1, 1, 1, 0.5)

            Text {
                anchors.centerIn: parent
                color: Qt.rgba(1, 1, 1, 1)
                font.pixelSize: countBadge.badgeR * 1.0
                font.bold: true
                text: Applet.directoryModel ? Applet.directoryModel.totalCount.toString() : ""
            }
        }
    }

    // --- Connections ---
    Connections {
        target: Applet
        function onFolderCountChanged() {
            toolTip.text = qsTr("Files: %1").arg(Applet.folderCount)
        }
    }

    // --- File browser popup ---
    PanelPopup {
        id: filePopup
        popupX: DockPanelPositioner.x
        popupY: DockPanelPositioner.y

        Rectangle {
            id: popupFrame
            width: popupWidth
            height: popupHeight
            color: "transparent"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: popupPadding
                spacing: popupSpacing

                // --- Toolbar ---
                RowLayout {
                    Layout.fillWidth: true
                    spacing: toolbarSpacing

                    Button {
                        flat: true
                        implicitWidth: toolbarBtnWidth
                        implicitHeight: toolbarBtnHeight
                        text: "←"
                        font.pixelSize: toolbarArrowFontSize
                        enabled: Applet.canGoBack
                        onClicked: Applet.goBack()
                    }

                    Button {
                        visible: false
                        flat: true
                        implicitWidth: toolbarBtnWidth
                        implicitHeight: toolbarBtnHeight
                        text: "→"
                        font.pixelSize: toolbarArrowFontSize
                        enabled: Applet.canGoForward
                        onClicked: Applet.goForward()
                    }

                    Label {
                        visible: false
                        Layout.fillWidth: true
                        text: Applet.directoryPath
                        elide: Text.ElideMiddle
                        font.pixelSize: 12
                        opacity: 0.6
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Button {
                        flat: true
                        visible: false
                        implicitWidth: toolbarBtnWidth
                        implicitHeight: toolbarBtnHeight
                        text: "↻"
                        font.pixelSize: toolbarArrowFontSize
                        onClicked: Applet.refreshDirectory()
                    }

                    Button {
                        flat: true
                        visible: false
                        implicitWidth: toolbarBtnWidth
                        implicitHeight: toolbarBtnHeight
                        text: Applet.iconViewMode === 0 ? "☰" : "⊞"
                        font.pixelSize: 14
                        onClicked: Applet.iconViewMode = Applet.iconViewMode === 0 ? 1 : 0
                    }
                }

                // --- File grid (large icon mode) ---
                GridView {
                    id: fileGrid
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    visible: Applet.iconViewMode === 0
                    cellWidth: gridCellWidth
                    cellHeight: gridCellHeight

                    model: Applet.directoryModel
                    currentIndex: -1

                    onVisibleChanged: {
                        if (visible) forceActiveFocus()
                    }

                    highlight: Rectangle {
                        color: Qt.rgba(0, 0, 0, gridHighlightAlpha)
                        radius: gridItemRadius
                    }
                    highlightFollowsCurrentItem: true

                    delegate: Item {
                        width: fileGrid.cellWidth
                        height: fileGrid.cellHeight

                        property bool hovered: gridMouseArea.containsMouse

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: gridItemMargin
                            radius: gridItemRadius
                            color: hovered ? Qt.rgba(0, 0, 0, gridHoverAlpha) : "transparent"
                            visible: hovered
                            Behavior on color { ColorAnimation { duration: 100 } }
                        }

                        Column {
                            anchors.fill: parent
                            anchors.topMargin: gridTopMargin
                            spacing: popupSpacing

                            ThumbnailItem {
                                anchors.horizontalCenter: parent.horizontalCenter
                                thumbnailUrl: model.thumbnailUrl || ""
                                iconName: model.iconName || "text-x-generic"
                                thumbSize: gridThumbSize
                                itemRadius: gridItemRadius
                            }

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: parent.width - gridTextHPadding * 2
                                text: model.fileName
                                font.pixelSize: gridTextFontSize
                                elide: Text.ElideRight
                                horizontalAlignment: Text.AlignHCenter
                                maximumLineCount: 2
                                wrapMode: Text.Wrap
                            }
                        }

                        MouseArea {
                            id: gridMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: fileGrid.currentIndex = index
                            onDoubleClicked: {
                                if (model.isDir) {
                                    Applet.navigateTo(model.filePath)
                                } else {
                                    Applet.openFile(model.filePath)
                                }
                            }
                        }
                    }

                    Keys.onLeftPressed: moveCurrentIndexLeft()
                    Keys.onRightPressed: moveCurrentIndexRight()
                    Keys.onUpPressed: moveCurrentIndexUp()
                    Keys.onDownPressed: moveCurrentIndexDown()
                    Keys.onReturnPressed: {
                        if (currentIndex >= 0) {
                            var entry = Applet.directoryModel.get(currentIndex)
                            if (entry.isDir)
                                Applet.navigateTo(entry.filePath)
                            else
                                Applet.openFile(entry.filePath)
                        }
                    }
                    Keys.onEscapePressed: filePopup.close()

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                }

                // --- File list (small icon mode) ---
                ListView {
                    id: fileList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    visible: Applet.iconViewMode === 1

                    model: Applet.directoryModel
                    currentIndex: -1

                    onVisibleChanged: {
                        if (visible) forceActiveFocus()
                    }

                    highlight: Rectangle {
                        color: Qt.rgba(0, 0, 0, listHighlightAlpha)
                        radius: listItemRadius
                    }
                    highlightFollowsCurrentItem: true

                    delegate: Item {
                        width: fileList.width
                        height: listRowHeight

                        property bool hovered: listMouseArea.containsMouse

                        Rectangle {
                            anchors.fill: parent
                            anchors.leftMargin: listItemMarginH
                            anchors.rightMargin: listItemMarginH
                            radius: listItemRadius
                            color: hovered ? Qt.rgba(0, 0, 0, listHoverAlpha) : "transparent"
                            visible: hovered
                            Behavior on color { ColorAnimation { duration: 100 } }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: listRowHPadding
                            anchors.rightMargin: listRowHPadding
                            spacing: popupSpacing

                            ThumbnailItem {
                                anchors.verticalCenter: parent.verticalCenter
                                thumbnailUrl: model.thumbnailUrl || ""
                                iconName: model.iconName || "text-x-generic"
                                thumbSize: listThumbSize
                                itemRadius: listItemRadius
                            }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width - listTextMarginRight
                                text: model.fileName
                                font.pixelSize: listTextFontSize
                                elide: Text.ElideRight
                            }
                        }

                        MouseArea {
                            id: listMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: fileList.currentIndex = index
                            onDoubleClicked: {
                                if (model.isDir) {
                                    Applet.navigateTo(model.filePath)
                                } else {
                                    Applet.openFile(model.filePath)
                                }
                            }
                        }
                    }

                    Keys.onUpPressed: decrementCurrentIndex()
                    Keys.onDownPressed: incrementCurrentIndex()
                    Keys.onReturnPressed: {
                        if (currentIndex >= 0) {
                            var entry = Applet.directoryModel.get(currentIndex)
                            if (entry.isDir)
                                Applet.navigateTo(entry.filePath)
                            else
                                Applet.openFile(entry.filePath)
                        }
                    }
                    Keys.onEscapePressed: filePopup.close()

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                }
            }
        }
    }

    // --- Mouse handler (no right-click menu) ---
    MouseArea {
        id: mouseHandler
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        hoverEnabled: true

        onClicked: function (mouse) {
            if (mouse.button === Qt.LeftButton) {
                var point = root.mapToItem(null, root.width / 2, root.height / 2)
                filePopup.DockPanelPositioner.bounding = Qt.rect(point.x, point.y, popupFrame.width, popupFrame.height)
                filePopup.open()
            }
        }

        onHoveredChanged: {
            if (mouseHandler.hovered) {
                var point = root.mapToItem(null, root.width / 2, root.height / 2)
                toolTip.DockPanelPositioner.bounding = Qt.rect(point.x, point.y, toolTip.width, toolTip.height)
                toolTip.open()
            } else {
                toolTip.close()
            }
        }
    }

    // --- Drop area for external folder drag ---
    DropArea {
        id: dropArea
        anchors.fill: parent
        keys: ["text/uri-list"]

        property bool dragHovering: false

        onEntered: function (drag) {
            if (drag.urls.length > 0) {
                drag.accepted = true
                dragHovering = true
            } else {
                drag.accepted = false
            }
        }

        onExited: {
            dragHovering = false
        }

        onDropped: function (drop) {
            dragHovering = false
            if (drop.urls.length === 0)
                return
            var localPath = Applet.localPathFromUrl(drop.urls[0].toString())
            if (!localPath)
                return
            if (Applet.isDirectory(localPath) || Applet.isFile(localPath)) {
                var dir = Applet.isDirectory(localPath) ? localPath : localPath.substring(0, localPath.lastIndexOf("/"))
                if (dir)
                    Applet.navigateTo(dir)
            }
            drop.accepted = true
        }

        Rectangle {
            anchors.fill: parent
            radius: dropRadius
            color: "transparent"
            border.width: dropBorderWidth
            border.color: Qt.rgba(0, 0.7, 1, 0.6)
            visible: dropArea.dragHovering

            Rectangle {
                anchors.fill: parent
                anchors.margins: dropInnerMargin
                radius: dropInnerRadius
                color: Qt.rgba(0, 0.7, 1, 0.15)
            }
        }
    }
}
