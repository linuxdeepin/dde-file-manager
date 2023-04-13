// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaulthelperreceiver.h"
#include "vaultassitcontrol.h"

#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/event/event.h>

#include <dfm-io/dfmio_utils.h>

DPUTILS_USE_NAMESPACE

VaultHelperReceiver::VaultHelperReceiver(QObject *parent)
    : QObject(parent)
{
}

void VaultHelperReceiver::initEventConnect()
{
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_MoveToTrash",
                            this, &VaultHelperReceiver::handlemoveToTrash);
}

bool VaultHelperReceiver::handlemoveToTrash(const quint64 windowId,
                                            const QList<QUrl> &sources,
                                            const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    if (sources.isEmpty())
        return false;
    if (!VaultAssitControl::instance()->isVaultFile(sources.first()))
        return false;

    QList<QUrl> localFileUrls = VaultAssitControl::instance()->transUrlsToLocal(sources);
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles,
                                 windowId,
                                 localFileUrls, flags, Q_NULLPTR);
    return true;
}
