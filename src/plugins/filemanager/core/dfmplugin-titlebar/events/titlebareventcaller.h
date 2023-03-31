// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TITLEBAREVENTCALLER_H
#define TITLEBAREVENTCALLER_H

#include "dfmplugin_titlebar_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <QObject>

namespace dfmplugin_titlebar {

class TitleBarEventCaller
{
    TitleBarEventCaller() = delete;

public:
    static void sendViewMode(QWidget *sender, DFMBASE_NAMESPACE::Global::ViewMode mode);
    static void sendDetailViewState(QWidget *sender, bool checked);
    static void sendCd(QWidget *sender, const QUrl &url);
    static void sendOpenFile(QWidget *sender, const QUrl &url);
    static void sendOpenWindow(const QUrl &url);
    static void sendOpenTab(quint64 windowId, const QUrl &url);
    static void sendSearch(QWidget *sender, const QString &keyword);
    static void sendStopSearch(QWidget *sender);
    static void sendShowFilterView(QWidget *sender, bool visible);
    static void sendCheckAddressInputStr(QWidget *sender, QString *str);
    static bool sendCheckTabAddable(quint64 windowId);
    static DFMGLOBAL_NAMESPACE::ViewMode sendGetDefualtViewMode(const QString &scheme);
};

}

#endif   // TITLEBAREVENTCALLER_H
