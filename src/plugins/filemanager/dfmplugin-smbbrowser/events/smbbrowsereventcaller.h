// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMBBROWSEREVENTCALLER_H
#define SMBBROWSEREVENTCALLER_H

#include "dfmplugin_smbbrowser_global.h"

#include <QObject>

DPSMBBROWSER_BEGIN_NAMESPACE

class SmbBrowserEventCaller
{
    SmbBrowserEventCaller() = delete;

public:
    static void sendOpenWindow(const QUrl &url);
    static void sendOpenTab(quint64 windowId, const QUrl &url);
    static bool sendCheckTabAddable(quint64 windowId);
    static void sendChangeCurrentUrl(quint64 windowId, const QUrl &url);
    static void sendShowPropertyDialog(const QUrl &url);
};

DPSMBBROWSER_END_NAMESPACE

#endif   // SMBBROWSEREVENTCALLER_H
