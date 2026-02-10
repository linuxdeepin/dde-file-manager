// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREEVENTSCALLER_H
#define SHAREEVENTSCALLER_H

#include "dfmplugin_myshares_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <QObject>

namespace dfmplugin_myshares {

class ShareEventsCaller
{
    ShareEventsCaller() = delete;

public:
    enum OpenMode {
        kOpenInCurrentWindow,
        kOpenInNewWindow,
        kOpenInNewTab,
    };
    static void sendOpenDirs(quint64 winId, const QList<QUrl> &urls, OpenMode mode);
    static void sendCancelSharing(const QUrl &url);
    static void sendShowProperty(const QList<QUrl> &urls);
    static void sendSwitchDisplayMode(quint64 winId, DFMBASE_NAMESPACE::Global::ViewMode mode);
    static void sendOpenWindow(const QUrl &url);
    static void sendOpenTab(quint64 windowId, const QUrl &url);
    static bool sendCheckTabAddable(quint64 windowId);
};

}

#endif   // SHAREEVENTSCALLER_H
