// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaulthelperreceiver.h"
#include "vaultassitcontrol.h"

#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/event/event.h>

#include <dfm-io/dfmio_utils.h>

#include <QApplication>

DPUTILS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

VaultHelperReceiver::VaultHelperReceiver(QObject *parent)
    : QObject(parent)
{
}

void VaultHelperReceiver::initEventConnect()
{
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_MoveToTrash",
                            this, &VaultHelperReceiver::handlemoveToTrash);
}

void VaultHelperReceiver::callBackFunction(const AbstractJobHandler::CallbackArgus args)
{
    JobHandlePointer jobHandle = args->value(AbstractJobHandler::CallbackKey::kJobHandle).value<JobHandlePointer>();
    if (jobHandle) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        connect(jobHandle.get(), &AbstractJobHandler::finishedNotify, this, &VaultHelperReceiver::handleFinishedNotify);
    }
}

bool VaultHelperReceiver::handlemoveToTrash(const quint64 windowId,
                                            const QList<QUrl> &sources,
                                            const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    if (sources.isEmpty())
        return false;
    if (!VaultAssitControl::instance()->isVaultFile(sources.first()))
        return false;

    DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback = std::bind(&VaultHelperReceiver::callBackFunction, this, std::placeholders::_1);
    QList<QUrl> localFileUrls = VaultAssitControl::instance()->transUrlsToLocal(sources);
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles,
                                 windowId,
                                 localFileUrls, flags, Q_NULLPTR, QVariant(), callback);

    return true;
}

void VaultHelperReceiver::handleFinishedNotify(const JobInfoPointer &jobInfo)
{
    Q_UNUSED(jobInfo)

    disconnect(qobject_cast<AbstractJobHandler*>(sender()), &AbstractJobHandler::finishedNotify, this, &VaultHelperReceiver::handleFinishedNotify);
    QApplication::restoreOverrideCursor();
}
