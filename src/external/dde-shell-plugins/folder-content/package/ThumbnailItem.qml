// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import Qt5Compat.GraphicalEffects
import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D

Item {
    id: root

    property string thumbnailUrl: ""
    property string iconName: ""
    property int thumbSize: 48
    property int itemRadius: 6

    readonly property real borderPixel: 1 / (Screen.devicePixelRatio > 0 ? Screen.devicePixelRatio : 1.0)
    readonly property bool thumbReady: thumbImg.status === Image.Ready

    width: thumbSize
    height: thumbSize

    Image {
        id: thumbImg
        anchors.fill: parent
        source: root.thumbnailUrl || ""
        sourceSize.width: root.thumbSize
        sourceSize.height: root.thumbSize
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        cache: false
        smooth: true
        visible: false
    }

    Rectangle {
        id: thumbMask
        anchors.fill: parent
        radius: root.itemRadius
        color: "white"
        visible: false
    }

    OpacityMask {
        anchors.fill: parent
        source: thumbImg
        maskSource: thumbMask
        visible: root.thumbReady
    }

    D.InsideBoxBorder {
        anchors.fill: parent
        radius: root.itemRadius
        color: Qt.rgba(1, 1, 1, 0.14)
        borderWidth: root.borderPixel
        visible: root.thumbReady
    }

    D.OutsideBoxBorder {
        anchors.fill: parent
        radius: root.itemRadius
        color: Qt.rgba(0, 0, 0, 0.12)
        borderWidth: root.borderPixel
        visible: root.thumbReady
    }

    D.DciIcon {
        anchors.fill: parent
        name: root.iconName || "text-x-generic"
        sourceSize: Qt.size(root.thumbSize, root.thumbSize)
        fillMode: Image.PreserveAspectFit
        visible: !root.thumbReady
        smooth: true
    }
}
