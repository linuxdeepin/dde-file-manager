// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebareventcaller.h"

#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/event/event.h>

#include <QUrl>

DPSIDEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void SideBarEventCaller::sendItemActived(quint64 windowId, const QUrl &url)
{
    dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, windowId, url);
}

void SideBarEventCaller::sendEject(const QUrl &url)
{
    fmInfo() << "Eject device: " << url;
    dpfSignalDispatcher->publish("dfmplugin_sidebar", "signal_Item_EjectClicked", url);
}

void SideBarEventCaller::sendOpenWindow(const QUrl &url, const bool isNew)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url, isNew);
}

void SideBarEventCaller::sendOpenTab(quint64 windowId, const QUrl &url)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewTab, windowId, url);
}

void SideBarEventCaller::sendShowFilePropertyDialog(const QUrl &url)
{
    QList<QUrl> urls;
    urls << url;
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", urls, QVariantHash());
}

bool SideBarEventCaller::sendCheckTabAddable(quint64 windowId)
{
    return dpfSlotChannel->push("dfmplugin_titlebar", "slot_Tab_Addable", windowId).toBool();
}
