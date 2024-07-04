// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBAREVENTCALLER_H
#define SIDEBAREVENTCALLER_H

#include "dfmplugin_sidebar_global.h"

#include <QObject>

DPSIDEBAR_BEGIN_NAMESPACE

class SideBarEventCaller
{
    SideBarEventCaller() = delete;

public:
    static void sendItemActived(quint64 windowId, const QUrl &url);
    static void sendEject(const QUrl &url);
    static void sendOpenWindow(const QUrl &url, const bool isNew = true);
    static void sendOpenTab(quint64 windowId, const QUrl &url);
    static void sendShowFilePropertyDialog(const QUrl &url);
    static bool sendCheckTabAddable(quint64 windowId);
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBAREVENTCALLER_H
