// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlebareventcaller.h"
#include "utils/titlebarhelper.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/dpf.h>

Q_DECLARE_METATYPE(QString *)

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
    Q_ASSERT(id > 0);
    dpfSlotChannel->push("dfmplugin_detailspace", "slot_DetailView_Show", id, checked);
}

void TitleBarEventCaller::sendCd(QWidget *sender, const QUrl &url)
{
    DFMBASE_USE_NAMESPACE
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);
    if (!url.isValid()) {
        fmWarning() << "Invalid url: " << url;
        return;
    }

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kChangeCurrentUrl, id, url);
}

void TitleBarEventCaller::sendOpenFile(QWidget *sender, const QUrl &url)
{
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFiles, id, QList<QUrl>() << url);
}

void TitleBarEventCaller::sendOpenWindow(const QUrl &url)
{
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenNewWindow, url);
}

void TitleBarEventCaller::sendOpenTab(quint64 windowId, const QUrl &url)
{
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenNewTab, windowId, url);
}

void TitleBarEventCaller::sendSearch(QWidget *sender, const QString &keyword)
{
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);
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

bool TitleBarEventCaller::sendCheckTabAddable(quint64 windowId)
{
    return dpfSlotChannel->push("dfmplugin_workspace", "slot_Tab_Addable", windowId).toBool();
}

ViewMode TitleBarEventCaller::sendGetDefualtViewMode(const QString &scheme)
{
    int defaultViewMode = dpfSlotChannel->push("dfmplugin_workspace", "slot_View_GetDefaultViewMode", scheme).toInt();
    return static_cast<ViewMode>(defaultViewMode);
}

void TitleBarEventCaller::sendDetailViewState(dfmgui::Applet *applet, bool checked)
{
    quint64 id = TitleBarHelper::windowId(applet);
    Q_ASSERT(id > 0);
    dpfSlotChannel->push("dfmplugin_detailspace", "slot_DetailView_Show", id, checked);
}

void TitleBarEventCaller::sendCd(dfmgui::Applet *applet, const QUrl &url)
{
    DFMBASE_USE_NAMESPACE
    quint64 id = TitleBarHelper::windowId(applet);
    Q_ASSERT(id > 0);
    if (!url.isValid()) {
        fmWarning() << "Invalid url: " << url;
        return;
    }

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kChangeCurrentUrl, id, url);
}
