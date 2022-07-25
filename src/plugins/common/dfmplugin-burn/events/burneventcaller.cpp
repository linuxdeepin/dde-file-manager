/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "burneventcaller.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/interfaces/abstractjobhandler.h"

#include <dfm-framework/dpf.h>

using namespace dfmplugin_burn;
DFMBASE_USE_NAMESPACE

void BurnEventCaller::sendPasteFiles(const QList<QUrl> &urls, const QUrl &dest, bool isCopy)
{
    if (isCopy)
        dpfSignalDispatcher->publish(GlobalEventType::kCopy, 0, urls, dest, AbstractJobHandler::JobFlag::kNoHint, nullptr);
    else
        dpfSignalDispatcher->publish(GlobalEventType::kCutFile, 0, urls, dest, AbstractJobHandler::JobFlag::kNoHint, nullptr);
}

void BurnEventCaller::sendCloseTab(const QUrl &url)
{
    dpfSlotChannel->push("dfmplugin_workspace", "slot_Tab_Close", url);
}
