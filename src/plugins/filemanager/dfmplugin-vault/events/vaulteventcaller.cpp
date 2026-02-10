// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaulteventcaller.h"

#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/event/event.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

void VaultEventCaller::sendItemActived(quint64 windowId, const QUrl &url)
{
    fmDebug() << "Vault: Sending item activated event - windowId:" << windowId << "url:" << url.toString();
    dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, windowId, url);
}

void VaultEventCaller::sendOpenWindow(const QUrl &url)
{
    fmDebug() << "Vault: Sending open window event - url:" << url.toString();
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url);
}

void VaultEventCaller::sendOpenTab(quint64 windowId, const QUrl &url)
{
    fmDebug() << "Vault: Sending open tab event - windowId:" << windowId << "url:" << url.toString();
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewTab, windowId, url);
}

void VaultEventCaller::sendVaultProperty(const QUrl &url)
{
    fmDebug() << "Vault: Sending vault property dialog event - url:" << url.toString();
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", QList<QUrl>() << url, QVariantHash());
}

void VaultEventCaller::sendOpenFiles(const quint64 windowID, const QList<QUrl> &urls)
{
    fmDebug() << "Vault: Sending open files event - windowId:" << windowID << "file count:" << urls.size();
    dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles, windowID, urls);
}
