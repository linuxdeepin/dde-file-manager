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
#include <QWaitCondition>

#include <syscall.h>
#include <fcntl.h>
#include <zlib.h>

constexpr uint32_t kBigFileSize { 300 * 1024 * 1024 };

DSC_USE_NAMESPACE
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
        if (info->exists())
            completeTargetFiles.append(info->url());
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

        bool workContinue = false;
        if (!doCopyFile(fileInfo, targetInfo, &workContinue)) {
            if (workContinue)
                continue;
            else
                return false;
        }
    }
    return true;
}

bool DoCopyFilesWorker::doCopyFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *workContinue)
{
    AbstractFileInfoPointer newTargetInfo(nullptr);
    bool result = false;
    if (!doCheckFile(fromInfo, toInfo, newTargetInfo, workContinue))
        return result;

    bool oldExist = newTargetInfo->exists();
    if (fromInfo->isSymLink()) {
        result = creatSystemLink(fromInfo, newTargetInfo, workContinue);
    } else if (fromInfo->isDir()) {
        result = checkAndCopyDir(fromInfo, newTargetInfo, workContinue);
    } else {
        result = checkAndCopyFile(fromInfo, newTargetInfo, workContinue);
    }

    if (targetInfo == toInfo && !oldExist) {
        completeFiles.append(fromInfo->url());
        precompleteTargetFileInfo.append(newTargetInfo);
    }

    return result;
}

bool DoCopyFilesWorker::creatSystemLink(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *workContinue)
{
    // 创建链接文件
    skipWritSize += dirSize;
    AbstractFileInfoPointer newFromInfo = fromInfo;
    if (jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyFollowSymlink)) {
        do {
            QUrl newUrl = newFromInfo->url();
            newUrl.setPath(newFromInfo->symLinkTarget());
            const AbstractFileInfoPointer &symlinkTarget = InfoFactory::create<AbstractFileInfo>(newUrl);

            if (!symlinkTarget || !symlinkTarget->exists()) {
                break;
            }

            newFromInfo = symlinkTarget;
        } while (newFromInfo->isSymLink());

        if (newFromInfo->exists()) {
            // copy file here
            if (fromInfo->isFile()) {
                return checkAndCopyFile(fromInfo, toInfo, workContinue);
            } else {
                return checkAndCopyDir(fromInfo, toInfo, workContinue);
            }
        }
    }

    AbstractJobHandler::SupportAction actionForlink { AbstractJobHandler::SupportAction::kNoAction };

    do {
        if (handler->createSystemLink(newFromInfo->url(), toInfo->url())) {
            return true;
        }
        actionForlink = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kSymlinkError, QString(QObject::tr("Fail to create symlink, cause: %1")).arg(handler->errorString()));
    } while (!isStopped() && actionForlink == AbstractJobHandler::SupportAction::kRetryAction);
    cancelThreadProcessingError();
    *workContinue = actionForlink == AbstractJobHandler::SupportAction::kSkipAction;
    return false;
}

bool DoCopyFilesWorker::checkAndCopyFile(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo, bool *workContinue)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

    while (!doCheckFileFreeSpace(fromInfo->size())) {
        action = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kNotEnoughSpaceError);
        if (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction) {
            continue;
        } else if (action == AbstractJobHandler::SupportAction::kSkipAction) {
            skipWritSize += fromInfo->size() <= 0 ? dirSize : fromInfo->size();
            cancelThreadProcessingError();
            *workContinue = true;
            return false;
        }

        if (action == AbstractJobHandler::SupportAction::kEnforceAction) {
            break;
        }
        cancelThreadProcessingError();
        return false;
    }
    cancelThreadProcessingError();

    if (isSourceFileLocal && isTargetFileLocal && fromInfo->size() < kBigFileSize) {
        if (!stateCheck())
            return false;

        QSharedPointer<SmallFileThreadCopyInfo> threadInfo(new SmallFileThreadCopyInfo);
        threadInfo->fromInfo = fromInfo;
        threadInfo->toInfo = toInfo;
        if (!smallFileThreadCopyInfoQueue)
            smallFileThreadCopyInfoQueue.reset(new QQueue<QSharedPointer<SmallFileThreadCopyInfo>>);

        {
            if (!smallFileThreadCopyInfoQueueMutex)
                smallFileThreadCopyInfoQueueMutex.reset(new QMutex);
            QMutexLocker lk(smallFileThreadCopyInfoQueueMutex.data());
            smallFileThreadCopyInfoQueue->enqueue(threadInfo);
        }

        if (!threadPool)
            threadPool.reset(new QThreadPool);
        QtConcurrent::run(threadPool.data(), this, static_cast<bool (DoCopyFilesWorker::*)()>(&DoCopyFilesWorker::doThreadPoolCopyFile));
        return true;
    }

    if (threadPool) {
        while (threadPool->activeThreadCount() > 0) {
            QThread::msleep(100);
        }
    }

    const QString &targetUrl = toInfo->url().toString();
    FileUtils::cacheCopyingFileUrl(targetUrl);
    bool result = false;
    bool ok = doCopyFilePractically(fromInfo, toInfo, &result);
    FileUtils::removeCopyingFileUrl(targetUrl);

    FileOperationsUtils::removeUsingName(toInfo->fileName());

    return ok;
}

bool DoCopyFilesWorker::checkAndCopyDir(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *workContinue)
{
    emitCurrentTaskNotify(fromInfo->url(), toInfo->url());
    // 检查文件的一些合法性，源文件是否存在，创建新的目标目录名称，检查新创建目标目录名称是否存在
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    QFileDevice::Permissions permissions = fromInfo->permissions();
    if (!toInfo->exists()) {
        do {
            if (handler->mkdir(toInfo->url()))
                break;

            action = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kMkdirError, QString(QObject::tr("Fail to create symlink, cause: %1")).arg(handler->errorString()));
        } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);
        cancelThreadProcessingError();
        if (AbstractJobHandler::SupportAction::kNoAction != action) {
            // skip write size += all file size in sources dir
            skipWritSize += dirSize;
            *workContinue = AbstractJobHandler::SupportAction::kSkipAction == action;
            return false;
        }

        if (jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyToSelf)) {
            QSharedPointer<DirSetPermissonInfo> dirinfo(new DirSetPermissonInfo);
            dirinfo->target = toInfo->url();
            dirinfo->permission = permissions;
            dirPermissonList.appendByLock(dirinfo);
            return true;
        }
    }

    if (fromInfo->countChildFile() <= 0) {
        handler->setPermissions(toInfo->url(), permissions);
        return true;
    }
    // 遍历源文件，执行一个一个的拷贝
    QString error;
    const AbstractDirIteratorPointer &iterator = DirIteratorFactory::create<AbstractDirIterator>(fromInfo->url(), &error);
    if (!iterator) {
        doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kProrogramError, QString(QObject::tr("create dir's iterator failed, case : %1")).arg(error));
        cancelThreadProcessingError();
        return false;
    }

    while (iterator->hasNext()) {
        if (!stateCheck()) {
            return false;
        }

        const QUrl &url = iterator->next();
        Q_UNUSED(url);
        const AbstractFileInfoPointer &info = iterator->fileInfo();
        if (!doCopyFile(info, toInfo, workContinue)) {
            return false;
        }
    }
    if (isTargetFileLocal && isSourceFileLocal) {
        QSharedPointer<DirSetPermissonInfo> dirinfo(new DirSetPermissonInfo);
        dirinfo->target = toInfo->url();
        dirinfo->permission = permissions;
        dirPermissonList.appendByLock(dirinfo);
    } else {
        handler->setPermissions(toInfo->url(), permissions);
    }

    return true;
}

bool DoCopyFilesWorker::doThreadPoolCopyFile()
{
    if (!stateCheck())
        return false;

    QSharedPointer<SmallFileThreadCopyInfo> threadInfo(nullptr);
    {
        if (!smallFileThreadCopyInfoQueueMutex)
            smallFileThreadCopyInfoQueueMutex.reset(new QMutex);

        QMutexLocker lk(smallFileThreadCopyInfoQueueMutex.data());
        if (!smallFileThreadCopyInfoQueue)
            smallFileThreadCopyInfoQueue.reset(new QQueue<QSharedPointer<SmallFileThreadCopyInfo>>);

        if (smallFileThreadCopyInfoQueue->count() <= 0) {
            return false;
        }
        threadInfo = smallFileThreadCopyInfoQueue->dequeue();
    }
    if (!threadInfo) {
        setStat(AbstractJobHandler::JobState::kStopState);
        qWarning() << " the threadInfo is nullptr, some error here! ";
        return false;
    }
    const QString &targetUrl = threadInfo->toInfo->url().toString();
    FileUtils::cacheCopyingFileUrl(targetUrl);
    bool result = false;
    bool ok = doCopyFilePractically(threadInfo->fromInfo, threadInfo->toInfo, &result);
    if (!ok)
        setStat(AbstractJobHandler::JobState::kStopState);
    FileUtils::removeCopyingFileUrl(targetUrl);
    FileOperationsUtils::removeUsingName(threadInfo->toInfo->fileName());
    return ok;
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
 * \brief DoCopyFilesWorker::supportActions get the support Acttions by error type
 * \param error error type
 * \return support action
 */
AbstractJobHandler::SupportActions DoCopyFilesWorker::supportActions(const AbstractJobHandler::JobErrorType &error)
{
    AbstractJobHandler::SupportActions support = AbstractJobHandler::SupportAction::kCancelAction;
    switch (error) {
    case AbstractJobHandler::JobErrorType::kPermissionError:
    case AbstractJobHandler::JobErrorType::kOpenError:
    case AbstractJobHandler::JobErrorType::kReadError:
    case AbstractJobHandler::JobErrorType::kWriteError:
    case AbstractJobHandler::JobErrorType::kSymlinkError:
    case AbstractJobHandler::JobErrorType::kMkdirError:
    case AbstractJobHandler::JobErrorType::kResizeError:
    case AbstractJobHandler::JobErrorType::kRemoveError:
    case AbstractJobHandler::JobErrorType::kRenameError:
    case AbstractJobHandler::JobErrorType::kIntegrityCheckingError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kRetryAction;
    case AbstractJobHandler::JobErrorType::kSpecialFileError:
        return AbstractJobHandler::SupportAction::kSkipAction;
    case AbstractJobHandler::JobErrorType::kFileSizeTooBigError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kEnforceAction;
    case AbstractJobHandler::JobErrorType::kNotEnoughSpaceError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kRetryAction | AbstractJobHandler::SupportAction::kEnforceAction;
    case AbstractJobHandler::JobErrorType::kFileExistsError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kReplaceAction | AbstractJobHandler::SupportAction::kCoexistAction;
    case AbstractJobHandler::JobErrorType::kDirectoryExistsError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kMergeAction | AbstractJobHandler::SupportAction::kCoexistAction;
    case AbstractJobHandler::JobErrorType::kTargetReadOnlyError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kEnforceAction;
    case AbstractJobHandler::JobErrorType::kTargetIsSelfError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kEnforceAction;
    case AbstractJobHandler::JobErrorType::kSymlinkToGvfsError:
        return support | AbstractJobHandler::SupportAction::kSkipAction;
    default:
        break;
    }

    return support;
}
/*!
 * \brief DoCopyFilesWorker::onUpdateProccess update proccess and speed slot
 */
void DoCopyFilesWorker::onUpdateProccess()
{
    // 当前写入文件大小获取
    qint64 writSize = getWriteDataSize();
    writSize += skipWritSize;
    emitProccessChangedNotify(writSize);
    emitSpeedUpdatedNotify(writSize);
}
