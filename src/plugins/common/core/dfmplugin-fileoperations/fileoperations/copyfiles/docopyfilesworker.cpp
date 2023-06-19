// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "docopyfilesworker.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/abstractdiriterator.h>
#include <dfm-base/utils/clipboard.h>

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

DPFILEOPERATIONS_USE_NAMESPACE
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
    // 深信服远程下载
    if (sourceUrls.isEmpty() && workData->jobFlags.testFlag(DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kCopyRemote)) {
        sourceUrls = dfmbase::ClipBoard::instance()->getRemoteUrls();
        qInfo() << "remote copy source urls list:" << sourceUrls;
    }
    // The endcopy interface function has been called here
    if (!AbstractWorker::doWork())
        return false;

    // check progress notify type
    determineCountProcessType();

    // 检查磁盘空间
    if (!checkTotalDiskSpaceAvailable(sourceUrls.isEmpty() ? QUrl() : sourceUrls.first(), targetOrgUrl, nullptr)){
        endWork();
        return false;
    }

    // init copy file ways
    initCopyWay();

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
    threadCopyFileCount = 0;

    FileOperateBaseWorker::stop();
}

bool DoCopyFilesWorker::initArgs()
{
    time.start();

    AbstractWorker::initArgs();

    if (sourceUrls.count() <= 0) {
        // pause and emit error msg
        qCritical() << "sorce file count = 0!!!";
        doHandleErrorAndWait(QUrl(), QUrl(), AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }
    if (!targetUrl.isValid()) {
        // pause and emit error msg
        qCritical() << "target url is Valid !!!";
        doHandleErrorAndWait(QUrl(), targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }
    targetInfo = InfoFactory::create<FileInfo>(targetUrl, Global::CreateFileInfoType::kCreateFileInfoSync);
    if (!targetInfo) {
        // pause and emit error msg
        qCritical() << "create target info error, url = " << targetUrl;
        doHandleErrorAndWait(QUrl(), targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }

    if (!targetInfo->exists()) {
        // pause and emit error msg
        qCritical() << "target dir is not exists, url = " << targetUrl;
        doHandleErrorAndWait(QUrl(), targetUrl, AbstractJobHandler::JobErrorType::kNonexistenceError, true);
        return false;
    }

    if (targetInfo->isAttributes(OptInfoType::kIsSymLink))
        targetOrgUrl = QUrl::fromLocalFile(targetInfo->pathOf(PathInfoType::kSymLinkTarget));

    workData->needSyncEveryRW = FileUtils::isGvfsFile(targetUrl);
    if (!workData->needSyncEveryRW) {
        const QString &fsType = DFMIO::DFMUtils::fsTypeFromUrl(targetUrl);
        workData->isFsTypeVfat = fsType.contains("vfat");
        workData->needSyncEveryRW = fsType == "cifs" || fsType == "vfat";
    }

    return true;
}

void DoCopyFilesWorker::endWork()
{
    waitThreadPoolOver();

    // deal target files
    for (FileInfoPointer info : precompleteTargetFileInfo) {
        if (info->exists()) {
            const QUrl &url = info->urlOf(UrlInfoType::kUrl);
            completeTargetFiles.append(url);
            info->refresh();
        }
    }
    precompleteTargetFileInfo.clear();

    // set dirs permissions
    setAllDirPermisson();

    FileOperateBaseWorker::endWork();
}

bool DoCopyFilesWorker::copyFiles()
{
    for (const QUrl &url : sourceUrls) {
        if (!stateCheck()) {
            return false;
        }
        FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
        if (!fileInfo || !targetInfo) {
            // pause and emit error msg
            qCritical() << "sorce file Info or target file info is nullptr : source file info is nullptr = " << (fileInfo == nullptr) << ", source file info is nullptr = " << (targetInfo == nullptr);
            const AbstractJobHandler::SupportAction action = doHandleErrorAndWait(url, targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
            if (AbstractJobHandler::SupportAction::kSkipAction != action) {
                return false;
            } else {
                continue;
            }
        }
        fileInfo->refresh();

        // check self
        if (fileInfo->isAttributes(OptInfoType::kIsDir)) {
            const bool higher = FileUtils::isHigherHierarchy(url, targetUrl) || url == targetUrl;
            if (higher) {
                emit requestShowTipsDialog(DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType::kCopyMoveToSelf, {});
                return false;
            }
        }

        bool skip = false;
        if (!doCopyFile(fileInfo, targetInfo, &skip)) {
            if (skip) {
                continue;
            } else {
                return false;
            }
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
