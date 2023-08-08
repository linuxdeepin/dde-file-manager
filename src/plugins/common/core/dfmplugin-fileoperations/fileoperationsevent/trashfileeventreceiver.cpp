// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashfileeventreceiver.h"
#include "fileoperations/filecopymovejob.h"
#include "fileoperationsevent/fileoperationseventhandler.h"

#include <dfm-base/utils/hidefilehelper.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/properties.h>
#include <dfm-base/utils/systempathutil.h>

#include <dfm-io/dfmio_utils.h>

#include <DDesktopServices>

#include <QDebug>
#include <QtConcurrent>
#include <QCoreApplication>

Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(bool *)
Q_DECLARE_METATYPE(QString *)

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

TrashFileEventReceiver::TrashFileEventReceiver(QObject *parent)
    : QObject(parent)
{
    copyMoveJob.reset(new FileCopyMoveJob);
    connect(this, &TrashFileEventReceiver::cleanTrashUrls, this, &TrashFileEventReceiver::onCleanTrashUrls, Qt::QueuedConnection);
    connect(qApp, &QCoreApplication::aboutToQuit, this, [=]() {
        stoped = true;
        if (trashIterator)
            trashIterator->cancel();

        future.waitForFinished();
    });
}

JobHandlePointer TrashFileEventReceiver::doMoveToTrash(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                       DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    Q_UNUSED(windowId);

    if (sources.isEmpty())
        return nullptr;

    if (SystemPathUtil::instance()->checkContainsSystemPath(sources)) {
        DialogManagerInstance->showDeleteSystemPathWarnDialog(windowId);
        return nullptr;
    }

    if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_MoveToTrash", windowId, sources, flags)) {
        return nullptr;
    }

    const QUrl &sourceFirst = sources.first();
    JobHandlePointer handle = nullptr;
    if (!FileUtils::fileCanTrash(sourceFirst) || !dfmio::DFMUtils::supportTrash(sourceFirst)) {
        if (DialogManagerInstance->showDeleteFilesDialog(sources, true) != QDialog::Accepted)
            return nullptr;
        handle = copyMoveJob->deletes(sources, flags);
    } else {
        // check url permission
        QList<QUrl> urlsCanTrash = sources;
        if (!flags.testFlag(AbstractJobHandler::JobFlag::kRevocation) && Application::instance()->genericAttribute(Application::kShowDeleteConfirmDialog).toBool()) {
            if (DialogManagerInstance->showNormalDeleteConfirmDialog(urlsCanTrash) != QDialog::Accepted)
                return nullptr;
        }
        handle = copyMoveJob->moveToTrash(urlsCanTrash, flags);
    }
    if (handleCallback)
        handleCallback(handle);
    return handle;
}

JobHandlePointer TrashFileEventReceiver::doRestoreFromTrash(const quint64 windowId, const QList<QUrl> sources, const QUrl target,
                                                            const AbstractJobHandler::JobFlags flags, AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    Q_UNUSED(windowId)

    if (sources.isEmpty())
        return nullptr;

    JobHandlePointer handle = copyMoveJob->restoreFromTrash(sources, target, flags);
    if (handleCallback)
        handleCallback(handle);
    return handle;
}

JobHandlePointer TrashFileEventReceiver::doCopyFromTrash(const quint64 windowId, const QList<QUrl> sources, const QUrl target,
                                                         const AbstractJobHandler::JobFlags flags, AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    Q_UNUSED(windowId)

    if (sources.isEmpty())
        return nullptr;

    JobHandlePointer handle = copyMoveJob->copyFromTrash(sources, target, flags);
    if (handleCallback)
        handleCallback(handle);
    return handle;
}

JobHandlePointer TrashFileEventReceiver::doCleanTrash(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                                      AbstractJobHandler::OperatorHandleCallback handleCallback, const bool showDelet)
{
    Q_UNUSED(windowId)
    Q_UNUSED(deleteNoticeType)

    if (stoped)
        return nullptr;

    if (!sources.isEmpty() && showDelet) {
        // Show delete files dialog
        if (DialogManagerInstance->showDeleteFilesDialog(sources) != QDialog::Accepted)
            return nullptr;
    }

    if (sources.isEmpty()) {
        future = QtConcurrent::run([windowId, deleteNoticeType, handleCallback]() {
            if (handleCallback)
                return instance()->countTrashFile(windowId, deleteNoticeType, handleCallback);
            return instance()->countTrashFile(windowId, deleteNoticeType, nullptr);
        });
        return nullptr;
    }

    if (!showDelet) {
        if (DialogManagerInstance->showClearTrashDialog(static_cast<quint64>(sources.length())) != QDialog::Accepted) return nullptr;
    }

    DWIDGET_USE_NAMESPACE
    DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_EmptyTrash);

    QList<QUrl> urls = std::move(sources);
    if (urls.isEmpty())
        urls.push_back(FileUtils::trashRootUrl());

    JobHandlePointer handle = copyMoveJob->cleanTrash(urls);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kCleanTrashType, handle);
    if (handleCallback)
        handleCallback(handle);
    return handle;
}

void TrashFileEventReceiver::countTrashFile(const quint64 windowId, const DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                            AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    if (stoped)
        return;
    DFMIO::DEnumerator enumerator(FileUtils::trashRootUrl());
    QList<QUrl> allFilesList;
    while (enumerator.hasNext()) {
        if (stoped)
            return;
        auto url = FileUtils::bindUrlTransform(enumerator.next());
        if (!allFilesList.contains(url))
            allFilesList.append(url);
    }

    if (stoped)
        return;

    if (allFilesList.isEmpty())
        return;

    emit cleanTrashUrls(windowId, allFilesList, deleteNoticeType, handleCallback);
}

TrashFileEventReceiver *TrashFileEventReceiver::instance()
{
    static TrashFileEventReceiver receiver;
    return &receiver;
}

void TrashFileEventReceiver::handleOperationMoveToTrash(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                        DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    auto handle = doMoveToTrash(windowId, sources, flags, handleCallback);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kMoveToTrashType, handle);
}

void TrashFileEventReceiver::handleOperationRestoreFromTrash(const quint64 windowId, const QList<QUrl> sources, const QUrl target,
                                                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                             DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    auto handle = doRestoreFromTrash(windowId, sources, target, flags, handleCallback);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kRestoreType, handle);
}

void TrashFileEventReceiver::handleOperationCleanTrash(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                                       DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    doCleanTrash(windowId, sources, deleteNoticeType, handleCallback);
}

void TrashFileEventReceiver::handleOperationMoveToTrash(const quint64 windowId,
                                                        const QList<QUrl> sources,
                                                        const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                        DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                        const QVariant custom,
                                                        DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback)
{

    JobHandlePointer handle = doMoveToTrash(windowId, sources, flags, handleCallback);
    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
    }
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kMoveToTrashType, handle);
}

void TrashFileEventReceiver::handleOperationRestoreFromTrash(const quint64 windowId,
                                                             const QList<QUrl> sources, const QUrl target,
                                                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                             DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                             const QVariant custom,
                                                             DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback)
{
    JobHandlePointer handle = doRestoreFromTrash(windowId, sources, target, flags, handleCallback);
    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
    }
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kRestoreType, handle);
}

void TrashFileEventReceiver::handleOperationCleanTrash(const quint64 windowId, const QList<QUrl> sources,
                                                       DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                       const QVariant custom, AbstractJobHandler::OperatorCallback callback)
{
    JobHandlePointer handle = doCleanTrash(windowId, sources, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, handleCallback);
    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
    }
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kCleanTrashType, handle);
}

void TrashFileEventReceiver::handleOperationCopyFromTrash(const quint64 windowId, const QList<QUrl> sources, const QUrl target,
                                                          const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                          DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    auto handle = doCopyFromTrash(windowId, sources, target, flags, handleCallback);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kRestoreType, handle);
}

void TrashFileEventReceiver::handleOperationCopyFromTrash(const quint64 windowId,
                                                          const QList<QUrl> sources, const QUrl target,
                                                          const AbstractJobHandler::JobFlags flags,
                                                          AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                          const QVariant custom,
                                                          AbstractJobHandler::OperatorCallback callback)
{
    JobHandlePointer handle = doCopyFromTrash(windowId, sources, target, flags, handleCallback);
    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
    }
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kRestoreType, handle);
}

JobHandlePointer TrashFileEventReceiver::onCleanTrashUrls(const quint64 windowId, const QList<QUrl> sources,
                                                          const AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                                          AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    if (stoped)
        return nullptr;
    return doCleanTrash(windowId, sources, deleteNoticeType, handleCallback, false);
}
