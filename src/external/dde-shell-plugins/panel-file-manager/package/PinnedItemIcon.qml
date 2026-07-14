// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import org.deepin.ds.dock 1.0
import org.deepin.dtk 1.0 as D

Item {
    id: root

    property string iconName: ""
    property var previewIcons: []
    property int iconSize: 32
    property real visualIconSize: iconSize
    property real sourceIconSize: visualIconSize
    property real visualTranslateY: 0
    property bool smooth: false
    property bool enableBrightnessEffect: false
    property real brightness: 0.0
    property int colorTheme: Dock.Dark
    readonly property var visiblePreviewIcons: {
        const icons = []
        if (!previewIcons) {
            return icons
        }

        for (let i = 0; i < previewIcons.length && icons.length < 4; ++i) {
            const icon = previewIcons[i]
            if (icon && icon.length > 0) {
                icons.push(icon)
            }
        }
        return icons
    }
    readonly property bool useCompositePreview: visiblePreviewIcons.length > 1
    readonly property int desiredCompositeOuterInset: Math.max(2, Math.round(visualIconSize * 0.12))
    readonly property int desiredCompositeGap: Math.max(1, Math.round(visualIconSize * 0.05))
    readonly property int compositeIconSize: Math.max(1, Math.floor((visualIconSize - desiredCompositeOuterInset * 2 - desiredCompositeGap) / 2))
    readonly property int compositeGap: Math.max(1, Math.min(desiredCompositeGap, Math.max(1, visualIconSize - compositeIconSize * 2)))
    readonly property int compositeContentSize: compositeIconSize * 2 + compositeGap
    readonly property int compositeOuterInset: Math.max(1, Math.floor((visualIconSize - compositeContentSize) / 2))

    width: iconSize
    height: iconSize

    Rectangle {
        width: root.visualIconSize
        height: root.visualIconSize
        anchors.centerIn: parent
        radius: Math.max(6, Math.round(root.visualIconSize / 4))
        color: root.colorTheme === Dock.Dark ?
                   Qt.rgba(1, 1, 1, 0.10) :
                   Qt.rgba(0, 0, 0, 0.10)
        border.width: 1
        border.color: root.colorTheme === Dock.Dark ?
                          Qt.rgba(1, 1, 1, 0.40) :
                          Qt.rgba(0, 0, 0, 0.20)
        visible: root.useCompositePreview
        transform: Translate {
            y: root.visualTranslateY
        }
    }

    Item {
        anchors.centerIn: parent
        width: root.compositeContentSize
        height: root.compositeContentSize
        visible: root.useCompositePreview
        transform: Translate {
            y: root.visualTranslateY
        }

        Grid {
            anchors.centerIn: parent
            columns: 2
            rows: 2
            rowSpacing: root.compositeGap
            columnSpacing: root.compositeGap

            Repeater {
                model: root.useCompositePreview ? root.visiblePreviewIcons : 0
                delegate: TaskIcon {
                    required property string modelData

                    iconName: modelData
                    width: root.compositeIconSize
                    height: root.compositeIconSize
                    sourceWidth: root.compositeIconSize
                    sourceHeight: root.compositeIconSize
                    smooth: root.smooth
                    enableBrightnessEffect: root.enableBrightnessEffect
                    brightness: root.brightness
                    retainWhileLoading: true
                }
            }
        }
    }

    TaskIcon {
        anchors.centerIn: parent
        width: root.iconSize
        height: root.iconSize
        visualWidth: root.visualIconSize
        visualHeight: root.visualIconSize
        sourceWidth: root.sourceIconSize
        sourceHeight: root.sourceIconSize
        visualTranslateY: root.visualTranslateY
        iconName: root.visiblePreviewIcons.length === 1 ? root.visiblePreviewIcons[0] : root.iconName
        visible: !root.useCompositePreview
        smooth: root.smooth
        enableBrightnessEffect: root.enableBrightnessEffect
        brightness: root.brightness
        retainWhileLoading: true
    }
}
