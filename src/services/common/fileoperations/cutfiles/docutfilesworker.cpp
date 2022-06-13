/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "docutfilesworker.h"
#include "fileoperations/fileoperationutils/fileoperationsutils.h"
#include "services/common/fileoperations/copyfiles/storageinfo.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/decorator/decoratorfileinfo.h"

#include "dfm-io/core/diofactory.h"

#include <QUrl>
#include <QProcess>
#include <QMutex>
#include <QStorageInfo>
#include <QWaitCondition>
#include <QQueue>
#include <QDebug>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <syscall.h>

DSC_USE_NAMESPACE

DoCutFilesWorker::DoCutFilesWorker(QObject *parent)
    : FileOperateBaseWorker(parent)
{
    jobType = AbstractJobHandler::JobType::kCutType;
}

DoCutFilesWorker::~DoCutFilesWorker()
{
    stop();
}

bool DoCutFilesWorker::doWork()
{
    // The endcopy interface function has been called here
    if (!AbstractWorker::doWork())
        return false;

    // check progress notify type
    determineCountProcessType();

    // 执行剪切
    if (!cutFiles()) {
        endWork();
        return false;
    }

    // sync
    syncFilesToDevice();

    // 完成
    endWork();

    return true;
}

void DoCutFilesWorker::stop()
{
    AbstractWorker::stop();
}

bool DoCutFilesWorker::initArgs()
{
    time.start();

    AbstractWorker::initArgs();

    if (sourceUrls.count() <= 0) {
        // pause and emit error msg
        doHandleErrorAndWait(QUrl(), QUrl(), AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }
    if (!targetUrl.isValid()) {
        // pause and emit error msg
        doHandleErrorAndWait(sourceUrls.first(), targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }
    targetInfo = InfoFactory::create<AbstractFileInfo>(targetUrl);
    if (!targetInfo) {
        // pause and emit error msg
        doHandleErrorAndWait(sourceUrls.first(), targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }

    if (!targetInfo->exists()) {
        // pause and emit error msg
        doHandleErrorAndWait(sourceUrls.first(), targetUrl, AbstractJobHandler::JobErrorType::kNonexistenceError);
        return false;
    }

    targetStorageInfo.reset(new StorageInfo(targetUrl.path()));

    return true;
}

bool DoCutFilesWorker::cutFiles()
{
    for (const auto &url : sourceUrls) {
        if (!stateCheck()) {
            return false;
        }

        const auto &fileInfo = InfoFactory::create<AbstractFileInfo>(url);
        if (!fileInfo) {
            // pause and emit error msg
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(url, targetUrl, AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                continue;
            }
        }
        fileInfo->refresh();

        // check self
        if (checkSelf(fileInfo))
            continue;

        // check hierarchy
        if (fileInfo->isDir()) {
            const bool higher = FileUtils::isHigherHierarchy(url, targetUrl) || url == targetUrl;
            if (higher) {
                emit requestShowTipsDialog(DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType::kCopyMoveToSelf, {});
                return false;
            }
        }

        // check link
        if (fileInfo->isSymLink()) {
            const bool ok = checkSymLink(fileInfo);
            if (ok)
                continue;
            else
                return false;
        }

        if (!doCutFile(fileInfo, targetInfo)) {
            return false;
        }
    }
    return true;
}

bool DoCutFilesWorker::doCutFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &targetPathInfo)
{
    // try rename
    bool ok = false;
    AbstractFileInfoPointer toInfo = nullptr;
    if (doRenameFile(fromInfo, targetPathInfo, toInfo, &ok) || ok) {
        return true;
    }

    if (stopWork.load()) {
        stopWork.store(false);
        return false;
    }

    qDebug() << "do rename failed, use copy and delete way, from url: " << fromInfo->url() << " to url: " << targetPathInfo->url();

    bool result = false;
    // check space
    if (!checkDiskSpaceAvailable(fromInfo->url(), targetPathInfo->url(), targetStorageInfo, &result))
        return result;

    if (!copyAndDeleteFile(fromInfo, targetPathInfo, toInfo, &result))
        return result;

    return true;
}

void DoCutFilesWorker::onUpdateProgress()
{
    const qint64 writSize = getWriteDataSize();
    emitProgressChangedNotify(writSize);
    emitSpeedUpdatedNotify(writSize);
}

void DoCutFilesWorker::emitCompleteFilesUpdatedNotify(const qint64 &writCount)
{
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kCompleteFilesKey, QVariant::fromValue(writCount));

    emit stateChangedNotify(info);
}

void DoCutFilesWorker::doOperateWork(AbstractJobHandler::SupportActions actions)
{
    AbstractWorker::doOperateWork(actions);
    resume();
}

bool DoCutFilesWorker::checkSymLink(const AbstractFileInfoPointer &fileInfo)
{
    const QUrl &sourceUrl = fileInfo->url();
    AbstractFileInfoPointer newTargetInfo(nullptr);
    bool result = false;
    bool ok = doCheckFile(fileInfo, targetInfo, fileInfo->fileName(), newTargetInfo, &result);
    if (!ok && !result)
        return false;
    ok = createSystemLink(fileInfo, newTargetInfo, true, false, &result);
    if (!ok && !result)
        return false;
    ok = deleteFile(sourceUrl, QUrl(), &result);
    if (!ok && !result)
        return false;

    completeSourceFiles.append(sourceUrl);
    completeTargetFiles.append(newTargetInfo->url());

    return true;
}

bool DoCutFilesWorker::checkSelf(const AbstractFileInfoPointer &fileInfo)
{
    const QString &fileName = fileInfo->fileName();
    QString newFileUrl = targetInfo->url().toString();
    if (!newFileUrl.endsWith("/"))
        newFileUrl.append("/");
    newFileUrl.append(fileName);
    DecoratorFileInfo newFileInfo(QUrl(newFileUrl, QUrl::TolerantMode));

    if (newFileInfo.url() == fileInfo->url()
        || (FileUtils::isSameFile(fileInfo->url(), newFileInfo.url()) && !fileInfo->isSymLink())) {
        return true;
    }
    return false;
}

bool DoCutFilesWorker::renameFileByHandler(const AbstractFileInfoPointer &sourceInfo, const AbstractFileInfoPointer &targetInfo)
{
    if (handler) {
        const QUrl &sourceUrl = sourceInfo->url();
        const QUrl &targetUrl = targetInfo->url();
        return handler->renameFile(sourceUrl, targetUrl);
    }
    return false;
}

bool DoCutFilesWorker::doRenameFile(const AbstractFileInfoPointer &sourceInfo, const AbstractFileInfoPointer &targetPathInfo, AbstractFileInfoPointer &toInfo, bool *ok)
{
    QSharedPointer<QStorageInfo> sourceStorageInfo = nullptr;
    sourceStorageInfo.reset(new QStorageInfo(sourceInfo->url().path()));

    const QUrl &sourceUrl = sourceInfo->url();

    toInfo.reset(nullptr);
    if (sourceStorageInfo->device() == targetStorageInfo->device()) {
        if (!doCheckFile(sourceInfo, targetPathInfo, sourceInfo->fileName(), toInfo, ok))
            return *ok;

        *ok = renameFileByHandler(sourceInfo, toInfo);
        if (*ok) {
            if (targetPathInfo == this->targetInfo) {
                completeSourceFiles.append(sourceUrl);
                completeTargetFiles.append(toInfo->url());
            }
        }
        return *ok;
    }

    if (!toInfo && !doCheckFile(sourceInfo, targetPathInfo, sourceInfo->fileName(), toInfo, ok))
        return false;

    return false;
}
