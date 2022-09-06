/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef SHAREEVENTSCALLER_H
#define SHAREEVENTSCALLER_H

#include "dfmplugin_myshares_global.h"

#include "dfm-base/dfm_global_defines.h"

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
