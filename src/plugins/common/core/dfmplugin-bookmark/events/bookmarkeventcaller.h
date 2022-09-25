/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng <gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge <zhengyouge@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
    static void sendDefaultItemActived(quint64 windowId, const QUrl &url);
};

}

#endif   // BOOKMARKEVENTCALLER_H
