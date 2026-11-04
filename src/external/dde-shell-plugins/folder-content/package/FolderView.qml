// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.deepin.dtk 1.0 as D

Item {
    id: root
    required property var presentation
    signal closeRequested()

    // The popup content takes focus so arrow keys and Return reach the grid.
    focus: true

    readonly property var directoryModel: presentation ? presentation.directoryModel : null

    readonly property int minimumWidth: 420
    readonly property int directoryCount: directoryModel ? directoryModel.totalCount : 0
    readonly property int itemCount: directoryCount + 1
    // Same sizing rule as FolderPopup: at most five columns and three visible
    // rows, further entries scroll vertically.
    readonly property int columns: Math.max(1, itemCount <= 4 ? itemCount : Math.min(5, Math.ceil(itemCount / 2)))
    readonly property int rows: Math.max(1, Math.ceil(itemCount / columns))
    readonly property real cellWidth: 96
    readonly property real cellHeight: 104
    // The header tracks the browsed directory, like the file manager title bar.
    readonly property string headerTitle: {
        if (!root.presentation)
            return ""
        const path = (root.presentation.directoryPath || "").replace(/\/+$/, "")
        const name = path.slice(path.lastIndexOf("/") + 1)
        return name.length > 0 ? name : root.presentation.title
    }

    implicitWidth: Math.max(minimumWidth, columns * cellWidth + 24)
    implicitHeight: Math.min(3, rows) * cellHeight + 56

    Keys.onEscapePressed: root.closeRequested()
    Keys.onLeftPressed: flick.moveSelection(-1)
    Keys.onRightPressed: flick.moveSelection(1)
    Keys.onUpPressed: flick.moveSelection(-root.columns)
    Keys.onDownPressed: flick.moveSelection(root.columns)
    Keys.onReturnPressed: flick.activate(flick.currentIndex)
    Keys.onEnterPressed: flick.activate(flick.currentIndex)

    // The popup content must actually own keyboard focus for the arrow/Enter
    // handling above to fire. Besides the construction hook, re-grab focus
    // whenever the popup becomes visible: for freshly pinned folders the
    // window can appear without handing focus to the content item.
    Component.onCompleted: root.forceActiveFocus()
    onVisibleChanged: {
        if (!visible)
            return
        root.forceActiveFocus()
        // Re-probe a mount that was unreachable last time the popup was
        // shown; once the network is back the directory loads on refresh.
        if (root.directoryModel && root.directoryModel.networkBusy)
            root.presentation.refreshDirectory()
    }

    Text {
        id: titleLabel
        // Leave the leading corner to the back button so the title never
        // overlaps it, while staying visually centered.
        anchors.top: parent.top
        anchors.topMargin: 12
        anchors.left: backButton.visible ? backButton.right : parent.left
        anchors.leftMargin: backButton.visible ? 4 : 16
        anchors.right: parent.right
        anchors.rightMargin: 16
        height: 28
        text: root.headerTitle
        textFormat: Text.PlainText
        color: D.DTK.palette.windowText
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        font: D.DTK.fontManager.t6
    }

    // The dock's quick panel uses a bare ActionButton with this icon for back
    // navigation, so the header keeps the same click target on both popups.
    D.ActionButton {
        id: backButton
        objectName: "backButton"
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.top: parent.top
        anchors.topMargin: 12
        width: 28
        height: 28
        visible: root.presentation ? root.presentation.canGoBack : false
        // Framed rounded button: a subtle border at rest, a stronger frame
        // plus a faint highlight on hover/press so it reads as clickable.
        background: Rectangle {
            radius: 8
            color: D.DTK.palette.highlight
            opacity: backButton.pressed ? 0.15 : backButton.hovered ? 0.08 : 0
            border.width: 1
            border.color: backButton.pressed || backButton.hovered
                           ? Qt.rgba(0.5, 0.5, 0.5, 0.8)
                           : Qt.rgba(0.5, 0.5, 0.5, 0.35)
        }
        icon.name: "arrow_ordinary_left"
        icon.width: 16
        icon.height: 16
        Accessible.name: qsTr("Back")
        onClicked: root.presentation.goBack()
    }

    // Horizontal separator under the title once the grid is scrolled, so the
    // header reads as a fixed bar above the scrolling content.
    Rectangle {
        anchors.top: parent.top
        anchors.topMargin: 44
        anchors.left: parent.left
        anchors.leftMargin: 14
        anchors.right: parent.right
        anchors.rightMargin: 14
        height: 1
        visible: flick.contentY > 0.5
        color: Qt.rgba(0.5, 0.5, 0.5, 0.25)
        z: 1
    }

    // Same structure as the dock's FolderPopup: a Flickable hosts a Grid whose
    // children feed straight off the provider's model, so no extra copy of the
    // directory is kept in QML. The last child is always the file manager entry.
    Flickable {
        id: flick
        anchors.top: parent.top
        anchors.topMargin: 44
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.right: parent.right
        anchors.rightMargin: 12
        contentWidth: width
        contentHeight: root.rows * root.cellHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick
        ScrollBar.vertical: ScrollBar {
            id: sb
            // The stock Chameleon style neither moves nor fades its thumb
            // correctly inside dde-shell, so the thumb is hand-positioned
            // (driven by position, which tracks contentY) with a self-made
            // fade: visible while scrolling, fading out when idle.
            policy: ScrollBar.AlwaysOn
            visible: flick.contentHeight > flick.height
            background: null
            contentItem: Rectangle {
                id: thumb
                width: 8
                radius: width / 2
                color: Qt.rgba(0.5, 0.5, 0.5, 0.7)
                y: sb.position * (sb.availableHeight - height)
                height: sb.availableHeight * sb.size
                opacity: 0
                Behavior on opacity { NumberAnimation { duration: 350 } }
                Connections {
                    target: flick
                    function onContentYChanged() {
                        thumb.opacity = 0.75
                        thumbHide.restart()
                    }
                }
                Connections {
                    target: sb
                    function onPressedChanged() {
                        if (sb.pressed) {
                            thumb.opacity = 0.75
                            thumbHide.restart()
                        }
                    }
                }
                Timer {
                    id: thumbHide
                    interval: 1600
                    onTriggered: thumb.opacity = 0
                }
                Component.onCompleted: {
                    // Hint once when the panel opens with scrollable content.
                    thumb.opacity = 0.75
                    thumbHide.restart()
                }
            }
        }

        // Keyboard navigation state: -1 means nothing selected. The trailing
        // "open in file manager" tile is part of the navigation range.
        property int currentIndex: -1

        onCurrentIndexChanged: ensureVisible(currentIndex)

        function ensureVisible(index) {
            if (index < 0 || index >= root.itemCount)
                return
            const row = Math.floor(index / root.columns)
            const top = row * root.cellHeight
            const bottom = top + root.cellHeight
            if (top < flick.contentY)
                flick.contentY = top
            else if (bottom > flick.contentY + flick.height)
                flick.contentY = bottom - flick.height
        }

        function moveSelection(delta) {
            if (root.itemCount <= 0)
                return
            let next = flick.currentIndex
            if (next < 0)
                next = delta > 0 ? 0 : root.itemCount - 1
            else
                next = Math.max(0, Math.min(root.itemCount - 1, next + delta))
            flick.currentIndex = next
        }

        function activate(index) {
            if (index < 0 || index >= root.itemCount)
                return
            // Last slot: reveal the browsed directory in the file manager.
            if (index === root.directoryCount) {
                root.presentation.openInFileManager()
                root.closeRequested()
                return
            }
            if (!root.directoryModel)
                return
            const entry = root.directoryModel.get(index)
            if (!entry || !entry.filePath)
                return
            if (entry.isDir) {
                flick.currentIndex = -1
                root.presentation.navigateTo(entry.filePath)
            } else {
                root.presentation.openFile(entry.filePath)
            }
        }

        // Browsing into another directory replaces the content, so the
        // keyboard selection never carries over between directories.
        Connections {
            target: root.presentation
            function onNavigationChanged() {
                flick.currentIndex = -1
            }
        }

        Grid {
            columns: root.columns
            Repeater {
                model: root.directoryModel
                Item {
                    id: entryDelegate
                    required property int index
                    required property string fileName
                    required property string filePath
                    required property string iconName
                    required property string thumbnailUrl
                    required property bool isDir

                    // Position in the keyboard navigation range, which also
                    // contains the trailing file manager tile.
                    readonly property int navIndex: index

                    // Stretch cells to fill the row so narrow column counts
                    // leave no empty strip on the right.
                    width: flick.width / root.columns
                    height: root.cellHeight

                    // Same hover/pressed feedback as the dock folder popup,
                    // plus a mid level for the keyboard selection.
                    Rectangle {
                        anchors.fill: parent
                        radius: 8
                        color: D.DTK.palette.highlight
                        opacity: pointer.pressed ? 0.25
                               : flick.currentIndex === entryDelegate.navIndex ? 0.2
                               : pointer.containsMouse ? 0.08 : 0
                    }

                    ThumbnailItem {
                        anchors.top: parent.top
                        anchors.topMargin: 4
                        anchors.horizontalCenter: parent.horizontalCenter
                        thumbnailUrl: entryDelegate.thumbnailUrl || ""
                        iconName: entryDelegate.iconName || "text-x-generic"
                        thumbSize: 48
                        itemRadius: 6
                    }

                    Text {
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width
                        height: 36
                        text: entryDelegate.fileName
                        textFormat: Text.PlainText
                        wrapMode: Text.Wrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                        horizontalAlignment: Text.AlignHCenter
                        color: D.DTK.palette.windowText
                        font: D.DTK.fontManager.t8
                    }

                    // A single click activates an entry: directories are entered
                    // and files opened with their default application.
                    MouseArea {
                        id: pointer
                        anchors.fill: parent
                        hoverEnabled: true
                        // Keep the press so the Flickable cannot turn the drag
                        // into a scroll: holding a tile and moving it must start
                        // the copy-drag (scrolling still works via wheel, bar
                        // or empty space).
                        preventStealing: true
                        // Hovering selects, so Enter opens the hovered file.
                        onContainsMouseChanged: {
                            if (pointer.containsMouse && flick.currentIndex !== entryDelegate.navIndex)
                                flick.currentIndex = entryDelegate.navIndex
                        }
                        // Copy-drag: once the press moves past the start-drag
                        // distance hand the file to a real system drag via the
                        // presentation, which offers it as a uri-list so the
                        // desktop or a file manager window copies it on drop.
                        property point pressOrigin
                        property bool dragStarted: false
                        onPressed: (mouse) => {
                            pressOrigin = Qt.point(mouse.x, mouse.y)
                            dragStarted = false
                        }
                        onPositionChanged: (mouse) => {
                            if (!pointer.pressed || dragStarted)
                                return
                            if (Math.abs(mouse.x - pressOrigin.x) > Qt.styleHints.startDragDistance
                                    || Math.abs(mouse.y - pressOrigin.y) > Qt.styleHints.startDragDistance) {
                                dragStarted = true
                                root.presentation.startFileDrag(entryDelegate.filePath, entryDelegate)
                            }
                        }
                        onClicked: {
                            if (dragStarted)
                                return
                            if (entryDelegate.isDir)
                                root.presentation.navigateTo(entryDelegate.filePath)
                            else
                                root.presentation.openFile(entryDelegate.filePath)
                        }
                    }
                }
            }
            // Always last: reveal the browsed directory in the file manager.
            Item {
                readonly property int navIndex: root.directoryCount
                width: flick.width / root.columns
                height: root.cellHeight

                Rectangle {
                    anchors.fill: parent
                    radius: 8
                    color: D.DTK.palette.highlight
                    opacity: fileManagerPointer.pressed ? 0.25
                           : flick.currentIndex === parent.navIndex ? 0.2
                           : fileManagerPointer.containsMouse ? 0.08 : 0
                }

                ThumbnailItem {
                    anchors.top: parent.top
                    anchors.topMargin: 4
                    anchors.horizontalCenter: parent.horizontalCenter
                    iconName: "dde-file-manager"
                    thumbSize: 48
                    itemRadius: 6
                }

                Text {
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width
                    height: 36
                    text: qsTr("Open in file manager")
                    textFormat: Text.PlainText
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                    color: D.DTK.palette.windowText
                    font: D.DTK.fontManager.t8
                }

                MouseArea {
                    id: fileManagerPointer
                    anchors.fill: parent
                    hoverEnabled: true
                    // Hovering selects so Enter works here too.
                    onContainsMouseChanged: {
                        if (fileManagerPointer.containsMouse && flick.currentIndex !== parent.navIndex)
                            flick.currentIndex = parent.navIndex
                    }
                    onClicked: {
                        root.presentation.openInFileManager()
                        root.closeRequested()
                    }
                }
            }
        }
    }

    // Shown when the browsed directory sits on a network mount whose host is
    // unreachable: the model stays empty and file-system access is skipped
    // (see NetworkGuard) so the popup cannot hang the dock UI. Clicking the
    // file manager tile still works and lets the file manager report the
    // broken mount.
    Text {
        anchors.centerIn: flick
        visible: root.directoryModel && root.directoryModel.networkBusy && root.directoryCount === 0
        text: qsTr("Network is unreachable")
        textFormat: Text.PlainText
        color: D.DTK.palette.windowText
        opacity: 0.6
        font: D.DTK.fontManager.t8
    }
}
