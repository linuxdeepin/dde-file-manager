/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#include "searcheventcaller.h"
#include "utils/searchhelper.h"

#include "dfm-base/dfm_event_defines.h"

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
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Spinner_Stop", winId);
}

}
