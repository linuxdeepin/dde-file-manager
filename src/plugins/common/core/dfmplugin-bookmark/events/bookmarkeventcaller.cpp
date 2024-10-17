// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bookmarkeventcaller.h"

#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/event/event.h>

#include <QUrl>

using namespace dfmplugin_bookmark;
DFMBASE_USE_NAMESPACE

void BookMarkEventCaller::sendBookMarkOpenInNewWindow(const QUrl &url)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url);
}

void BookMarkEventCaller::sendBookMarkOpenInNewTab(quint64 windowId, const QUrl &url)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewTab, windowId, url);
}

void BookMarkEventCaller::sendShowBookMarkPropertyDialog(const QUrl &url)
{
    QList<QUrl> urls;
    urls << url;
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", urls, QVariantHash());
}

void BookMarkEventCaller::sendOpenBookMarkInWindow(quint64 windowId, const QUrl &url)
{
    dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, windowId, url);
}

bool BookMarkEventCaller::sendCheckTabAddable(quint64 windowId)
{
    return dpfSlotChannel->push("dfmplugin_titlebar", "slot_Tab_Addable", windowId).toBool();
}
