// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
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
    fmDebug() << "Sending open window event - URLs count:" << urls.size() << "isNew:" << isNew;

    bool hooked = dpfHookSequence->run(kEventNS, "hook_SendOpenWindow", urls);
    if (hooked) {
        fmDebug() << "Open window event hooked by sequence";
        return;
    }

    if (urls.isEmpty()) {
        fmDebug() << "Publishing open new window event with empty URL";
        dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, QUrl(), isNew);
    } else {
        fmDebug() << "Publishing open new window events for" << urls.size() << "URLs";
        for (const QUrl &url : urls)
            dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url, isNew);
    }
}

void WorkspaceEventCaller::sendOpenNewTab(const quint64 windowId, const QUrl &url)
{
    fmDebug() << "Sending open new tab event - windowId:" << windowId << "URL:" << url.toString();
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewTab, windowId, url);
}

void WorkspaceEventCaller::sendChangeCurrentUrl(const quint64 windowId, const QUrl &url)
{
    fmDebug() << "Sending change current URL event - windowId:" << windowId << "URL:" << url.toString();

    bool hooked = dpfHookSequence->run(kEventNS, "hook_SendChangeCurrentUrl", windowId, url);
    if (hooked) {
        fmDebug() << "Change current URL event hooked by sequence";
        return;
    }

    if (!url.isEmpty()) {
        fmDebug() << "Publishing change current URL event";
        dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, windowId, url);
    }
}

void WorkspaceEventCaller::sendOpenAsAdmin(const QUrl &url)
{
    fmDebug() << "Sending open as admin event for URL:" << url.toString();
    dpfSignalDispatcher->publish(GlobalEventType::kOpenAsAdmin, url);
}

void WorkspaceEventCaller::sendShowCustomTopWidget(const quint64 windowID, const QString &scheme, bool visible)
{
    fmDebug() << "Sending show custom top widget event - windowID:" << windowID << "scheme:" << scheme << "visible:" << visible;
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
    fmDebug() << "Rename start edit event result:" << ret;
    return ret;
}

bool WorkspaceEventCaller::sendRenameEndEdit(const quint64 &winId, const QUrl &url)
{
    bool ret = dpfSignalDispatcher->publish(kEventNS, "signal_View_RenameEndEdit", winId, url);
    fmDebug() << "Rename end edit event result:" << ret;
    return ret;
}

bool WorkspaceEventCaller::sendViewItemClicked(const QVariantMap &data)
{
    bool ret = dpfSignalDispatcher->publish(kEventNS, "signal_View_ItemClicked", data);
    fmDebug() << "View item clicked event result:" << ret;
    return ret;
}

void WorkspaceEventCaller::sendEnterDirReportLog(const QVariantMap &data)
{
    fmDebug() << "Sending enter directory report log - data count:" << data.size();
    dpfSignalDispatcher->publish("dfmplugin_workspace", "signal_ReportLog_Commit", QString("EnterDirectory"), data);
}

void WorkspaceEventCaller::sendModelFilesEmpty()
{
    fmDebug() << "Sending model files empty event";
    dpfSignalDispatcher->publish(kEventNS, "signal_Model_EmptyDir");
}

bool WorkspaceEventCaller::sendCheckTabAddable(quint64 windowId)
{
    return dpfSlotChannel->push("dfmplugin_titlebar", "slot_Tab_Addable", windowId).toBool();
}

void WorkspaceEventCaller::sendCloseTab(const QUrl &url)
{
    fmDebug() << "Sending close tab event for URL:" << url.toString();
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Tab_Close", url);
}

void WorkspaceEventCaller::sendViewModeChanged(quint64 windowId, Global::ViewMode mode)
{
    fmDebug() << "Sending view mode changed event - windowId:" << windowId << "mode:" << static_cast<int>(mode);
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::kSwitchViewMode, windowId, static_cast<int>(mode));
}
