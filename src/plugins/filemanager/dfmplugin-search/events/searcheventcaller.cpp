// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searcheventcaller.h"
#include "utils/searchhelper.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-framework/dpf.h>

namespace dfmplugin_search {

void dfmplugin_search::SearchEventCaller::sendChangeCurrentUrl(quint64 winId, const QUrl &url)
{
    DFMBASE_USE_NAMESPACE
    dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, winId, url);
}

void SearchEventCaller::sendShowAdvanceSearchBar(quint64 winId, bool visible)
{
    dpfSlotChannel->push("dfmplugin_workspace", "slot_ShowCustomTopWidget", winId, SearchHelper::scheme(), visible);
}

void SearchEventCaller::sendShowAdvanceSearchButton(quint64 winId, bool visible)
{
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_FilterButton_Show", winId, visible);
}

void SearchEventCaller::sendStartSpinner(quint64 winId)
{
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Spinner_Start", winId);
}

void SearchEventCaller::sendStopSpinner(quint64 winId)
{
    if (dfmbase::FileManagerWindowsManager::instance().findWindowById(winId))
        dpfSlotChannel->push("dfmplugin_titlebar", "slot_Spinner_Stop", winId);
}

}
