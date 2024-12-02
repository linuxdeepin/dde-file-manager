// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recenteventcaller.h"

#include <dfm-base/dfm_event_defines.h>

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
    return dpfSlotChannel->push("dfmplugin_titlebar", "slot_Tab_Addable", windowId).toBool();
}
