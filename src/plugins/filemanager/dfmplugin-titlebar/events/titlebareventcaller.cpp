// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlebareventcaller.h"
#include "utils/titlebarhelper.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/dpf.h>

using namespace dfmplugin_titlebar;
DFMGLOBAL_USE_NAMESPACE

void TitleBarEventCaller::sendViewMode(QWidget *sender, DFMBASE_NAMESPACE::Global::ViewMode mode)
{
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kSwitchViewMode, id, int(mode));
}

void TitleBarEventCaller::sendDetailViewState(QWidget *sender, bool checked)
{
    quint64 id = TitleBarHelper::windowId(sender);
    if (id < 1) {
        fmWarning() << "Cannot send detail view state: invalid window id" << id;
        return;
    }
    dpfSlotChannel->push("dfmplugin_detailspace", "slot_DetailView_Show", id, checked);
}

void TitleBarEventCaller::sendCd(QWidget *sender, const QUrl &url)
{
    DFMBASE_USE_NAMESPACE
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);
    if (!url.isValid()) {
        fmWarning() << "Cannot change directory: invalid URL" << url;
        return;
    }

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kChangeCurrentUrl, id, url);
}

void TitleBarEventCaller::sendChangeCurrentUrl(QWidget *sender, const QUrl &url)
{
    quint64 id = TitleBarHelper::windowId(sender);
    if (id > 0 && url.isValid()) {
        dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kChangeCurrentUrl, id, url);
    } else {
        fmWarning() << "Cannot send URL change: invalid window id or URL, id:" << id << "URL:" << url;
    }
}

void TitleBarEventCaller::sendOpenFile(QWidget *sender, const QUrl &url)
{
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);

    fmInfo() << "Sending open file signal, window id:" << id << "file:" << url.toString();
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFiles, id, QList<QUrl>() << url);
}

void TitleBarEventCaller::sendOpenWindow(const QUrl &url)
{
    fmInfo() << "Sending open new window signal, URL:" << url.toString();
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenNewWindow, url);
}

void TitleBarEventCaller::sendOpenTab(quint64 windowId, const QUrl &url)
{
    fmInfo() << "Sending open new tab signal, window id:" << windowId << "URL:" << url.toString();
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenNewTab, windowId, url);
}

void TitleBarEventCaller::sendSearch(QWidget *sender, const QString &keyword)
{
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);

    fmInfo() << "Sending search start signal, window id:" << id << "keyword:" << keyword;
    dpfSignalDispatcher->publish("dfmplugin_titlebar", "signal_Search_Start", id, keyword);
}

void TitleBarEventCaller::sendStopSearch(QWidget *sender)
{
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);
    dpfSignalDispatcher->publish("dfmplugin_titlebar", "signal_Search_Stop", id);
}

void TitleBarEventCaller::sendShowFilterView(QWidget *sender, bool visible)
{
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);
    dpfSignalDispatcher->publish("dfmplugin_titlebar", "signal_FilterView_Show", id, visible);
}

void TitleBarEventCaller::sendCheckAddressInputStr(QWidget *sender, QString *str)
{
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);
    dpfSignalDispatcher->publish("dfmplugin_titlebar", "signal_InputAdddressStr_Check", id, str);
}

void TitleBarEventCaller::sendTabChanged(QWidget *sender, const QString &uniqueId)
{
    quint64 windowId = TitleBarHelper::windowId(sender);
    dpfSignalDispatcher->publish("dfmplugin_titlebar", "signal_Tab_Changed", windowId, uniqueId);
}

void TitleBarEventCaller::sendTabCreated(QWidget *sender, const QString &uniqueId)
{
    quint64 windowId = TitleBarHelper::windowId(sender);
    dpfSignalDispatcher->publish("dfmplugin_titlebar", "signal_Tab_Created", windowId, uniqueId);
}

void TitleBarEventCaller::sendTabRemoved(QWidget *sender, const QString &removedId, const QString &nextId)
{
    quint64 windowId = TitleBarHelper::windowId(sender);
    dpfSignalDispatcher->publish("dfmplugin_titlebar", "signal_Tab_Removed", windowId, removedId, nextId);
}

ViewMode TitleBarEventCaller::sendGetDefualtViewMode(const QString &scheme)
{
    int defaultViewMode = dpfSlotChannel->push("dfmplugin_workspace", "slot_View_GetDefaultViewMode", scheme).toInt();
    return static_cast<ViewMode>(defaultViewMode);
}

ItemRoles TitleBarEventCaller::sendCurrentSortRole(QWidget *sender)
{
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);
    int currentSortRole = dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_CurrentSortRole", id).toInt();
    return static_cast<ItemRoles>(currentSortRole);
}

void TitleBarEventCaller::sendSetSort(QWidget *sender, ItemRoles role)
{
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);
    dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetSort", id, role);
}

ItemRoles TitleBarEventCaller::sendCurrentGroupRole(QWidget *sender)
{
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);
    int currentGroupRole = dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_CurrentGroupStrategy", id).toInt();
    return static_cast<ItemRoles>(currentGroupRole);
}

void TitleBarEventCaller::sendSetGroup(QWidget *sender, ItemRoles role)
{
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);
    dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetGroup", id, role);
}
