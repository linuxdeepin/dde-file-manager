// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOOKMARKEVENTCALLER_H
#define BOOKMARKEVENTCALLER_H

//#include "dfmplugin_bookmark_global.h"

#include <QObject>

namespace dfmplugin_bookmark {

class BookMarkEventCaller
{
    BookMarkEventCaller() = delete;

public:
    static void sendBookMarkOpenInNewWindow(const QUrl &url);
    static void sendBookMarkOpenInNewTab(quint64 windowId, const QUrl &url);
    static void sendShowBookMarkPropertyDialog(const QUrl &url);
    static void sendOpenBookMarkInWindow(quint64 windowId, const QUrl &url);
    static bool sendCheckTabAddable(quint64 windowId);
};

}

#endif   // BOOKMARKEVENTCALLER_H
