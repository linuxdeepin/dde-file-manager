// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaulteventcaller.h"

#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/event/event.h>

Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(bool *)

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

void VaultEventCaller::sendItemActived(quint64 windowId, const QUrl &url)
{
    dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, windowId, url);
}

void VaultEventCaller::sendOpenWindow(const QUrl &url)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url);
}

void VaultEventCaller::sendOpenTab(quint64 windowId, const QUrl &url)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewTab, windowId, url);
}

void VaultEventCaller::sendVaultProperty(const QUrl &url)
{
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", QList<QUrl>() << url, QVariantHash());
}

void VaultEventCaller::sendBookMarkDisabled(const QString scheme)
{
    dpfSlotChannel->push("dfmplugin_bookmark", "slot_Scheme_Disable", scheme);
}

void VaultEventCaller::sendOpenFiles(const quint64 windowID, const QList<QUrl> &urls)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles, windowID, urls);
}

bool VaultEventCaller::checkTPMAvailable()
{
    return dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_TPMIsAvailablePro").toBool();
}

bool VaultEventCaller::getRandomByTPM(int size, QString *output)
{
    return dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_GetRandomByTPMPro", size, output).toBool();
}

bool VaultEventCaller::isSupportAlgoByTPM(const QString &algoName, bool *support)
{
    return dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_IsTPMSupportAlgoPro", algoName, support).toBool();
}

bool VaultEventCaller::encryptByTPM(const QVariantMap &map)
{
    return dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_EncryptByTPMPro", map).toBool();
}

bool VaultEventCaller::decryptByTPM(const QVariantMap &map, QString *psw)
{
    return dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_DecryptByTPMPro", map, psw).toBool();
}
