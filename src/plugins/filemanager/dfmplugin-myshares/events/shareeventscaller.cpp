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

#include "services/common/propertydialog/property_defines.h"
#include "services/common/usershare/usershare_defines.h"
#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/framework.h>

DPMYSHARES_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

#define dispatcher dpfInstance.eventDispatcher()

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
            dispatcher.publish(GlobalEventType::kOpenNewWindow, url);
    } else {
        switch (mode) {
        case ShareEventsCaller::OpenMode::kOpenInCurrentWindow:
            dispatcher.publish(GlobalEventType::kChangeCurrentUrl, winId, convertedUrls.first());
            break;
        case ShareEventsCaller::OpenMode::kOpenInNewWindow:
            dispatcher.publish(GlobalEventType::kOpenNewWindow, convertedUrls.first());
            break;
        case ShareEventsCaller::OpenMode::kOpenInNewTab:
            dispatcher.publish(GlobalEventType::kOpenNewTab, winId, convertedUrls.first());
            break;
        }
    }
}

void ShareEventsCaller::sendCancelSharing(const QUrl &url)
{
    dispatcher.publish(DSC_NAMESPACE::EventType::kRemoveShare, url.path());
}

void ShareEventsCaller::sendShowProperty(const QList<QUrl> &urls)
{
    dispatcher.publish(DSC_NAMESPACE::Property::EventType::kEvokePropertyDialog, urls);
}

void ShareEventsCaller::sendSwitchDisplayMode(quint64 winId, Global::ViewMode mode)
{
    dispatcher.publish(DFMBASE_NAMESPACE::GlobalEventType::kSwitchViewMode, winId, int(mode));
}
