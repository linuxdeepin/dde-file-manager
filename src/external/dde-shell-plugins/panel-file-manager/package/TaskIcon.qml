// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import Qt5Compat.GraphicalEffects
import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D

Item {
    id: root

    property string iconName: ""
    property bool smooth: false
    property bool retainWhileLoading: true
    property real visualWidth: width
    property real visualHeight: height
    property real sourceWidth: visualWidth
    property real sourceHeight: visualHeight
    property real visualTranslateY: 0
    property bool enableBrightnessEffect: false
    property real brightness: 0.0
    readonly property real sourceImageDevicePixelRatio: Panel.devicePixelRatio > 0
                                                     ? Panel.devicePixelRatio
                                                     : (Screen.devicePixelRatio > 0 ? Screen.devicePixelRatio : 1.0)
    readonly property bool useSourceImage: iconName.indexOf("data:") === 0
                                           || iconName.indexOf("file:") === 0
                                           || iconName.indexOf("qrc:") === 0
                                           || iconName.indexOf("image:") === 0
                                           || iconName.indexOf("http:") === 0
                                           || iconName.indexOf("https:") === 0
                                           || iconName.indexOf(":/") === 0
                                           || iconName.indexOf("/") === 0
    readonly property real dciRenderWidth: Math.max(1, root.sourceWidth)
    readonly property real dciRenderHeight: Math.max(1, root.sourceHeight)
    readonly property real dciVisualScale: Math.min(Math.max(1, root.visualWidth) / dciRenderWidth,
                                                    Math.max(1, root.visualHeight) / dciRenderHeight)

    D.DciIcon {
        width: root.dciRenderWidth
        height: root.dciRenderHeight
        anchors.centerIn: parent
        sourceSize: Qt.size(Math.round(root.dciRenderWidth),
                            Math.round(root.dciRenderHeight))
        name: root.useSourceImage ? "" : root.iconName
        smooth: root.smooth
        retainWhileLoading: root.retainWhileLoading
        visible: root.iconName.length > 0 && !root.useSourceImage
        scale: root.dciVisualScale
        transformOrigin: Item.Center
        transform: Translate {
            y: root.visualTranslateY
        }
        layer.enabled: root.enableBrightnessEffect && visible
        layer.smooth: root.smooth
        layer.effect: BrightnessContrast {
            brightness: root.brightness
            contrast: 0
        }
    }

    Image {
        width: Math.max(1, root.visualWidth)
        height: Math.max(1, root.visualHeight)
        anchors.centerIn: parent
        source: root.useSourceImage ? root.iconName : ""
        sourceSize: Qt.size(Math.max(1, Math.round(root.sourceWidth * root.sourceImageDevicePixelRatio)),
                            Math.max(1, Math.round(root.sourceHeight * root.sourceImageDevicePixelRatio)))
        fillMode: Image.PreserveAspectFit
        asynchronous: true
        smooth: true
        visible: root.iconName.length > 0 && root.useSourceImage
        transform: Translate {
            y: root.visualTranslateY
        }
        layer.enabled: root.enableBrightnessEffect && visible
        layer.smooth: true
        layer.effect: BrightnessContrast {
            brightness: root.brightness
            contrast: 0
        }
    }
}
