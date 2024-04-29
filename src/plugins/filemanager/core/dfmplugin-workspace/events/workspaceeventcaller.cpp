// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspaceeventcaller.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>

#include <dfm-framework/dpf.h>

DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;
DFMBASE_USE_NAMESPACE

static constexpr char kEventNS[] { DPF_MACRO_TO_STR(DPWORKSPACE_NAMESPACE) };

void WorkspaceEventCaller::sendOpenWindow(const QList<QUrl> &urls, const bool isNew)
{
    bool hooked = dpfHookSequence->run(kEventNS, "hook_SendOpenWindow", urls);
    if (hooked)
        return;

    if (urls.isEmpty()) {
        dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, QUrl(), isNew);
    } else {
        for (const QUrl &url : urls)
            dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url, isNew);
    }
}

void WorkspaceEventCaller::sendChangeCurrentUrl(const quint64 windowId, const QUrl &url)
{
    bool hooked = dpfHookSequence->run(kEventNS, "hook_SendChangeCurrentUrl", windowId, url);
    if (hooked)
        return;

    if (!url.isEmpty())
        dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, windowId, url);
}

void WorkspaceEventCaller::sendOpenAsAdmin(const QUrl &url)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenAsAdmin, url);
}

void WorkspaceEventCaller::sendTabAdded(const quint64 windowID)
{
    dpfSignalDispatcher->publish(kEventNS, "signal_Tab_Added", windowID);
}

void WorkspaceEventCaller::sendTabChanged(const quint64 windowID, const int index)
{
    dpfSignalDispatcher->publish(kEventNS, "signal_Tab_Changed", windowID, index);
}

void WorkspaceEventCaller::sendTabMoved(const quint64 windowID, const int from, const int to)
{
    dpfSignalDispatcher->publish(kEventNS, "signal_Tab_Moved", windowID, from, to);
}

void WorkspaceEventCaller::sendTabRemoved(const quint64 windowID, const int index)
{
    dpfSignalDispatcher->publish(kEventNS, "signal_Tab_Removed", windowID, index);
}

void WorkspaceEventCaller::sendShowCustomTopWidget(const quint64 windowID, const QString &scheme, bool visible)
{
    dpfSlotChannel->push(kEventNS, "slot_ShowCustomTopWidget", windowID, scheme, visible);
}

void WorkspaceEventCaller::sendPaintEmblems(QPainter *painter, const QRectF &paintArea, const FileInfoPointer &info)
{
    dpfSlotChannel->push("dfmplugin_emblem", "slot_FileEmblems_Paint", painter, paintArea, info);
}

void WorkspaceEventCaller::sendViewSelectionChanged(const quint64 windowID, const QItemSelection &selected, const QItemSelection &deselected)
{
    dpfSignalDispatcher->publish(kEventNS, "signal_View_SelectionChanged", windowID, selected, deselected);
}

bool WorkspaceEventCaller::sendRenameStartEdit(const quint64 &winId, const QUrl &url)
{
    bool ret = dpfSignalDispatcher->publish(kEventNS, "signal_View_RenameStartEdit", winId, url);
    return ret;
}

bool WorkspaceEventCaller::sendRenameEndEdit(const quint64 &winId, const QUrl &url)
{
    bool ret = dpfSignalDispatcher->publish(kEventNS, "signal_View_RenameEndEdit", winId, url);
    return ret;
}

bool WorkspaceEventCaller::sendViewItemClicked(const QVariantMap &data)
{
    bool ret = dpfSignalDispatcher->publish(kEventNS, "signal_View_ItemClicked", data);
    return ret;
}

void WorkspaceEventCaller::sendEnterDirReportLog(const QVariantMap &data)
{
    dpfSignalDispatcher->publish("dfmplugin_workspace", "signal_ReportLog_Commit", QString("EnterDirectory"), data);
}

void WorkspaceEventCaller::sendModelFilesEmpty()
{
    dpfSignalDispatcher->publish(kEventNS, "signal_Model_EmptyDir");
}
