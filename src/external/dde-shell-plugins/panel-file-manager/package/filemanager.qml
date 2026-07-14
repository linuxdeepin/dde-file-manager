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
        radius: 12
        color: "transparent"

        Behavior on color { ColorAnimation { duration: 120 } }

        states: State {
            when: mouseHandler.containsMouse
            PropertyChanges {
                hoverBg.color: Qt.rgba(1.0, 1.0, 1.0, 0.15)
            }
        }
    }

    Rectangle {
        id: pressBg
        anchors.centerIn: parent
        width: hoverSize
        height: hoverSize
        radius: 12
        color: "transparent"

        Behavior on color { ColorAnimation { duration: 80 } }

        states: State {
            when: mouseHandler.pressed
            PropertyChanges {
                pressBg.color: Qt.rgba(1.0, 1.0, 1.0, 0.25)
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

    // --- Dock icon: PinnedItemIcon (replaces Canvas) ---
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

            color: "#0096C7"
            border.width: 0.8
            border.color: "rgba(255,255,255,0.5)"

            Text {
                anchors.centerIn: parent
                color: "#FFFFFF"
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
            width: 480
            height: 400
            color: "transparent"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6

                // --- Toolbar ---
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Button {
                        flat: true
                        implicitWidth: 32
                        implicitHeight: 28
                        text: "←"
                        font.pixelSize: 16
                        enabled: Applet.canGoBack
                        onClicked: Applet.goBack()
                    }

                    Button {
                        visible: false
                        flat: true
                        implicitWidth: 32
                        implicitHeight: 28
                        text: "→"
                        font.pixelSize: 16
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
                        implicitWidth: 32
                        implicitHeight: 28
                        text: "↻"
                        font.pixelSize: 16
                        onClicked: Applet.refreshDirectory()
                    }

                    Button {
                        flat: true
                        visible: false
                        implicitWidth: 32
                        implicitHeight: 28
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
                    cellWidth: 90
                    cellHeight: 100

                    model: Applet.directoryModel
                    currentIndex: -1

                    onVisibleChanged: {
                        if (visible) forceActiveFocus()
                    }

                    highlight: Rectangle {
                        color: Qt.rgba(0, 0, 0, 0.08)
                        radius: 6
                    }
                    highlightFollowsCurrentItem: true

                    delegate: Item {
                        width: fileGrid.cellWidth
                        height: fileGrid.cellHeight

                        property bool hovered: gridMouseArea.containsMouse

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 2
                            radius: 6
                            color: hovered ? Qt.rgba(0, 0, 0, 0.06) : "transparent"
                            visible: hovered
                            Behavior on color { ColorAnimation { duration: 100 } }
                        }

                        Column {
                            anchors.fill: parent
                            anchors.topMargin: 6
                            spacing: 4

                            // --- Thumbnail with OpacityMask ---
                            Item {
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: 52
                                height: 52

                                Image {
                                    id: gridThumbImg
                                    anchors.fill: parent
                                    source: model.thumbnailUrl || ""
                                    sourceSize.width: 52
                                    sourceSize.height: 52
                                    fillMode: Image.PreserveAspectCrop
                                    asynchronous: true
                                    cache: false
                                    smooth: true
                                    visible: false
                                }

                                Rectangle {
                                    id: gridThumbMask
                                    anchors.fill: parent
                                    radius: 6
                                    color: "white"
                                    visible: false
                                }

                                OpacityMask {
                                    anchors.fill: parent
                                    source: gridThumbImg
                                    maskSource: gridThumbMask
                                    visible: gridThumbImg.status === Image.Ready
                                }

                                D.InsideBoxBorder {
                                    anchors.fill: parent
                                    radius: 6
                                    color: Qt.rgba(1, 1, 1, 0.14)
                                    borderWidth: 1 / Screen.devicePixelRatio
                                    visible: gridThumbImg.status === Image.Ready
                                }

                                D.OutsideBoxBorder {
                                    anchors.fill: parent
                                    radius: 6
                                    color: Qt.rgba(0, 0, 0, 0.12)
                                    borderWidth: 1 / Screen.devicePixelRatio
                                    visible: gridThumbImg.status === Image.Ready
                                }

                                // Fallback icon when thumbnail not ready
                                Image {
                                    anchors.fill: parent
                                    source: model.iconUrl
                                    sourceSize.width: 52
                                    sourceSize.height: 52
                                    fillMode: Image.PreserveAspectFit
                                    visible: gridThumbImg.status !== Image.Ready
                                }
                            }

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: parent.width - 8
                                text: model.fileName
                                font.pixelSize: 11
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
                        color: Qt.rgba(0, 0, 0, 0.08)
                        radius: 4
                    }
                    highlightFollowsCurrentItem: true

                    delegate: Item {
                        width: fileList.width
                        height: 36

                        property bool hovered: listMouseArea.containsMouse

                        Rectangle {
                            anchors.fill: parent
                            anchors.leftMargin: 2
                            anchors.rightMargin: 2
                            radius: 4
                            color: hovered ? Qt.rgba(0, 0, 0, 0.06) : "transparent"
                            visible: hovered
                            Behavior on color { ColorAnimation { duration: 100 } }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            spacing: 8

                            // --- Thumbnail with OpacityMask for list view ---
                            Item {
                                anchors.verticalCenter: parent.verticalCenter
                                width: 24
                                height: 24

                                Image {
                                    id: listThumbImg
                                    anchors.fill: parent
                                    source: model.thumbnailUrl || ""
                                    sourceSize.width: 24
                                    sourceSize.height: 24
                                    fillMode: Image.PreserveAspectCrop
                                    asynchronous: true
                                    cache: false
                                    smooth: true
                                    visible: false
                                }

                                Rectangle {
                                    id: listThumbMask
                                    anchors.fill: parent
                                    radius: 4
                                    color: "white"
                                    visible: false
                                }

                                OpacityMask {
                                    anchors.fill: parent
                                    source: listThumbImg
                                    maskSource: listThumbMask
                                    visible: listThumbImg.status === Image.Ready
                                }

                                D.InsideBoxBorder {
                                    anchors.fill: parent
                                    radius: 4
                                    color: Qt.rgba(1, 1, 1, 0.14)
                                    borderWidth: 1 / Screen.devicePixelRatio
                                    visible: listThumbImg.status === Image.Ready
                                }

                                D.OutsideBoxBorder {
                                    anchors.fill: parent
                                    radius: 4
                                    color: Qt.rgba(0, 0, 0, 0.12)
                                    borderWidth: 1 / Screen.devicePixelRatio
                                    visible: listThumbImg.status === Image.Ready
                                }

                                Image {
                                    anchors.fill: parent
                                    source: model.iconUrl
                                    sourceSize.width: 24
                                    sourceSize.height: 24
                                    fillMode: Image.PreserveAspectFit
                                    visible: listThumbImg.status !== Image.Ready
                                }
                            }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width - 40
                                text: model.fileName
                                font.pixelSize: 12
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
            var localPath = drop.urls[0].toString().replace(/^file:\/\//, "")
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
            radius: 8
            color: "transparent"
            border.width: 2
            border.color: Qt.rgba(0, 0.7, 1, 0.6)
            visible: dropArea.dragHovering

            Rectangle {
                anchors.fill: parent
                anchors.margins: 2
                radius: 6
                color: Qt.rgba(0, 0.7, 1, 0.15)
            }
        }
    }
}
