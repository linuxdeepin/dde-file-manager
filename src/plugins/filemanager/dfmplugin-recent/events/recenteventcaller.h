/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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
#ifndef RECENTEVENTCALLER_H
#define RECENTEVENTCALLER_H

#include "dfmplugin_recent_global.h"

#include "dfm-base/utils/clipboard.h"
#include "dfm-base/interfaces/abstractjobhandler.h"

#include <QObject>

DFMBASE_USE_NAMESPACE
DPRECENT_BEGIN_NAMESPACE

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
};

DPRECENT_END_NAMESPACE

#endif   // RECENTEVENTCALLER_H
