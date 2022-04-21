/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
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
#include "fileoperationseventreceiver.h"

#include "fileoperations/filecopymovejob.h"
#include "dfm-base/utils/hidefilehelper.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/file/local/localfilehandler.h"
#include "dfm-base/utils/windowutils.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/base/schemefactory.h"

#include <QDebug>

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

FileOperationsEventReceiver::FileOperationsEventReceiver(QObject *parent)
    : QObject(parent), dialogManager(DialogManagerInstance)
{
    getServiceMutex.reset(new QMutex);
    functionsMutex.reset(new QMutex);
    copyMoveJob.reset(new FileCopyMoveJob);
    initDBus();
    initService();
}

bool FileOperationsEventReceiver::initService()
{
    QMutexLocker lk(getServiceMutex.data());
    if (operationsService.isNull()) {
        auto &ctx = DPF_NAMESPACE::Framework::instance().serviceContext();
        operationsService = ctx.service<DSC_NAMESPACE::FileOperationsService>(DSC_NAMESPACE::FileOperationsService::name());
        if (!operationsService) {
            QString errStr;
            if (!ctx.load(DSC_NAMESPACE::FileOperationsService::name(), &errStr)) {
                qCritical() << errStr;
                abort();
            }
            operationsService = ctx.service<DSC_NAMESPACE::FileOperationsService>(DSC_NAMESPACE::FileOperationsService::name());
        }
    }

    return operationsService && dialogManager;
}

QString FileOperationsEventReceiver::newDocmentName(QString targetdir,
                                                    const QString suffix,
                                                    const CreateFileType fileType)
{
    QString suffixex;
    QString baseName;
    switch (fileType) {
    case CreateFileType::kCreateFileTypeFolder:
        baseName = QObject::tr("New Folder");
        break;
    case CreateFileType::kCreateFileTypeText:
        baseName = QObject::tr("Text");
        suffixex = "txt";
        break;
    case CreateFileType::kCreateFileTypeWord:
        baseName = QObject::tr("Document");
        suffixex = DFMBASE_NAMESPACE::WindowUtils::isWayLand() ? "wps" : "doc";
        break;
    case CreateFileType::kCreateFileTypeExcel:
        baseName = QObject::tr("Spreadsheet");
        suffixex = DFMBASE_NAMESPACE::WindowUtils::isWayLand() ? "et" : "xls";
        break;
    case CreateFileType::kCreateFileTypePowerpoint:
        baseName = QObject::tr("Presentation");
        suffixex = DFMBASE_NAMESPACE::WindowUtils::isWayLand() ? "dps" : "ppt";
        break;
    case CreateFileType::kCreateFileTypeDefault:
        baseName = QObject::tr("New File");
        suffixex = suffix;
        break;
    default:
        return QString();
    }
    return newDocmentName(targetdir, baseName, suffixex);
}

QString FileOperationsEventReceiver::newDocmentName(QString targetdir,
                                                    const QString &baseName,
                                                    const QString &suffix)
{
    if (targetdir.isEmpty())
        return QString();

    if (targetdir.endsWith(QDir::separator()))
        targetdir.chop(1);

    int i = 0;
    QString filePath = suffix.isEmpty() ? QString("%1/%2").arg(targetdir, baseName) : QString("%1/%2.%3").arg(targetdir, baseName, suffix);
    while (true) {
        if (QFile(filePath).exists()) {
            ++i;
            filePath = suffix.isEmpty()
                    ? QString("%1/%2 %3").arg(targetdir, baseName, QString::number(i))
                    : QString("%1/%2 %3.%4").arg(targetdir, baseName, QString::number(i), suffix);
        } else {
            return filePath;
        }
    }
}

void FileOperationsEventReceiver::initDBus()
{
    qInfo() << "Start initilize dbus: `OperationsStackManagerInterface`";
    // Note: the plugin depends on `dde-file-manager-server`!
    // the plugin will not work if `dde-file-manager-server` not run.
    static const QString OperationsStackService = "com.deepin.filemanager.service";
    static const QString OperationsStackPath = "/com/deepin/filemanager/service/OperationsStackManager";

    operationsStackDbus.reset(new OperationsStackManagerInterface(OperationsStackService,
                                                                  OperationsStackPath,
                                                                  QDBusConnection::sessionBus(),
                                                                  this));
    qInfo() << "Finish initilize dbus: `OperationsStackManagerInterface`";
}

bool FileOperationsEventReceiver::revocation(const quint64 windowId, const QVariantMap &ret,
                                             DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle)
{
    if (!ret.contains("event") || !ret.contains("sources") || !ret.contains("target"))
        return false;
    GlobalEventType eventType = static_cast<GlobalEventType>(ret.value("event").value<uint16_t>());
    QList<QUrl> sources;
    QStringList listUrls = ret.value("sources").value<QStringList>();
    for (const auto &url : listUrls)
        sources.append(QUrl(url));

    QUrl target = QUrl(ret.value("target").value<QString>());
    switch (eventType) {
    case kCutFile:
        handleOperationCut(windowId, sources, target, AbstractJobHandler::JobFlag::kRevocation, handle);
        break;
    case kDeleteFiles:
        handleOperationDeletes(windowId, sources, AbstractJobHandler::JobFlag::kRevocation, handle);
        break;
    case kMoveToTrash:
        handleOperationMoveToTrash(windowId, sources, AbstractJobHandler::JobFlag::kRevocation, handle);
        break;
    case kRestoreFromTrash:
        handleOperationRestoreFromTrash(windowId, sources, AbstractJobHandler::JobFlag::kRevocation, handle);
        break;
    default:
        return false;
    }

    return true;
}

bool FileOperationsEventReceiver::doRenameFiles(const QList<QUrl> urls, const QPair<QString, QString> pair,
                                                const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> pair2,
                                                const RenameTypes type, QMap<QUrl, QUrl> &successUrls, QString &errorMsg)
{
    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    bool ok = false;
    switch (type) {
    case RenameTypes::kBatchRepalce:
        ok = fileHandler.renameFileBatchReplace(urls, pair, successUrls);
        break;
    case RenameTypes::kBatchCustom:
        ok = fileHandler.renameFileBatchCustom(urls, pair, successUrls);
        break;
    case RenameTypes::kBatchAppend:
        ok = fileHandler.renameFileBatchAppend(urls, pair2, successUrls);
        break;
    }
    if (!ok) {
        errorMsg = fileHandler.errorString();
        DialogManagerInstance->showErrorDialog("Rename file error: %1", errorMsg);
    }
    return ok;
}

FileOperationsEventReceiver *FileOperationsEventReceiver::instance()
{
    static FileOperationsEventReceiver receiver;
    return &receiver;
}

void FileOperationsEventReceiver::connectService()
{
    dpfInstance.eventUnicast().connect("dfm_service_common::FileOperationsService::registerOperations", this, &FileOperationsEventReceiver::invokeRegister);
    dpfInstance.eventUnicast().connect("dfm_service_common::FileOperationsService::unregisterOperations", this, &FileOperationsEventReceiver::invokeUnregister);
}

void FileOperationsEventReceiver::handleOperationCopy(const quint64 windowId,
                                                      const QList<QUrl> sources,
                                                      const QUrl target,
                                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                      DFMGLOBAL_NAMESPACE::OperatorHandleCallback callbaskHandle)
{
    Q_UNUSED(windowId);
    if (!sources.isEmpty() && !target.isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(target.scheme());
        }
        if (function && function->copy) {
            JobHandlePointer handle = function->copy(windowId, sources, target, flags);
            if (callbaskHandle)
                callbaskHandle(handle);
            return;
        }
    }
    JobHandlePointer handle = copyMoveJob->copy(sources, target, flags);
    if (callbaskHandle)
        callbaskHandle(handle);
}

void FileOperationsEventReceiver::handleOperationCut(quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                     DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback)
{
    Q_UNUSED(windowId);
    if (!sources.isEmpty() && !target.isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(target.scheme());
        }
        if (function && function->cut) {
            JobHandlePointer handle = function->cut(windowId, sources, target, flags);
            if (handleCallback)
                handleCallback(handle);
            return;
        }
    }
    JobHandlePointer handle = copyMoveJob->cut(sources, target, flags);
    if (handleCallback)
        handleCallback(handle);
}

void FileOperationsEventReceiver::handleOperationMoveToTrash(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                             DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback)
{
    Q_UNUSED(windowId);

    if (!sources.isEmpty() && !sources.first().isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(sources.first().scheme());
        }
        if (function && function->moveToTash) {
            JobHandlePointer handle = function->moveToTash(windowId, sources, flags);
            if (handleCallback)
                handleCallback(handle);
            return;
        }
    }

    JobHandlePointer handle = copyMoveJob->moveToTrash(sources, flags);
    if (handleCallback)
        handleCallback(handle);
}

void FileOperationsEventReceiver::handleOperationRestoreFromTrash(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                                  DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback)
{
    Q_UNUSED(windowId);
    if (!sources.isEmpty() && !sources.first().isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(sources.first().scheme());
        }
        if (function && function->restoreFromTrash) {
            JobHandlePointer handle = function->restoreFromTrash(windowId, sources, flags);
            if (handleCallback)
                handleCallback(handle);
            return;
        }
    }
    JobHandlePointer handle = copyMoveJob->restoreFromTrash(sources, flags);
    if (handleCallback)
        handleCallback(handle);
}

void FileOperationsEventReceiver::handleOperationDeletes(const quint64 windowId,
                                                         const QList<QUrl> sources,
                                                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                         DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback)
{
    Q_UNUSED(windowId);
    //Delete local file with shift+delete, show a confirm dialog.
    if (DialogManagerInstance->showDeleteFilesClearTrashDialog(sources) != QDialog::Accepted)
        return;

    if (!sources.isEmpty() && !sources.first().isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(sources.first().scheme());
        }
        if (function && function->deletes) {
            JobHandlePointer handle = function->deletes(windowId, sources, flags);
            if (handleCallback)
                handleCallback(handle);
            return;
        }
    }
    JobHandlePointer handle = copyMoveJob->deletes(sources, flags);
    if (handleCallback)
        handleCallback(handle);
}

void FileOperationsEventReceiver::handleOperationCleanTrash(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                                            DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback)
{
    //清空回收站操作弹框提示（这里只会显示Emtpy按钮）
    bool isFileAlreadyInTrash = (deleteNoticeType == AbstractJobHandler::DeleteDialogNoticeType::kDeleteTashFiles);   //检查用户是否从回收站内部删除文件
    //在此处理从回收站内部选择文件的删除操作(若不在这里处理，会进入到handleOperationCleanTrash()中，导致弹框提示无法区分"Delete"和"Empty"按钮的显示)
    if (!sources.isEmpty()) {
        //Show clear trash dialog
        if (DialogManagerInstance->showDeleteFilesClearTrashDialog(sources, !isFileAlreadyInTrash) != QDialog::Accepted)
            return;
    }

    Q_UNUSED(windowId);
    if (!sources.isEmpty() && !sources.first().isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(sources.first().scheme());
        }
        if (function && function->cleanTrash) {
            JobHandlePointer handle = function->cleanTrash(windowId, sources);
            if (handleCallback)
                handleCallback(handle);
            return;
        }
    }
    JobHandlePointer handle = copyMoveJob->cleanTrash(sources);
    if (handleCallback)
        handleCallback(handle);
}

void FileOperationsEventReceiver::handleOperationCopy(const quint64 windowId,
                                                      const QList<QUrl> sources,
                                                      const QUrl target,
                                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                      DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback,
                                                      const QVariant custom,
                                                      DFMGLOBAL_NAMESPACE::OperatorCallback callback)
{
    JobHandlePointer handle = copyMoveJob->copy(sources, target, flags);
    if (handleCallback)
        handleCallback(handle);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

void FileOperationsEventReceiver::handleOperationCut(const quint64 windowId,
                                                     const QList<QUrl> sources,
                                                     const QUrl target,
                                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                     DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback,
                                                     const QVariant custom,
                                                     DFMBASE_NAMESPACE::Global::OperatorCallback callback)
{
    JobHandlePointer handle = copyMoveJob->cut(sources, target, flags);
    if (handleCallback)
        handleCallback(handle);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

void FileOperationsEventReceiver::handleOperationMoveToTrash(const quint64 windowId,
                                                             const QList<QUrl> sources,
                                                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                             DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback,
                                                             const QVariant custom,
                                                             DFMBASE_NAMESPACE::Global::OperatorCallback callback)
{
    JobHandlePointer handle = copyMoveJob->moveToTrash(sources, flags);
    if (handleCallback)
        handleCallback(handle);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

void FileOperationsEventReceiver::handleOperationRestoreFromTrash(const quint64 windowId,
                                                                  const QList<QUrl> sources,
                                                                  const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                                  DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback,
                                                                  const QVariant custom,
                                                                  DFMBASE_NAMESPACE::Global::OperatorCallback callback)
{
    JobHandlePointer handle = copyMoveJob->restoreFromTrash(sources, flags);
    if (handleCallback)
        handleCallback(handle);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

void FileOperationsEventReceiver::handleOperationDeletes(const quint64 windowId,
                                                         const QList<QUrl> sources,
                                                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                         DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback,
                                                         const QVariant custom,
                                                         DFMBASE_NAMESPACE::Global::OperatorCallback callback)
{
    JobHandlePointer handle = copyMoveJob->deletes(sources, flags);
    if (handleCallback)
        handleCallback(handle);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

void FileOperationsEventReceiver::handleOperationCleanTrash(const quint64 windowId, const QList<QUrl> sources,
                                                            DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback,
                                                            const QVariant custom, OperatorCallback callback)
{
    JobHandlePointer handle = copyMoveJob->deletes(sources);
    if (handleCallback)
        handleCallback(handle);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::handleOperationOpenFiles(const quint64 windowId,
                                                           const QList<QUrl> urls)
{
    Q_UNUSED(windowId);
    bool ok = false;
    QString error;
    if (!urls.isEmpty() && !urls.first().isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(urls.first().scheme());
        }
        if (function && function->openFiles) {
            ok = function->openFiles(windowId, urls, &error);
            if (!ok) {
                dialogManager->showErrorDialog("open file error", error);
            }
            // TODO:: file Open finished need to send file Open finished event
            dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFilesResult, windowId, urls, ok, error);
            return ok;
        }
    }
    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    ok = fileHandler.openFiles(urls);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showErrorDialog("open file error", error);
    }
    // TODO:: file Open finished need to send file Open finished event
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFilesResult, windowId, urls, ok, error);
    return ok;
}

void FileOperationsEventReceiver::handleOperationOpenFiles(const quint64 windowId,
                                                           const QList<QUrl> urls,
                                                           const QVariant custom,
                                                           DFMBASE_NAMESPACE::Global::OperatorCallback callback)
{
    bool ok = handleOperationOpenFiles(windowId, urls);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(urls));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

void FileOperationsEventReceiver::handleOperationOpenFilesByApp(const quint64 windowId,
                                                                const QList<QUrl> urls,
                                                                const QList<QString> apps,
                                                                const QVariant custom,
                                                                DFMBASE_NAMESPACE::Global::OperatorCallback callback)
{
    bool ok = handleOperationOpenFilesByApp(windowId, urls, apps);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(urls));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::handleOperationOpenFilesByApp(const quint64 windowId,
                                                                const QList<QUrl> urls,
                                                                const QList<QString> apps)
{
    Q_UNUSED(windowId);
    bool ok = false;
    QString error;
    if (!urls.isEmpty() && !urls.first().isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(urls.first().scheme());
        }
        if (function && function->openFilesByApp) {
            ok = function->openFilesByApp(windowId, urls, apps, &error);
            if (!ok) {
                dialogManager->showErrorDialog("open file by app error", error);
            }
            // TODO:: file openFilesByApp finished need to send file openFilesByApp finished event
            dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFilesByAppResult, windowId, urls, ok, error);
            return ok;
        }
    }
    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    QString app;
    if (apps.count() == 1) {
        app = apps.at(0);
    }
    ok = fileHandler.openFilesByApp(urls, app);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showErrorDialog("open file by app error", error);
    }
    // TODO:: file openFilesByApp finished need to send file openFilesByApp finished event
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFilesByAppResult, windowId, urls, ok, error);
    return ok;
}

bool FileOperationsEventReceiver::handleOperationRenameFile(const quint64 windowId,
                                                            const QUrl oldUrl,
                                                            const QUrl newUrl)
{
    Q_UNUSED(windowId);
    bool ok = false;
    QString error;
    if (!oldUrl.isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(oldUrl.scheme());
        }
        if (function && function->renameFile) {

            ok = function->renameFile(windowId, oldUrl, newUrl, &error);
            if (!ok) {
                dialogManager->showErrorDialog("rename file error", error);
            }
            // TODO:: file renameFile finished need to send file renameFile finished event
            dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                                  windowId, QList<QUrl>() << oldUrl << newUrl, ok, error);
            return ok;
        }
    }
    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    ok = fileHandler.renameFile(oldUrl, newUrl);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showErrorDialog("rename file error", error);
    }
    // TODO:: file renameFile finished need to send file renameFile finished event
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                          windowId, QList<QUrl>() << oldUrl << newUrl, ok, error);
    return ok;
}

void FileOperationsEventReceiver::handleOperationRenameFile(const quint64 windowId,
                                                            const QUrl oldUrl,
                                                            const QUrl newUrl, const QVariant custom,
                                                            OperatorCallback callback)
{
    bool ok = handleOperationRenameFile(windowId, oldUrl, newUrl);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << oldUrl));
        args->insert(CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << oldUrl));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::handleOperationRenameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, QString> pair, const bool replace)
{
    QMap<QUrl, QUrl> successUrls;
    QString error;
    RenameTypes type = RenameTypes::kBatchRepalce;
    if (!replace)
        type = RenameTypes::kBatchCustom;
    bool ok = doRenameFiles(urls, pair, {}, type, successUrls, error);

    // publish result
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                          windowId, urls, successUrls.values(), ok, error);
    return ok;
}

void FileOperationsEventReceiver::handleOperationRenameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, QString> pair, const bool replace, const QVariant custom, OperatorCallback callback)
{
    QMap<QUrl, QUrl> successUrls;
    QString error;
    RenameTypes type = RenameTypes::kBatchRepalce;
    if (!replace)
        type = RenameTypes::kBatchCustom;
    bool ok = doRenameFiles(urls, pair, {}, type, successUrls, error);
    // publish result
    const QList<QUrl> &lists = successUrls.values();
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                          windowId, urls, lists, ok, error);

    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << urls));
        args->insert(CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << lists));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::handleOperationRenameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> pair)
{
    QMap<QUrl, QUrl> successUrls;
    QString error;
    bool ok = doRenameFiles(urls, {}, pair, RenameTypes::kBatchAppend, successUrls, error);

    // publish result
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                          windowId, urls, successUrls.values(), ok, error);
    return ok;
}

void FileOperationsEventReceiver::handleOperationRenameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> pair, const QVariant custom, OperatorCallback callback)
{
    QMap<QUrl, QUrl> successUrls;
    QString error;
    bool ok = doRenameFiles(urls, {}, pair, RenameTypes::kBatchAppend, successUrls, error);
    const QList<QUrl> &lists = successUrls.values();
    // publish result
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                          windowId, urls, lists, ok, error);

    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << urls));
        args->insert(CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << lists));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

QString FileOperationsEventReceiver::handleOperationMkdir(const quint64 windowId,
                                                          const QUrl url,
                                                          const CreateFileType fileType)
{
    if (url.isLocalFile()) {
        QString newPath = newDocmentName(url.path(), QString(), fileType);
        if (newPath.isEmpty())
            return newPath;

        QUrl urlNew;
        urlNew.setScheme(url.scheme());
        urlNew.setPath(newPath);

        return handleOperationMkdir(windowId, urlNew) ? newPath : QString();
    } else {
        QString error;
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(url.scheme());
        }
        if (function && function->makeDir) {
            bool ok = false;
            ok = function->makeDir(windowId, url, &error, fileType);
            if (!ok) {
                dialogManager->showErrorDialog("make dir error", error);
            }
            // TODO:: make dir finished need to send make dir finished event
            dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kMkdirResult,
                                                  windowId, QList<QUrl>() << url, ok, error);
            return ok ? url.path() : QString();
        }
        return handleOperationMkdir(windowId, url) ? url.path() : QString();
    }
}

void FileOperationsEventReceiver::handleOperationMkdir(const quint64 windowId,
                                                       const QUrl url,
                                                       CreateFileType fileType,
                                                       const QVariant custom,
                                                       OperatorCallback callback)
{
    QString newPath = handleOperationMkdir(windowId, url, fileType);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
        args->insert(CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << QUrl::fromLocalFile(newPath)));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(!newPath.isEmpty()));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::handleOperationMkdir(const quint64 windowId, const QUrl url)
{
    Q_UNUSED(windowId);
    bool ok = false;
    QString error;
    if (!url.isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(url.scheme());
        }
        if (function && function->makeDir) {
            ok = function->makeDir(windowId, url, &error, DFMGLOBAL_NAMESPACE::CreateFileType::kCreateFileTypeUnknow);
            if (!ok) {
                dialogManager->showErrorDialog("make dir error", error);
            }
            // TODO:: make dir finished need to send make dir finished event
            dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kMkdirResult,
                                                  windowId, QList<QUrl>() << url, ok, error);
            return ok;
        }
    }
    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    ok = fileHandler.mkdir(url);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showErrorDialog("make dir error", error);
    }
    // TODO:: make dir finished need to send make dir finished event
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kMkdirResult,
                                          windowId, QList<QUrl>() << url, ok, error);
    return ok;
}

void FileOperationsEventReceiver::handleOperationMkdir(const quint64 windowId,
                                                       const QUrl url,
                                                       const QVariant custom,
                                                       OperatorCallback callback)
{
    bool ok = handleOperationMkdir(windowId, url);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::handleOperationTouchFile(const quint64 windowId,
                                                           const QUrl url)
{
    Q_UNUSED(windowId);
    bool ok = false;
    QString error;
    if (!url.isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(url.scheme());
        }
        if (function && function->touchFile) {
            ok = function->touchFile(windowId, url, &error, DFMGLOBAL_NAMESPACE::CreateFileType::kCreateFileTypeUnknow);
            if (!ok) {
                dialogManager->showErrorDialog("touch file error", error);
            }
            // TODO:: touch file finished need to send touch file finished event
            dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kTouchFileResult,
                                                  windowId, QList<QUrl>() << url, ok, error);
            return ok;
        }
    }
    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    ok = fileHandler.touchFile(url);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showErrorDialog("touch file error", error);
    }
    // TODO:: touch file finished need to send touch file finished event
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kTouchFileResult,
                                          windowId, QList<QUrl>() << url, ok, error);
    return ok;
}

void FileOperationsEventReceiver::handleOperationTouchFile(const quint64 windowId,
                                                           const QUrl url, const QVariant custom,
                                                           OperatorCallback callback)
{
    bool ok = handleOperationTouchFile(windowId, url);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

QString FileOperationsEventReceiver::handleOperationTouchFile(const quint64 windowId,
                                                              const QUrl url,
                                                              const CreateFileType fileType,
                                                              const QString suffix)
{
    Q_UNUSED(suffix)
    if (url.isLocalFile()) {
        QString newPath = newDocmentName(url.path(), QString(), fileType);
        if (newPath.isEmpty())
            return newPath;

        QUrl urlNew;
        urlNew.setScheme(url.scheme());
        urlNew.setPath(newPath);

        return handleOperationTouchFile(windowId, urlNew) ? newPath : QString();
    } else {
        QString error;
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(url.scheme());
        }
        if (function && function->touchFile) {
            bool ok = false;
            ok = function->touchFile(windowId, url, &error, fileType);
            if (!ok) {
                dialogManager->showErrorDialog("touch file error", error);
            }
            // TODO:: touch file finished need to send touch file finished event
            dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kTouchFileResult,
                                                  windowId, QList<QUrl>() << url, ok, error);
            return ok ? url.path() : QString();
        }
        return handleOperationTouchFile(windowId, url) ? url.path() : QString();
    }
}

void FileOperationsEventReceiver::handleOperationTouchFile(const quint64 windowId,
                                                           const QUrl url,
                                                           const CreateFileType fileType,
                                                           const QString suffix,
                                                           const QVariant custom,
                                                           OperatorCallback callback)
{
    QString newPath = handleOperationTouchFile(windowId, url, fileType, suffix);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
        args->insert(CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << QUrl::fromLocalFile(newPath)));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(!newPath.isEmpty()));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::handleOperationLinkFile(const quint64 windowId,
                                                          const QUrl url,
                                                          const QUrl link)
{
    bool ok = false;
    QString error;
    if (!url.isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(url.scheme());
        }
        if (function && function->linkFile) {
            ok = function->linkFile(windowId, url, link, &error);
            if (!ok) {
                dialogManager->showErrorDialog("link file error", error);
            }
            // TODO:: link file finished need to send link file finished event
            dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kCreateSymlinkResult,
                                                  windowId, QList<QUrl>() << url << link, ok, error);
            return ok;
        }
    }

    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    ok = fileHandler.createSystemLink(url, link);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showErrorDialog("link file error", error);
    }
    // TODO:: link file finished need to send link file finished event
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kCreateSymlinkResult,
                                          windowId, QList<QUrl>() << url << link, ok, error);
    return ok;
}

void FileOperationsEventReceiver::handleOperationLinkFile(const quint64 windowId,
                                                          const QUrl url,
                                                          const QUrl link, const QVariant custom,
                                                          DFMBASE_NAMESPACE::Global::OperatorCallback callback)
{
    bool ok = handleOperationLinkFile(windowId, url, link);

    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
        args->insert(CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << link));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::handleOperationSetPermission(const quint64 windowId,
                                                               const QUrl url,
                                                               const QFileDevice::Permissions permissions)
{
    QString error;
    bool ok = false;
    if (!url.isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(url.scheme());
        }
        if (function && function->setPermission) {
            ok = function->setPermission(windowId, url, permissions, &error);
            if (!ok) {
                dialogManager->showErrorDialog("set file permissions error", error);
            }
            // TODO:: set file permissions finished need to send set file permissions finished event
            dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kSetPermissionResult,
                                                  windowId, QList<QUrl>() << url, ok, error);
            return ok;
        }
    }
    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    ok = fileHandler.setPermissions(url, permissions);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showErrorDialog("set file permissions error", error);
    }
    // TODO:: set file permissions finished need to send set file permissions finished event
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kSetPermissionResult,
                                          windowId, QList<QUrl>() << url, ok, error);
    return ok;
}

void FileOperationsEventReceiver::handleOperationSetPermission(const quint64 windowId,
                                                               const QUrl url,
                                                               const QFileDevice::Permissions permissions, const QVariant custom,
                                                               DFMBASE_NAMESPACE::Global::OperatorCallback callback)
{
    bool ok = handleOperationSetPermission(windowId, url, permissions);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::handleOperationWriteToClipboard(const quint64 windowId,
                                                                  const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                                                                  const QList<QUrl> urls)
{
    QString error;
    if (!urls.isEmpty() && !urls.first().isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(urls.first().scheme());
        }
        if (function && function->writeUrlsToClipboard) {
            function->writeUrlsToClipboard(windowId, action, urls);
            return true;
        }
    }
    DFMBASE_NAMESPACE::ClipBoard::instance()->setUrlsToClipboard(urls, action);
    return true;
}

bool FileOperationsEventReceiver::handleOperationWriteDataToClipboard(const quint64 windowId, QMimeData *data)
{
    Q_UNUSED(windowId);
    if (!data) {
        qWarning() << " write to clipboard data is nullptr!!!!!!!";
        return false;
    }

    DFMBASE_NAMESPACE::ClipBoard::instance()->setDataToClipboard(data);
    return true;
}

bool FileOperationsEventReceiver::handleOperationOpenInTerminal(const quint64 windowId, const QList<QUrl> urls)
{
    QString error;
    bool ok = false;
    bool result = false;
    QSharedPointer<LocalFileHandler> fileHandler = nullptr;
    if (urls.count() > 0 && !urls.first().isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(urls.first().scheme());
        }
        if (function && function->openInTerminal) {
            ok = function->openInTerminal(windowId, urls, &error);
            if (!result)
                result = ok;
            if (!ok) {
                dialogManager->showErrorDialog("open file in terminal error", error);
            }
            // TODO:: open file in terminal finished need to send open file in terminal finished event
            dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenInTerminalResult,
                                                  windowId, urls, result, error);
            return ok;
        }
    }

    for (const auto &url : urls) {
        const QString &current_dir = QDir::currentPath();
        QDir::setCurrent(url.toLocalFile());
        if (!url.isLocalFile()) {
        }
        if (fileHandler.isNull())
            fileHandler.reset(new LocalFileHandler());
        ok = QProcess::startDetached(fileHandler->defaultTerminalPath());
        if (!result)
            result = ok;
        QDir::setCurrent(current_dir);
    }

    // TODO:: open file in terminal finished need to send open file in terminal finished event
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenInTerminalResult,
                                          windowId, urls, result, error);

    return ok;
}

bool FileOperationsEventReceiver::handleOperationSaveOperations(const QVariantMap values)
{
    if (operationsStackDbus) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = operationsStackDbus->SaveOperations(values);
        reply.waitForFinished();
        if (!reply.isValid()) {
            qCritical() << "D-Bus reply is invalid " << reply.error();
            return false;
        }
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
        return true;
    }
    return false;
}

bool FileOperationsEventReceiver::handleOperationCleanSaveOperationsStack()
{
    if (operationsStackDbus) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        operationsStackDbus->CleanOperations();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
        return true;
    }
    return false;
}

bool FileOperationsEventReceiver::handleOperationRevocation(const quint64 windowId,
                                                            DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle)
{
    QVariantMap ret;
    if (operationsStackDbus) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = operationsStackDbus->RevocationOperations();
        reply.waitForFinished();
        if (reply.isValid())
            ret = reply.value();
        else {
            qCritical() << "D-Bus reply is invalid " << reply.error();
            return false;
        }
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;

        return revocation(windowId, ret, handle);
    }

    return false;
}

bool FileOperationsEventReceiver::handleOperationHideFiles(const quint64 windowId, const QList<QUrl> urls)
{
    Q_UNUSED(windowId)

    for (const QUrl &url : urls) {
        AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
        if (info) {
            const QUrl &parentUrl = info->parentUrl();
            const QString &fileName = info->fileName();

            HideFileHelper helper(parentUrl);
            helper.contains(fileName) ? helper.remove(fileName) : helper.insert(fileName);
            helper.save();
        }
    }

    return true;
}

void FileOperationsEventReceiver::handleOperationHideFiles(const quint64 windowId, const QList<QUrl> urls, const QVariant custom, OperatorCallback callback)
{
    bool ok = handleOperationHideFiles(windowId, urls);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << urls));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

void FileOperationsEventReceiver::invokeRegister(const QString scheme, const FileOperationsFunctions functions)
{
    QMutexLocker lk(functionsMutex.data());
    this->functions.insert(scheme, functions);
}

void FileOperationsEventReceiver::invokeUnregister(const QString scheme)
{
    QMutexLocker lk(functionsMutex.data());
    functions.remove(scheme);
}
