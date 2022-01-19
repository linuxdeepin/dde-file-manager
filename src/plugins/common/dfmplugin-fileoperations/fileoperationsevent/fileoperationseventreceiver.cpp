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
#include "dfm-base/base/urlroute.h"
#include "dfm-base/file/local/localfilehandler.h"
#include "dfm-base/utils/windowutils.h"
#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/framework.h>

#include <QDebug>
#include <QUrl>

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

FileOperationsEventReceiver::FileOperationsEventReceiver(QObject *parent)
    : QObject(parent)
{
    getServiceMutex.reset(new QMutex);
    functionsMutex.reset(new QMutex);
    copyMoveJob.reset(new FileCopyMoveJob);
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
    if (dialogService.isNull()) {
        auto &ctx = DPF_NAMESPACE::Framework::instance().serviceContext();
        dialogService = ctx.service<DSC_NAMESPACE::DialogService>(DSC_NAMESPACE::DialogService::name());
        if (!dialogService) {
            QString errStr;
            if (!ctx.load(DSC_NAMESPACE::DialogService::name(), &errStr)) {
                qCritical() << errStr;
                abort();
            }
            dialogService = ctx.service<DSC_NAMESPACE::DialogService>(DSC_NAMESPACE::DialogService::name());
        }
    }
    return operationsService && dialogService;
}

bool FileOperationsEventReceiver::getDialogService()
{
    QMutexLocker lk(getServiceMutex.data());
    if (dialogService.isNull()) {
        auto &ctx = DPF_NAMESPACE::Framework::instance().serviceContext();
        dialogService = ctx.service<DSC_NAMESPACE::DialogService>(DSC_NAMESPACE::DialogService::name());
        if (!dialogService) {
            QString errStr;
            if (!ctx.load(DSC_NAMESPACE::DialogService::name(), &errStr)) {
                qCritical() << errStr;
                abort();
            }
            dialogService = ctx.service<DSC_NAMESPACE::DialogService>(DSC_NAMESPACE::DialogService::name());
        }
    }
    return dialogService;
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

JobHandlePointer FileOperationsEventReceiver::handleOperationCopy(const quint64 windowId,
                                                                  const QList<QUrl> sources,
                                                                  const QUrl target,
                                                                  const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(windowId);
    if (!sources.isEmpty() && !sources.first().isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(sources.first().scheme());
        }
        if (function && function->copy) {
            return function->copy(windowId, sources, target, flags);
        }
    }
    return copyMoveJob->copy(sources, target, flags);
}

JobHandlePointer FileOperationsEventReceiver::handleOperationCut(quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(windowId);
    if (!sources.isEmpty() && !sources.first().isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(sources.first().scheme());
        }
        if (function && function->cut) {
            return function->cut(windowId, sources, target, flags);
        }
    }
    return copyMoveJob->cut(sources, target, flags);
}

JobHandlePointer FileOperationsEventReceiver::handleOperationMoveToTrash(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(windowId);
    if (!sources.isEmpty() && !sources.first().isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(sources.first().scheme());
        }
        if (function && function->moveToTash) {
            return function->moveToTash(windowId, sources, flags);
        }
    }
    return copyMoveJob->moveToTrash(sources, flags);
}

JobHandlePointer FileOperationsEventReceiver::handleOperationRestoreFromTrash(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(windowId);
    if (!sources.isEmpty() && !sources.first().isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(sources.first().scheme());
        }
        if (function && function->restoreFromTrash) {
            return function->restoreFromTrash(windowId, sources, flags);
        }
    }
    return copyMoveJob->restoreFromTrash(sources, flags);
}

JobHandlePointer FileOperationsEventReceiver::handleOperationDeletes(const quint64 windowId,
                                                                     const QList<QUrl> sources,
                                                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(windowId);
    if (!sources.isEmpty() && !sources.first().isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(sources.first().scheme());
        }
        if (function && function->deletes) {
            return function->deletes(windowId, sources, flags);
        }
    }
    return copyMoveJob->deletes(sources, flags);
}

JobHandlePointer FileOperationsEventReceiver::handleOperationCleanTrash(const quint64 windowId, const QList<QUrl> sources)
{
    Q_UNUSED(windowId);
    if (!sources.isEmpty() && !sources.first().isLocalFile()) {
        FileOperationsFunctions function { nullptr };
        {
            QMutexLocker lk(functionsMutex.data());
            function = this->functions.value(sources.first().scheme());
        }
        if (function && function->cleanTrash) {
            return function->cleanTrash(windowId, sources);
        }
    }
    return copyMoveJob->cleanTrash(sources);
}

void FileOperationsEventReceiver::handleOperationCopy(const quint64 windowId,
                                                      const QList<QUrl> sources,
                                                      const QUrl target,
                                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                      DFMBASE_NAMESPACE::Global::OperaterCallback callback)
{
    JobHandlePointer handle = copyMoveJob->copy(sources, target, flags);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
        callback(args);
    }
}

void FileOperationsEventReceiver::handleOperationCut(const quint64 windowId,
                                                     const QList<QUrl> sources,
                                                     const QUrl target,
                                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                     DFMBASE_NAMESPACE::Global::OperaterCallback callback)
{
    JobHandlePointer handle = copyMoveJob->cut(sources, target, flags);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
        callback(args);
    }
}

void FileOperationsEventReceiver::handleOperationMoveToTrash(const quint64 windowId,
                                                             const QList<QUrl> sources,
                                                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                             DFMBASE_NAMESPACE::Global::OperaterCallback callback)
{
    JobHandlePointer handle = copyMoveJob->moveToTrash(sources, flags);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
        callback(args);
    }
}

void FileOperationsEventReceiver::handleOperationRestoreFromTrash(const quint64 windowId,
                                                                  const QList<QUrl> sources,
                                                                  const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                                  DFMBASE_NAMESPACE::Global::OperaterCallback callback)
{
    JobHandlePointer handle = copyMoveJob->restoreFromTrash(sources, flags);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
        callback(args);
    }
}

void FileOperationsEventReceiver::handleOperationDeletes(const quint64 windowId,
                                                         const QList<QUrl> sources,
                                                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                         DFMBASE_NAMESPACE::Global::OperaterCallback callback)
{
    JobHandlePointer handle = copyMoveJob->deletes(sources, flags);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
        callback(args);
    }
}

void FileOperationsEventReceiver::handleOperationCleanTrash(const quint64 windowId, const QList<QUrl> sources, OperaterCallback callback)
{
    JobHandlePointer handle = copyMoveJob->deletes(sources);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
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
            if (!ok && getDialogService()) {
                dialogService->showErrorDialog("open file error", error);
            }
            // TODO:: file Open finished need to send file Open finished event
            dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFiles, windowId, urls, ok, error);
            return ok;
        }
    }
    DFMBASE_NAMESPACE::LocalFileHandler filehandler;
    ok = filehandler.openFiles(urls);
    if (!ok && getDialogService()) {
        error = filehandler.errorString();
        dialogService->showErrorDialog("open file error", error);
    }
    // TODO:: file Open finished need to send file Open finished event
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFiles, windowId, urls, ok, error);
    return ok;
}

void FileOperationsEventReceiver::handleOperationOpenFiles(const quint64 windowId,
                                                           const QList<QUrl> urls,
                                                           const QVariant custom,
                                                           DFMBASE_NAMESPACE::Global::OperaterCallback callback)
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
                                                                DFMBASE_NAMESPACE::Global::OperaterCallback callback)
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
            if (!ok && getDialogService()) {
                dialogService->showErrorDialog("open file by app error", error);
            }
            // TODO:: file openFilesByApp finished need to send file openFilesByApp finished event
            dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFilesByAppResult, windowId, urls, ok, error);
            return ok;
        }
    }
    DFMBASE_NAMESPACE::LocalFileHandler filehandler;
    QString app;
    if (apps.count() == 1) {
        app = apps.at(0);
    }
    ok = filehandler.openFilesByApp(urls, app);
    if (!ok && getDialogService()) {
        error = filehandler.errorString();
        dialogService->showErrorDialog("open file by app error", error);
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
            if (!ok && getDialogService()) {
                dialogService->showErrorDialog("rename file error", error);
            }
            // TODO:: file renameFile finished need to send file renameFile finished event
            dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                                  windowId, QList<QUrl>() << oldUrl << newUrl, ok, error);
            return ok;
        }
    }
    DFMBASE_NAMESPACE::LocalFileHandler filehandler;
    ok = filehandler.renameFile(oldUrl, newUrl);
    if (!ok && getDialogService()) {
        error = filehandler.errorString();
        dialogService->showErrorDialog("rename file error", error);
    }
    // TODO:: file renameFile finished need to send file renameFile finished event
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                          windowId, QList<QUrl>() << oldUrl << newUrl, ok, error);
    return ok;
}

void FileOperationsEventReceiver::handleOperationRenameFile(const quint64 windowId,
                                                            const QUrl oldUrl,
                                                            const QUrl newUrl, const QVariant custom,
                                                            OperaterCallback callback)
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

QString FileOperationsEventReceiver::handleOperationMkdir(const quint64 windowId,
                                                          const QUrl url,
                                                          const CreateFileType fileType)
{
    QString newPath = newDocmentName(url.path(), QString(), fileType);
    if (newPath.isEmpty())
        return newPath;

    return handleOperationMkdir(windowId, QUrl::fromLocalFile(newPath)) ? newPath : QString();
}

void FileOperationsEventReceiver::handleOperationMkdir(const quint64 windowId,
                                                       const QUrl url,
                                                       CreateFileType fileType,
                                                       const QVariant custom,
                                                       OperaterCallback callback)
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

void FileOperationsEventReceiver::handleOperationMkdir(const quint64 windowId,
                                                       const QUrl url, const QVariant custom,
                                                       OperaterCallback callback)
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

bool FileOperationsEventReceiver::handleOperationMkdir(const quint64 windowId,
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
        if (function && function->makedir) {
            ok = function->makedir(windowId, url, &error);
            if (!ok && getDialogService()) {
                dialogService->showErrorDialog("make dir error", error);
            }
            // TODO:: make dir finished need to send make dir finished event
            dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kMkdirResult,
                                                  windowId, QList<QUrl>() << url, ok, error);
            return ok;
        }
    }
    DFMBASE_NAMESPACE::LocalFileHandler filehandler;
    ok = filehandler.mkdir(url);
    if (!ok && getDialogService()) {
        error = filehandler.errorString();
        dialogService->showErrorDialog("make dir error", error);
    }
    // TODO:: make dir finished need to send make dir finished event
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kMkdirResult,
                                          windowId, QList<QUrl>() << url, ok, error);
    return ok;
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
            ok = function->touchFile(windowId, url, &error);
            if (!ok && getDialogService()) {
                dialogService->showErrorDialog("touch file error", error);
            }
            // TODO:: touch file finished need to send touch file finished event
            dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kTouchFileResult,
                                                  windowId, QList<QUrl>() << url, ok, error);
            return ok;
        }
    }
    DFMBASE_NAMESPACE::LocalFileHandler filehandler;
    ok = filehandler.touchFile(url);
    if (!ok && getDialogService()) {
        error = filehandler.errorString();
        dialogService->showErrorDialog("touch file error", error);
    }
    // TODO:: touch file finished need to send touch file finished event
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kTouchFileResult,
                                          windowId, QList<QUrl>() << url, ok, error);
    return ok;
}

void FileOperationsEventReceiver::handleOperationTouchFile(const quint64 windowId,
                                                           const QUrl url, const QVariant custom,
                                                           OperaterCallback callback)
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
                                                              const CreateFileType fileType, const QString suffix)
{
    QString newPath = newDocmentName(url.path(), suffix, fileType);
    if (newPath.isEmpty())
        return newPath;

    return handleOperationTouchFile(windowId, QUrl::fromLocalFile(newPath)) ? newPath : QString();
}

void FileOperationsEventReceiver::handleOperationTouchFile(const quint64 windowId,
                                                           const QUrl url,
                                                           const CreateFileType fileType,
                                                           const QString suffix,
                                                           const QVariant custom,
                                                           OperaterCallback callback)
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
            if (!ok && getDialogService()) {
                dialogService->showErrorDialog("link file error", error);
            }
            // TODO:: link file finished need to send link file finished event
            dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kCreateSymlinkResult,
                                                  windowId, QList<QUrl>() << url << link, ok, error);
            return ok;
        }
    }

    DFMBASE_NAMESPACE::LocalFileHandler filehandler;
    ok = filehandler.createSystemLink(url, link);
    if (!ok && getDialogService()) {
        error = filehandler.errorString();
        dialogService->showErrorDialog("link file error", error);
    }
    // TODO:: link file finished need to send link file finished event
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kCreateSymlinkResult,
                                          windowId, QList<QUrl>() << url << link, ok, error);
    return ok;
}

void FileOperationsEventReceiver::handleOperationLinkFile(const quint64 windowId,
                                                          const QUrl url,
                                                          const QUrl link, const QVariant custom,
                                                          DFMBASE_NAMESPACE::Global::OperaterCallback callback)
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
            if (!ok && getDialogService()) {
                dialogService->showErrorDialog("set file permissions error", error);
            }
            // TODO:: set file permissions finished need to send set file permissions finished event
            dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kSetPermissionResult,
                                                  windowId, QList<QUrl>() << url, ok, error);
            return ok;
        }
    }
    DFMBASE_NAMESPACE::LocalFileHandler filehandler;
    ok = filehandler.setPermissions(url, permissions);
    if (!ok && getDialogService()) {
        error = filehandler.errorString();
        dialogService->showErrorDialog("set file permissions error", error);
    }
    // TODO:: set file permissions finished need to send set file permissions finished event
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kSetPermissionResult,
                                          windowId, QList<QUrl>() << url, ok, error);
    return ok;
}

void FileOperationsEventReceiver::handleOperationSetPermission(const quint64 windowId,
                                                               const QUrl url,
                                                               const QFileDevice::Permissions permissions, const QVariant custom,
                                                               DFMBASE_NAMESPACE::Global::OperaterCallback callback)
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
    QSharedPointer<LocalFileHandler> filehandler = nullptr;
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
            if (!ok && getDialogService()) {
                dialogService->showErrorDialog("open file in terminal error", error);
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
        if (filehandler.isNull())
            filehandler.reset(new LocalFileHandler());
        ok = QProcess::startDetached(filehandler->defaultTerminalPath());
        if (!result)
            result = ok;
        QDir::setCurrent(current_dir);
    }

    // TODO:: open file in terminal finished need to send open file in terminal finished event
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenInTerminalResult,
                                          windowId, urls, result, error);

    return ok;
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
