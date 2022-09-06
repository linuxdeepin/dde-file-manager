/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "shareeventscaller.h"
#include "utils/shareutils.h"

#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/dpf.h>

using namespace dfmplugin_myshares;
DFMBASE_USE_NAMESPACE

void ShareEventsCaller::sendOpenDirs(quint64 winId, const QList<QUrl> &urls, ShareEventsCaller::OpenMode mode)
{
    if (urls.count() == 0)
        return;

    QList<QUrl> convertedUrls = urls;
    for (auto &url : convertedUrls) {
        auto u = ShareUtils::convertToLocalUrl(url);
        url = u.isValid() ? u : url;
    }

    if (urls.count() > 1) {
        for (auto url : convertedUrls)
            dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url);
    } else {
        switch (mode) {
        case ShareEventsCaller::OpenMode::kOpenInCurrentWindow:
            dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, winId, convertedUrls.first());
            break;
        case ShareEventsCaller::OpenMode::kOpenInNewWindow:
            dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, convertedUrls.first());
            break;
        case ShareEventsCaller::OpenMode::kOpenInNewTab:
            dpfSignalDispatcher->publish(GlobalEventType::kOpenNewTab, winId, convertedUrls.first());
            break;
        }
    }
}

void ShareEventsCaller::sendCancelSharing(const QUrl &url)
{
    dpfSlotChannel->push("dfmplugin_dirshare", "slot_Share_RemoveShare", url.path());
}

void ShareEventsCaller::sendShowProperty(const QList<QUrl> &urls)
{
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", urls);
}

void ShareEventsCaller::sendSwitchDisplayMode(quint64 winId, Global::ViewMode mode)
{
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kSwitchViewMode, winId, int(mode));
}

void ShareEventsCaller::sendOpenWindow(const QUrl &url)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url);
}

void ShareEventsCaller::sendOpenTab(quint64 windowId, const QUrl &url)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewTab, windowId, url);
}

bool ShareEventsCaller::sendCheckTabAddable(quint64 windowId)
{
    return dpfSlotChannel->push("dfmplugin_workspace", "slot_Tab_Addable", windowId).toBool();
}
