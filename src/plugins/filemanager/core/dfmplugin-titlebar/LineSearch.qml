// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls

TextEdit {
    width: 200

    TextField {
        anchors.fill: parent
        horizontalAlignment: TextInput.AlignCenter
        placeholderText: qsTr("Search")
    }
}
