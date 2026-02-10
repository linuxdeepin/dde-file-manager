// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTEVENTCALLER_H
#define RECENTEVENTCALLER_H

#include "dfmplugin_recent_global.h"

#include <dfm-base/utils/clipboard.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QObject>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_recent {

class RecentEventCaller
{
    RecentEventCaller() = delete;

public:
    static void sendOpenWindow(const QUrl &url);
    static void sendOpenTab(quint64 windowId, const QUrl &url);
    static void sendOpenFiles(const quint64 windowID, const QList<QUrl> &urls);
    static void sendWriteToClipboard(const quint64 windowId, const ClipBoard::ClipboardAction action, const QList<QUrl> &urls);

    static void sendCopyFiles(const quint64 windowId, const QList<QUrl> &sourceUrls,
                              const QUrl &target,
                              const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    static void sendCutFiles(const quint64 windowId, const QList<QUrl> &sourceUrls,
                             const QUrl &target,
                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    static bool sendCheckTabAddable(quint64 windowId);
};

}

#endif   // RECENTEVENTCALLER_H
