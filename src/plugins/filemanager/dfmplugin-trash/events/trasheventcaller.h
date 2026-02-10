// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHEVENTCALLER_H
#define TRASHEVENTCALLER_H

#include "dfmplugin_trash_global.h"

#include <QObject>

namespace dfmplugin_trash {

class TrashEventCaller
{
    TrashEventCaller() = delete;

public:
    static void sendOpenWindow(const QUrl &url);
    static void sendOpenTab(quint64 windowId, const QUrl &url);
    static void sendOpenFiles(const quint64 windowID, const QList<QUrl> &urls);
    static void sendEmptyTrash(const quint64 windowId, const QList<QUrl> &urls);
    static void sendTrashPropertyDialog(const QUrl &url);
    static void sendShowEmptyTrash(quint64 winId, bool visible);
    static bool sendCheckTabAddable(quint64 windowId);
};

}

#endif   // TRASHEVENTCALLER_H
