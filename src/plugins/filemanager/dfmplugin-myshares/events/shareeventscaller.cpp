// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shareeventscaller.h"
#include "utils/shareutils.h"

#include <dfm-base/dfm_event_defines.h>

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
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", urls, QVariantHash());
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
    return dpfSlotChannel->push("dfmplugin_titlebar", "slot_Tab_Addable", windowId).toBool();
}
