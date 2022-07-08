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

#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/dpf.h>
#include <QUrl>

using namespace dfmplugin_recent;
DFMBASE_USE_NAMESPACE

void RecentEventCaller::sendOpenWindow(const QUrl &url)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url);
}

void RecentEventCaller::sendOpenTab(quint64 windowId, const QUrl &url)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewTab, windowId, url);
}

void RecentEventCaller::sendOpenFiles(const quint64 windowID, const QList<QUrl> &urls)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles, windowID, urls);
}

void RecentEventCaller::sendWriteToClipboard(const quint64 windowID, const ClipBoard::ClipboardAction action, const QList<QUrl> &urls)
{
    dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard, windowID, action, urls);
}

void RecentEventCaller::sendCopyFiles(const quint64 windowID, const QList<QUrl> &sourceUrls, const QUrl &target, const AbstractJobHandler::JobFlags flags)
{
    dpfSignalDispatcher->publish(GlobalEventType::kCopy, windowID, sourceUrls, target, flags, nullptr);
}

void RecentEventCaller::sendCutFiles(const quint64 windowID, const QList<QUrl> &sourceUrls, const QUrl &target, const AbstractJobHandler::JobFlags flags)
{
    dpfSignalDispatcher->publish(GlobalEventType::kCutFile, windowID, sourceUrls, target, flags, nullptr);
}

bool RecentEventCaller::sendCheckTabAddable(quint64 windowId)
{
    return dpfSlotChannel->push("dfmplugin_workspace", "slot_Tab_Addable", windowId).toBool();
}
