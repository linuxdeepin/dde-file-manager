// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <dtkwidget_global.h>
#include <dtkgui_config.h>

#include <QtConcurrent>
#include <QCoreApplication>

Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(bool *)
Q_DECLARE_METATYPE(QString *)

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

TrashFileEventReceiver::TrashFileEventReceiver(QObject *parent)
    : QObject(parent)
{
    copyMoveJob.reset(new FileCopyMoveJob);
    connect(this, &TrashFileEventReceiver::cleanTrashUrls, this, &TrashFileEventReceiver::onCleanTrashUrls, Qt::QueuedConnection);
    connect(qApp, &QCoreApplication::aboutToQuit, this, [=]() {
        stoped = true;
        if (trashIterator) {
            fmInfo() << "Cancelling trash iterator on application quit";
            trashIterator->cancel();
        }

        future.waitForFinished();
        fmInfo() << "TrashFileEventReceiver cleanup completed";
    });

    fmInfo() << "TrashFileEventReceiver initialized successfully";
}

JobHandlePointer TrashFileEventReceiver::doMoveToTrash(const quint64 windowId, const QList<QUrl> &sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                       DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback, const bool isInit)
{
    Q_UNUSED(windowId);

    // 源文件是空
    if (sources.isEmpty()) {
        fmWarning() << "Move to trash operation aborted: source list is empty";
        return nullptr;
    }

    fmInfo() << "Processing move to trash operation for" << sources.count() << "items";

    // 其他插件处理了相应的操作，直接返回
    if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_MoveToTrash", windowId, sources, flags)) {
        fmInfo() << "Move to trash operation handled by other plugin";
        return nullptr;
    }

    if (SystemPathUtil::instance()->checkContainsSystemPath(sources)) {
        fmWarning() << "Move to trash operation blocked: contains system path";
        DialogManagerInstance->showDeleteSystemPathWarnDialog(windowId);
        return nullptr;
    }

    // gio can only handle canonical file paths
    const QUrl &sourceFirst = sources.first();
    JobHandlePointer handle = nullptr;
    bool nullDirDelete = false;
    if (sources.count() == 1) {
        auto info = InfoFactory::create<FileInfo>(sourceFirst);
        nullDirDelete = info && info->isAttributes(OptInfoType::kIsDir)
                && !info->isAttributes(OptInfoType::kIsSymLink)
                && !info->isAttributes(OptInfoType::kIsWritable);
    }

    if (nullDirDelete || !FileUtils::fileCanTrash(sourceFirst)) {
        fmInfo() << "File cannot be moved to trash, will perform direct delete operation:"
                 << "nullDirDelete=" << nullDirDelete
                 << "canTrash=" << FileUtils::fileCanTrash(sourceFirst);

        if (DialogManagerInstance->showDeleteFilesDialog(sources, true) != QDialog::Accepted) {
            fmInfo() << "Delete operation cancelled by user";
            return nullptr;
        }
        handle = copyMoveJob->deletes(sources, flags, isInit);
        if (!isInit)
            return handle;
    } else {
        fmInfo() << "Processing normal move to trash operation";

        // check url permission
        QList<QUrl> urlsCanTrash = sources;
        if (!flags.testFlag(AbstractJobHandler::JobFlag::kRevocation) && Application::instance()->genericAttribute(Application::kShowDeleteConfirmDialog).toBool()) {
            if (DialogManagerInstance->showNormalDeleteConfirmDialog(urlsCanTrash) != QDialog::Accepted) {
                fmInfo() << "Move to trash operation cancelled by user";
                return nullptr;
            }
        }
        handle = copyMoveJob->moveToTrash(urlsCanTrash, flags, isInit);
        if (!isInit)
            return handle;
    }
    if (handleCallback)
        handleCallback(handle);

    fmInfo() << "Move to trash operation completed successfully";
    return handle;
}

JobHandlePointer TrashFileEventReceiver::doRestoreFromTrash(const quint64 windowId, const QList<QUrl> &sources, const QUrl &target,
                                                            const AbstractJobHandler::JobFlags flags, AbstractJobHandler::OperatorHandleCallback handleCallback, const bool isInit)
{
    Q_UNUSED(windowId)

    if (sources.isEmpty()) {
        fmWarning() << "Restore from trash operation aborted: source list is empty";
        return nullptr;
    }

    fmInfo() << "Processing restore from trash operation for" << sources.count() << "items to target:" << target;

    JobHandlePointer handle = copyMoveJob->restoreFromTrash(sources, target, flags);
    if (!isInit)
        return handle;
    if (handleCallback)
        handleCallback(handle);

    fmInfo() << "Restore from trash operation completed successfully";
    return handle;
}

JobHandlePointer TrashFileEventReceiver::doCopyFromTrash(const quint64 windowId, const QList<QUrl> &sources, const QUrl &target,
                                                         const AbstractJobHandler::JobFlags flags,
                                                         AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    Q_UNUSED(windowId)

    if (sources.isEmpty()) {
        fmWarning() << "Copy from trash operation aborted: source list is empty";
        return nullptr;
    }

    fmInfo() << "Processing copy from trash operation for" << sources.count() << "items to target:" << target;

    JobHandlePointer handle = copyMoveJob->copyFromTrash(sources, target, flags);
    if (handleCallback)
        handleCallback(handle);

    fmInfo() << "Copy from trash operation completed successfully";
    return handle;
}

JobHandlePointer TrashFileEventReceiver::doCleanTrash(const quint64 windowId, const QList<QUrl> &sources, const AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                                      AbstractJobHandler::OperatorHandleCallback handleCallback, const bool showDelet)
{
    Q_UNUSED(windowId)
    Q_UNUSED(deleteNoticeType)

    if (stoped) {
        fmWarning() << "Clean trash operation aborted: receiver is stopping";
        return nullptr;
    }

    fmInfo() << "Processing clean trash operation for" << sources.count() << "items, showDialog:" << showDelet;

    if (!sources.isEmpty() && showDelet) {
        // Show delete files dialog
        if (DialogManagerInstance->showDeleteFilesDialog(sources) != QDialog::Accepted) {
            fmInfo() << "Clean trash operation cancelled by user";
            return nullptr;
        }
    }

    if (sources.isEmpty()) {
        fmInfo() << "Starting async trash count operation";
        future = QtConcurrent::run([windowId, deleteNoticeType, handleCallback]() {
            if (handleCallback)
                return instance()->countTrashFile(windowId, deleteNoticeType, handleCallback);
            return instance()->countTrashFile(windowId, deleteNoticeType, nullptr);
        });
        return nullptr;
    }

    if (!showDelet) {
        if (DialogManagerInstance->showClearTrashDialog(static_cast<quint64>(sources.length())) != QDialog::Accepted) {
            fmInfo() << "Clear trash operation cancelled by user";
            return nullptr;
        }
    }

    fmInfo() << "Playing trash empty sound effect";
    DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_EmptyTrash);

    QList<QUrl> urls = std::move(sources);
    if (urls.isEmpty())
        urls.push_back(FileUtils::trashRootUrl());

    JobHandlePointer handle = copyMoveJob->cleanTrash(urls);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kCleanTrashType, handle);
    if (handleCallback)
        handleCallback(handle);

    fmInfo() << "Clean trash operation completed successfully";
    return handle;
}

void TrashFileEventReceiver::countTrashFile(const quint64 windowId, const DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                            AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    if (stoped) {
        fmWarning() << "Count trash file operation aborted: receiver is stopping";
        return;
    }

    fmInfo() << "Starting trash file enumeration";
    DFMIO::DEnumerator enumerator(FileUtils::trashRootUrl());
    QList<QUrl> allFilesList;
    int processedCount = 0;

    while (enumerator.hasNext()) {
        if (stoped) {
            fmWarning() << "Count trash file operation interrupted by stop signal";
            return;
        }
        auto url = FileUtils::bindUrlTransform(enumerator.next());
        if (!allFilesList.contains(url)) {
            allFilesList.append(url);
            processedCount++;
        }
    }

    if (stoped) {
        fmWarning() << "Count trash file operation interrupted after processing" << processedCount << "files";
        return;
    }

    if (allFilesList.isEmpty()) {
        fmInfo() << "Trash is empty, no files to process";
        return;
    }

    fmInfo() << "Trash enumeration completed, found" << allFilesList.count() << "files";
    emit cleanTrashUrls(windowId, allFilesList, deleteNoticeType, handleCallback);
}

TrashFileEventReceiver *TrashFileEventReceiver::instance()
{
    static TrashFileEventReceiver receiver;
    return &receiver;
}

void TrashFileEventReceiver::handleOperationMoveToTrash(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlag flags,
                                                        DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    fmInfo() << "Handling move to trash operation, window ID:" << windowId << "items count:" << sources.count();
    auto handle = doMoveToTrash(windowId, sources, flags, handleCallback);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kMoveToTrashType, handle);
}

void TrashFileEventReceiver::handleOperationRestoreFromTrash(const quint64 windowId, const QList<QUrl> sources, const QUrl target,
                                                             const AbstractJobHandler::JobFlag flags,
                                                             DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    fmInfo() << "Handling restore from trash operation, window ID:" << windowId << "items count:" << sources.count();
    auto handle = doRestoreFromTrash(windowId, sources, target, flags, handleCallback);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kRestoreType, handle);
}

void TrashFileEventReceiver::handleOperationCleanTrash(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                                       DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    fmInfo() << "Handling clean trash operation, window ID:" << windowId << "items count:" << sources.count();
    doCleanTrash(windowId, sources, deleteNoticeType, handleCallback);
}

void TrashFileEventReceiver::handleOperationMoveToTrash(const quint64 windowId,
                                                        const QList<QUrl> sources,
                                                        const AbstractJobHandler::JobFlag flags,
                                                        DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                        const QVariant custom,
                                                        DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback)
{
    fmInfo() << "Handling move to trash operation with callback, window ID:" << windowId << "items count:" << sources.count();

    JobHandlePointer handle = doMoveToTrash(windowId, sources, flags, handleCallback);
    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
        fmInfo() << "Custom callback executed for move to trash operation";
    }
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kMoveToTrashType, handle);
}

void TrashFileEventReceiver::handleOperationRestoreFromTrash(const quint64 windowId,
                                                             const QList<QUrl> sources, const QUrl target,
                                                             const AbstractJobHandler::JobFlag flags,
                                                             DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                             const QVariant custom,
                                                             DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback)
{
    fmInfo() << "Handling restore from trash operation with callback, window ID:" << windowId << "items count:" << sources.count();

    JobHandlePointer handle = doRestoreFromTrash(windowId, sources, target, flags, handleCallback);
    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
        fmInfo() << "Custom callback executed for restore from trash operation";
    }
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kRestoreType, handle);
}

void TrashFileEventReceiver::handleOperationCleanTrash(const quint64 windowId, const QList<QUrl> sources,
                                                       DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                       const QVariant custom, AbstractJobHandler::OperatorCallback callback)
{
    fmInfo() << "Handling clean trash operation with callback, window ID:" << windowId << "items count:" << sources.count();

    JobHandlePointer handle = doCleanTrash(windowId, sources, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, handleCallback);
    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
        fmInfo() << "Custom callback executed for clean trash operation";
    }
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kCleanTrashType, handle);
}

void TrashFileEventReceiver::handleOperationCopyFromTrash(const quint64 windowId, const QList<QUrl> &sources, const QUrl &target,
                                                          const AbstractJobHandler::JobFlag flags,
                                                          DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    fmInfo() << "Handling copy from trash operation, window ID:" << windowId << "items count:" << sources.count();
    auto handle = doCopyFromTrash(windowId, sources, target, flags, handleCallback);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kRestoreType, handle);
}

void TrashFileEventReceiver::handleOperationCopyFromTrash(const quint64 windowId,
                                                          const QList<QUrl> &sources, const QUrl &target,
                                                          const AbstractJobHandler::JobFlag flags,
                                                          AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                          const QVariant custom,
                                                          AbstractJobHandler::OperatorCallback callback)
{
    fmInfo() << "Handling copy from trash operation with callback, window ID:" << windowId << "items count:" << sources.count();

    JobHandlePointer handle = doCopyFromTrash(windowId, sources, target, flags, handleCallback);
    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
        fmInfo() << "Custom callback executed for copy from trash operation";
    }
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kRestoreType, handle);
}

void TrashFileEventReceiver::handleSaveRedoOpt(const QString &token, const bool moreThanZero)
{
    fmInfo() << "Processing save redo operation for token:" << token << "moreThanZero:" << moreThanZero;

    QVariantMap ret;
    {
        QMutexLocker lk(&undoLock);
        if (!undoOpts.contains(token)) {
            fmWarning() << "Token not found in undo operations:" << token;
            return;
        }
        ret = undoOpts.take(token);
    }
    if (ret.isEmpty()) {
        fmWarning() << "Empty undo operation data for token:" << token;
        return;
    }

    GlobalEventType undoEventType = static_cast<GlobalEventType>(ret.value("undoevent").value<uint16_t>());
    QList<QUrl> undoSources = QUrl::fromStringList(ret.value("undosources").toStringList());
    QList<QUrl> undoTargets = QUrl::fromStringList(ret.value("undotargets").toStringList());
    GlobalEventType redoEventType = static_cast<GlobalEventType>(ret.value("redoevent").value<uint16_t>());
    QList<QUrl> redoSources = QUrl::fromStringList(ret.value("redosources").toStringList());
    QList<QUrl> redoTargets = QUrl::fromStringList(ret.value("redotargets").toStringList());

    if (redoEventType == GlobalEventType::kTouchFile && moreThanZero) {
        fmInfo() << "Skipping touch file redo operation due to size constraint";
        return;
    }

    fmInfo() << "Saving redo operation: undoType=" << static_cast<int>(undoEventType)
             << "redoType=" << static_cast<int>(redoEventType)
             << "undoSources=" << undoSources.count()
             << "redoSources=" << redoSources.count();

    // save operation by dbus
    QVariantMap values;
    values.insert("undoevent", QVariant::fromValue(static_cast<uint16_t>(redoEventType)));
    values.insert("undosources", QUrl::toStringList(redoSources));
    values.insert("undotargets", QUrl::toStringList(redoTargets));
    values.insert("redoevent", QVariant::fromValue(static_cast<uint16_t>(undoEventType)));
    values.insert("redosources", QUrl::toStringList(undoSources));
    values.insert("redotargets", QUrl::toStringList(undoTargets));
    dpfSignalDispatcher->publish(GlobalEventType::kSaveRedoOperator, values);

    fmInfo() << "Redo operation saved successfully";
}

void TrashFileEventReceiver::handleOperationUndoMoveToTrash(const quint64 windowId,
                                                            const QList<QUrl> &sources,
                                                            const AbstractJobHandler::JobFlag flags,
                                                            AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                            const QVariantMap &op)
{
    fmInfo() << "Processing undo move to trash operation, window ID:" << windowId << "items count:" << sources.count();

    auto handle = doMoveToTrash(windowId, sources, flags, handleCallback, false);

    if (!handle) {
        fmWarning() << "Failed to create job handle for undo move to trash operation";
        return;
    }

    connect(handle.get(), &AbstractJobHandler::requestSaveRedoOperation, this,
            &TrashFileEventReceiver::handleSaveRedoOpt);
    {
        QMutexLocker lk(&undoLock);
        QString token = QString::number(quintptr(handle.get()), 16);
        undoOpts.insert(token, op);
        fmInfo() << "Stored undo operation data with token:" << token;
    }
    copyMoveJob->initArguments(handle);
    if (handleCallback)
        handleCallback(handle);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kMoveToTrashType, handle);

    fmInfo() << "Undo move to trash operation setup completed";
}

void TrashFileEventReceiver::handleOperationUndoRestoreFromTrash(const quint64 windowId, const QList<QUrl> &sources, const QUrl &target, const AbstractJobHandler::JobFlag flags, AbstractJobHandler::OperatorHandleCallback handleCallback, const QVariantMap &op)
{
    fmInfo() << "Processing undo restore from trash operation, window ID:" << windowId << "items count:" << sources.count();

    auto handle = doRestoreFromTrash(windowId, sources, target, flags, handleCallback, false);
    if (!handle) {
        fmWarning() << "Failed to create job handle for undo restore from trash operation";
        return;
    }

    connect(handle.get(), &AbstractJobHandler::requestSaveRedoOperation, this,
            &TrashFileEventReceiver::handleSaveRedoOpt);
    {
        QMutexLocker lk(&undoLock);
        QString token = QString::number(quintptr(handle.get()), 16);
        undoOpts.insert(token, op);
        fmInfo() << "Stored undo restore operation data with token:" << token;
    }
    copyMoveJob->initArguments(handle);
    if (handleCallback)
        handleCallback(handle);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kRestoreType, handle);

    fmInfo() << "Undo restore from trash operation setup completed";
}

JobHandlePointer TrashFileEventReceiver::onCleanTrashUrls(const quint64 windowId, const QList<QUrl> &sources,
                                                          const AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                                          AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    if (stoped) {
        fmWarning() << "Clean trash URLs operation aborted: receiver is stopping";
        return nullptr;
    }

    fmInfo() << "Executing clean trash URLs operation for" << sources.count() << "items";
    return doCleanTrash(windowId, sources, deleteNoticeType, handleCallback, false);
}
