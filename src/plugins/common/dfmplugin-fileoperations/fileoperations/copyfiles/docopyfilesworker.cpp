// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "docopyfilesworker.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/abstractdiriterator.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/protocolutils.h>

#include <dfm-io/dfmio_utils.h>

#include <QUrl>

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
    if (!workData) {
        return false;
    }
    // 深信服远程下载
    if (sourceUrls.isEmpty() && workData->jobFlags.testFlag(DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kCopyRemote)) {
        sourceUrls = dfmbase::ClipBoard::instance()->getRemoteUrls();
        emit requestTaskDailog();
        fmInfo() << "Remote copy initiated - source count:" << sourceUrls.count();
    }
    // The endcopy interface function has been called here
    if (!AbstractWorker::doWork())
        return false;

    // check progress notify type
    determineCountProcessType();

    // 检查磁盘空间
    if (!checkTotalDiskSpaceAvailable(sourceUrls.isEmpty() ? QUrl() : sourceUrls.first(), targetOrgUrl, nullptr)) {
        endWork();
        return false;
    }

    // init copy file ways
    initCopyWay();

    // do main process
    fmInfo() << "Start copying files - count:" << sourceUrls.count();
    if (!copyFiles()) {
        endWork();
        return false;
    }

    // end
    fmInfo() << "Copy operation completed successfully";
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
    AbstractWorker::initArgs();

    if (sourceUrls.count() <= 0) {
        // pause and emit error msg
        fmCritical() << "Copy operation failed: no source files";
        doHandleErrorAndWait(QUrl(), QUrl(), AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }
    if (!targetUrl.isValid()) {
        // pause and emit error msg
        fmCritical() << "Copy operation failed: invalid target URL";
        doHandleErrorAndWait(QUrl(), targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }
    targetInfo.reset(new DFileInfo(targetUrl));
    if (!targetInfo) {
        // pause and emit error msg
        fmCritical() << "Copy operation failed: cannot create target file info";
        doHandleErrorAndWait(QUrl(), targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }
    targetInfo->initQuerier();
    if (!targetInfo->exists()) {
        // pause and emit error msg
        fmCritical() << "Copy operation failed: target directory does not exist";
        doHandleErrorAndWait(QUrl(), targetUrl, AbstractJobHandler::JobErrorType::kNonexistenceError, true);
        return false;
    }

    if (targetInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool())
        targetOrgUrl = QUrl::fromLocalFile(targetInfo->attribute(DFileInfo::AttributeID::kStandardSymlinkTarget).toString());

    return true;
}

void DoCopyFilesWorker::endWork()
{
    waitThreadPoolOver();

    // deal target files
    for (DFileInfoPointer info : precompleteTargetFileInfo) {
        info->initQuerier();
        if (info->exists()) {
            const QUrl &url = info->uri();
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
        DFileInfoPointer fileInfo(new DFileInfo(url));
        if (!targetInfo) {
            // pause and emit error msg
            fmCritical() << "Copy operation failed: target info is null";
            const AbstractJobHandler::SupportAction action = doHandleErrorAndWait(url, targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
            if (AbstractJobHandler::SupportAction::kSkipAction != action) {
                return false;
            } else {
                continue;
            }
        }

        // check self
        if (fileInfo->attribute(DFileInfo::AttributeID::kStandardIsDir).toBool()) {
            const bool higher = FileUtils::isHigherHierarchy(url, targetUrl) || url == targetUrl;
            if (higher) {
                fmWarning() << "Cannot copy directory to itself or parent - from:" << url << "to:" << targetUrl;
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
