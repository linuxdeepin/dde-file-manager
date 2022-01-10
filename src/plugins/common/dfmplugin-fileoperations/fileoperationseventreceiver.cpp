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
#include "dfm-base/base/urlroute.h"
#include "fileoperations/fileoperationsutils.h"
#include "dfm-base/file/local/localfilehandler.h"

#include <QDebug>
#include <QUrl>
#include <dfm-framework/framework.h>

#include <functional>

DPFILEOPERATIONS_USE_NAMESPACE

FileOperationsEventReceiver::FileOperationsEventReceiver(QObject *parent)
    : QObject(parent)
{
    getServiceMutex.reset(new QMutex);
    copyMoveUtils.reset(new FileOperationsUtils);
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

FileOperationsEventReceiver *FileOperationsEventReceiver::instance()
{
    static FileOperationsEventReceiver receiver;
    return &receiver;
}

void FileOperationsEventReceiver::handleOperationCopy(quint64 windowId, const QList<QUrl> sources, const QUrl target,
                                                      const dfmbase::AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(windowId);
    copyMoveUtils->copy(sources, target, flags);
    // TODO:: file copy finished need to send copy finished event
}

void FileOperationsEventReceiver::handleOperationCut(quint64 windowId, const QList<QUrl> sources, const QUrl target, const dfmbase::AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(windowId);
    copyMoveUtils->cut(sources, target, flags);
    // TODO:: file cut finished need to send cut file finished event
}

void FileOperationsEventReceiver::handleOperationMoveToTrash(quint64 windowId, const QList<QUrl> sources, const dfmbase::AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(windowId);
    copyMoveUtils->moveToTrash(sources, flags);
    // TODO:: file moveToTrash finished need to send file moveToTrash finished event
}

void FileOperationsEventReceiver::handleOperationRestoreFromTrash(quint64 windowId, const QList<QUrl> sources, const dfmbase::AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(windowId);
    copyMoveUtils->restoreFromTrash(sources, flags);
    // TODO:: file restoreFromTrash finished need to send file restoreFromTrash finished event
}

void FileOperationsEventReceiver::handleOperationDeletes(quint64 windowId, const QList<QUrl> sources, const dfmbase::AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(windowId);
    copyMoveUtils->deletes(sources, flags);
    // TODO:: file deletes finished need to send file deletes finished event
}

void FileOperationsEventReceiver::handleOperationOpenFiles(quint64 windowId, QList<QUrl> urls)
{
    Q_UNUSED(windowId);
    DFMBASE_NAMESPACE::LocalFileHandler filehandler;
    if (!filehandler.openFiles(urls) && getDialogService()) {
        dialogService->showErrorDialog("open file error", filehandler.errorString());
    }
    // TODO:: file Open finished need to send file Open finished event
}

void FileOperationsEventReceiver::handleOperationOpenFilesByApp(quint64 windowId, QList<QUrl> urls, QList<QString> apps)
{
    Q_UNUSED(windowId);
    DFMBASE_NAMESPACE::LocalFileHandler filehandler;
    QString app;
    if (apps.count() == 1) {
        app = apps.at(0);
    }
    if (!filehandler.openFilesByApp(urls, app) && getDialogService()) {
        dialogService->showErrorDialog("open file error", filehandler.errorString());
    }
    // TODO:: file openFilesByApp finished need to send file openFilesByApp finished event
}

void FileOperationsEventReceiver::handleOperationRenameFile(quint64 windowId, QUrl oldUrl, QUrl newUrl)
{
    Q_UNUSED(windowId);
    DFMBASE_NAMESPACE::LocalFileHandler filehandler;
    if (!filehandler.renameFile(oldUrl, newUrl) && getDialogService()) {
        dialogService->showErrorDialog("rename file error", filehandler.errorString());
    }
    // TODO:: file renameFile finished need to send file renameFile finished event
}

void FileOperationsEventReceiver::handleOperationMkdir(quint64 windowId, QUrl url)
{
    Q_UNUSED(windowId);
    DFMBASE_NAMESPACE::LocalFileHandler filehandler;
    if (!filehandler.mkdir(url) && getDialogService()) {
        dialogService->showErrorDialog("make dir error", filehandler.errorString());
    }
    // TODO:: make dir finished need to send make dir finished event
}

void FileOperationsEventReceiver::handleOperationTouchFile(quint64 windowId, QUrl url)
{
    Q_UNUSED(windowId);
    DFMBASE_NAMESPACE::LocalFileHandler filehandler;
    if (!filehandler.touchFile(url) && getDialogService()) {
        dialogService->showErrorDialog("touch file error", filehandler.errorString());
    }
    // TODO:: touch file finished need to send touch file finished event
}

void FileOperationsEventReceiver::handleOperationLinkFile(quint64 windowId, QUrl url, QUrl link)
{
    Q_UNUSED(windowId);
    DFMBASE_NAMESPACE::LocalFileHandler filehandler;
    if (!filehandler.createSystemLink(url, link) && getDialogService()) {
        dialogService->showErrorDialog("link file error", filehandler.errorString());
    }
    // TODO:: link file finished need to send link file finished event
}

void FileOperationsEventReceiver::handleOperationSetPermission(quint64 windowId, QUrl url, QFileDevice::Permissions permissions)
{
    Q_UNUSED(windowId);
    DFMBASE_NAMESPACE::LocalFileHandler filehandler;
    if (!filehandler.setPermissions(url, permissions) && getDialogService()) {
        dialogService->showErrorDialog("set file permissions error", filehandler.errorString());
    }
    // TODO:: set file permissions finished need to send set file permissions finished event
}

void FileOperationsEventReceiver::handleOperationSetWriteToClipboard(quint64 windowId, dfmbase::ClipBoard::ClipboardAction action, QList<QUrl> urls)
{
    Q_UNUSED(windowId);
    DFMBASE_NAMESPACE::ClipBoard::instance()->setUrlsToClipboard(urls, action);
    // TODO:: set Write Urls to clipboard finished need to send set Write Urls to clipboard finished event
}
