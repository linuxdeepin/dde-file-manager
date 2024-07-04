// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts

/*!
 * 用于边栏的横向分裂器，提供拖拽控件，动画交互功能
 */
Item {
    id: spliter

    // 是否响应动画触发事件
    property alias enableAnimation: trans.enabled
    // 是否响应 Hover 和拖拽效果
    property alias enableMouse: mouseLoader.active
    // 控件展开控制， expand 更新为 true 时触发弹出动画，为 false 时触发隐藏动画
    property bool expand: true
    property alias handle: handleLoader.sourceComponent
    property int handleWidth: 6
    // 分裂器附加 anchor 位置，允许在父控件左侧或右侧的边缘，计算方向相反
    property bool leftSide: false
    property real maximumWidth: 600
    property real minimumWidth: 100
    // 缓存手动拖拽的宽度，用于动画恢复
    property real showWidth: 200
    property Item target: parent
    property alias transitionsRunning: trans.running

    function adjustTargetWidth(xOffset) {
        var adjWidth;
        if (null === spliter.target) {
            return;
        }
        adjWidth = spliter.target.width + (leftSide ? -xOffset : xOffset);
        adjWidth = Math.min(Math.max(minimumWidth, adjWidth), maximumWidth);
        setTargetWidth(adjWidth);
    }

    function setTargetWidth(adjWidth) {
        // 缓存用于动画的宽度
        showWidth = adjWidth;
        if (expand) {
            // 外部组件可能被布局管理
            if (null !== target.Layout) {
                target.Layout.preferredWidth = adjWidth;
            } else {
                target.width = adjWidth;
            }
        }
    }

    implicitHeight: target ? target.height : 0
    implicitWidth: handleWidth

    // 拖拽也会触发属性变更,因此未使用 Behaviour
    states: [
        State {
            name: "show"
            when: spliter.expand

            PropertyChanges {
                Layout.preferredWidth: showWidth
                explicit: true
                target: spliter.target
            }
        },
        State {
            name: "hide"
            when: !spliter.expand

            PropertyChanges {
                Layout.preferredWidth: 0
                explicit: true
                target: spliter.target
            }
        }
    ]
    transitions: Transition {
        id: trans

        onRunningChanged: {
            if (running && spliter.expand) {
                spliter.target.visible = true;
            } else if (!running && !spliter.expand) {
                spliter.target.visible = false;
            }
        }

        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
            property: "Layout.preferredWidth"
        }
    }

    anchors {
        left: leftSide ? target.left : undefined
        right: leftSide ? undefined : target.right
    }

    Loader {
        id: handleLoader

        sourceComponent: Item {
            height: spliter.height
            width: handleWidth
        }
    }

    Loader {
        id: mouseLoader

        anchors.fill: parent

        sourceComponent: MouseArea {
            property real baseX

            anchors.fill: parent
            cursorShape: (containsMouse || pressed) ? Qt.SplitHCursor : Qt.ArrowCursor
            hoverEnabled: true

            onPositionChanged: {
                if (pressed) {
                    spliter.adjustTargetWidth(mouseX - baseX);
                }
            }
            onPressed: {
                baseX = mouseX;
            }
        }
    }
}
