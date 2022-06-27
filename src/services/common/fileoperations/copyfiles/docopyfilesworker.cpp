/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#include "docopyfilesworker.h"
#include "storageinfo.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/interfaces/abstractdiriterator.h"
#include "dfm-base/utils/decorator/decoratorfileinfo.h"

#include <dfm-io/dfmio_global.h>
#include <dfm-io/core/diofactory.h>
#include <dfm-io/dfmio_utils.h>

#include <QUrl>
#include <QDebug>
#include <QApplication>
#include <QTime>
#include <QRegularExpression>
#include <QProcess>
#include <QtConcurrent/QtConcurrent>

#include <syscall.h>
#include <fcntl.h>
#include <zlib.h>

using namespace dfm_service_common;
USING_IO_NAMESPACE

DoCopyFilesWorker::DoCopyFilesWorker(QObject *parent)
    : FileOperateBaseWorker(parent)
{
    jobType = AbstractJobHandler::JobType::kCopyType;
}

DoCopyFilesWorker::~DoCopyFilesWorker()
{
    stop();
}

bool DoCopyFilesWorker::doWork()
{
    // The endcopy interface function has been called here
    if (!AbstractWorker::doWork())
        return false;

    // check progress notify type
    determineCountProcessType();

    // do main process
    if (!copyFiles()) {
        endWork();
        return false;
    }

    // sync
    syncFilesToDevice();

    // end
    endWork();

    return true;
}

void DoCopyFilesWorker::stop()
{
    // clean muilt thread copy file info queue
    if (smallFileThreadCopyInfoQueue) {
        if (!smallFileThreadCopyInfoQueueMutex)
            smallFileThreadCopyInfoQueueMutex.reset(new QMutex);

        QMutexLocker lk(smallFileThreadCopyInfoQueueMutex.data());
        smallFileThreadCopyInfoQueue.clear();
    }

    AbstractWorker::stop();
}

bool DoCopyFilesWorker::initArgs()
{
    time.start();

    AbstractWorker::initArgs();

    if (sourceUrls.count() <= 0) {
        // pause and emit error msg
        doHandleErrorAndWait(QUrl(), QUrl(), AbstractJobHandler::JobErrorType::kProrogramError);
        cancelThreadProcessingError();
        return false;
    }
    if (!targetUrl.isValid()) {
        // pause and emit error msg
        doHandleErrorAndWait(QUrl(), targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
        cancelThreadProcessingError();
        return false;
    }
    targetInfo = InfoFactory::create<AbstractFileInfo>(targetUrl);
    if (!targetInfo) {
        // pause and emit error msg
        doHandleErrorAndWait(QUrl(), targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
        cancelThreadProcessingError();
        return false;
    }

    if (!targetInfo->exists()) {
        // pause and emit error msg
        doHandleErrorAndWait(QUrl(), targetUrl, AbstractJobHandler::JobErrorType::kNonexistenceError);
        cancelThreadProcessingError();
        return false;
    }

    needSyncEveryRW = FileUtils::isGvfsFile(targetUrl);
    if (!needSyncEveryRW) {
        const QString &fsType = DFMIO::DFMUtils::fsTypeFromUrl(targetUrl);
        isFsTypeVfat = fsType.contains("vfat");
        needSyncEveryRW = fsType == "cifs" || fsType == "vfat";
    }

    return true;
}

void DoCopyFilesWorker::endWork()
{
    // wait for thread pool over
    if (threadPool) {
        while (threadPool->activeThreadCount() > 0) {
            QThread::msleep(100);
        }
    }

    // deal target files
    for (AbstractFileInfoPointer info : precompleteTargetFileInfo) {
        if (info->exists()) {
            completeTargetFiles.append(info->url());
            info->refresh();
        }
    }
    precompleteTargetFileInfo.clear();

    // set dirs permissions
    setAllDirPermisson();

    AbstractWorker::endWork();
}

bool DoCopyFilesWorker::copyFiles()
{
    for (const QUrl &url : sourceUrls) {
        if (!stateCheck()) {
            return false;
        }
        AbstractFileInfoPointer fileInfo = InfoFactory::create<AbstractFileInfo>(url);
        if (!fileInfo || !targetInfo) {
            // pause and emit error msg
            const AbstractJobHandler::SupportAction action = doHandleErrorAndWait(url, targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
            if (AbstractJobHandler::SupportAction::kSkipAction != action) {
                cancelThreadProcessingError();
                return false;
            } else {
                continue;
            }
        }
        fileInfo->refresh();

        // check self
        if (fileInfo->isDir()) {
            const bool higher = FileUtils::isHigherHierarchy(url, targetUrl) || url == targetUrl;
            if (higher) {
                emit requestShowTipsDialog(DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType::kCopyMoveToSelf, {});
                return false;
            }
        }

        bool skip = false;
        if (!doCopyFile(fileInfo, targetInfo, &skip)) {
            if (skip)
                continue;
            else
                return false;
        }
    }
    return true;
}

/*!
 * \brief DoCopyFilesWorker::setStat Set current task status
 * \param stat task status
 */
void DoCopyFilesWorker::setStat(const AbstractJobHandler::JobState &stat)
{
    AbstractWorker::setStat(stat);

    if (isStopped() && smallFileThreadCopyInfoQueue) {
        if (smallFileThreadCopyInfoQueueMutex)
            smallFileThreadCopyInfoQueueMutex.reset(new QMutex);
        QMutexLocker lk(smallFileThreadCopyInfoQueueMutex.data());
        smallFileThreadCopyInfoQueue.clear();
    }
}

/*!
 * \brief DoCopyFilesWorker::onUpdateProccess update proccess and speed slot
 */
void DoCopyFilesWorker::onUpdateProgress()
{
    // 当前写入文件大小获取
    const qint64 writeSize = getWriteDataSize();
    emitProgressChangedNotify(writeSize);
    emitSpeedUpdatedNotify(writeSize);
}
