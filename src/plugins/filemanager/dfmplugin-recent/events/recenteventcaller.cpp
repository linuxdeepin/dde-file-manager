/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#include "recenteventcaller.h"

#include "services/filemanager/sidebar/sidebar_defines.h"
#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/framework.h>
#include <QUrl>

DPRECENT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

static DPF_NAMESPACE::EventDispatcherManager *dispatcher()
{
    return &dpfInstance.eventDispatcher();
}

void RecentEventCaller::sendOpenWindow(const QUrl &url)
{
    dispatcher()->publish(GlobalEventType::kOpenNewWindow, url);
}

void RecentEventCaller::sendOpenTab(quint64 windowId, const QUrl &url)
{
    dispatcher()->publish(GlobalEventType::kOpenNewTab, windowId, url);
}

void RecentEventCaller::sendOpenFiles(const quint64 windowID, const QList<QUrl> &urls)
{
    dispatcher()->publish(GlobalEventType::kOpenFiles, windowID, urls);
}

void RecentEventCaller::sendWriteToClipboard(const quint64 windowID, const ClipBoard::ClipboardAction action, const QList<QUrl> &urls)
{
    dispatcher()->publish(GlobalEventType::kWriteUrlsToClipboard, windowID, action, urls);
}

void RecentEventCaller::sendCopyFiles(const quint64 windowID, const QList<QUrl> &sourceUrls, const QUrl &target, const AbstractJobHandler::JobFlags flags)
{
    dispatcher()->publish(GlobalEventType::kCopy, windowID, sourceUrls, target, flags);
}

void RecentEventCaller::sendCutFiles(const quint64 windowID, const QList<QUrl> &sourceUrls, const QUrl &target, const AbstractJobHandler::JobFlags flags)
{
    dispatcher()->publish(GlobalEventType::kCutFile, windowID, sourceUrls, target, flags);
}
