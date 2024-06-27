// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls

TextEdit {
    property bool visibleState: true

    width: 200

    Behavior on width {
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    TextField {
        anchors.fill: parent
        horizontalAlignment: TextInput.AlignCenter
        placeholderText: qsTr("Search")
    }
}
