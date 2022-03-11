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

#include <dfm-io/dfmio_global.h>
#include <dfm-io/core/diofactory.h>

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

static const quint32 kMaxBufferLength { 1024 * 1024 * 1 };
static const quint32 kBigFileSize { 300 * 1024 * 1024 };

DSC_USE_NAMESPACE
USING_IO_NAMESPACE

DoCopyFilesWorker::DoCopyFilesWorker(QObject *parent)
    : AbstractWorker(parent)
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

    return true;
}

void DoCopyFilesWorker::determineCountProcessType()
{
    // 检查目标文件的有效性
    // 判读目标文件的位置（在可移除设备并且不是ext系列的设备上使用读取写入设备大小，
    // 其他都是读取当前线程写入磁盘的数据，如果采用多线程拷贝就自行统计）

    if (!targetStorageInfo)
        targetStorageInfo.reset(new StorageInfo(targetUrl.path()));

    qDebug("Target block device: \"%s\", Root Path: \"%s\"", targetStorageInfo->device().constData(), qPrintable(targetStorageInfo->rootPath()));

    if (targetStorageInfo->isLocalDevice()) {
        bool isFileSystemTypeExt = targetStorageInfo->fileSystemType().startsWith("ext");
        isTargetFileLocal = FileOperationsUtils::isFileOnDisk(targetUrl);

        if (!isFileSystemTypeExt) {
            const QByteArray dev_path = targetStorageInfo->device();

            QProcess process;
            process.start("lsblk", { "-niro", "MAJ:MIN,HOTPLUG,LOG-SEC", dev_path }, QIODevice::ReadOnly);

            if (process.waitForFinished(3000)) {
                if (process.exitCode() == 0) {
                    const QByteArray &data = process.readAllStandardOutput();
                    const QByteArrayList &list = data.split(' ');

                    qDebug("lsblk result data: \"%s\"", data.constData());

                    if (list.size() == 3) {
                        targetSysDevPath = "/sys/dev/block/" + list.first();
                        targetIsRemovable = list.at(1) == "1";

                        bool ok = false;
                        targetLogSecionSize = static_cast<qint16>(list.at(2).toInt(&ok));

                        if (!ok) {
                            targetLogSecionSize = 512;

                            qWarning() << "get target log secion size failed!";
                        }

                        if (targetIsRemovable) {
                            countWriteType = CountWriteSizeType::kWriteBlockType;
                            targetDeviceStartSectorsWritten = getSectorsWritten();
                        }

                        qDebug("Block device path: \"%s\", Sys dev path: \"%s\", Is removable: %d, Log-Sec: %d",
                               qPrintable(dev_path), qPrintable(targetSysDevPath), bool(targetIsRemovable), targetLogSecionSize);
                    } else {
                        qWarning("Failed on parse the lsblk result data, data: \"%s\"", data.constData());
                    }
                } else {
                    qWarning("Failed on exec lsblk command, exit code: %d, error message: \"%s\"", process.exitCode(), process.readAllStandardError().constData());
                }
            }
        }
        qDebug("targetIsRemovable = %d", bool(targetIsRemovable));
    }
    if (isSourceFileLocal && isTargetFileLocal)
        countWriteType = CountWriteSizeType::kCustomizeType;

    copyTid = (countWriteType == CountWriteSizeType::kTidType) ? syscall(SYS_gettid) : -1;
}

bool DoCopyFilesWorker::copyFiles()
{
    for (const QUrl &url : sourceUrls) {
        if (!stateCheck()) {
            return false;
        }
        const auto &fileInfo = InfoFactory::create<AbstractFileInfo>(url);
        if (!fileInfo) {
            // pause and emit error msg
            AbstractJobHandler::SupportAction action = doHandleErrorAndWait(url, targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
            if (AbstractJobHandler::SupportAction::kSkipAction != action) {
                cancelThreadProcessingError();
                return false;
            } else {
                continue;
            }
        }
        if (!targetInfo) {
            // pause and emit error msg
            doHandleErrorAndWait(QUrl(), targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
            cancelThreadProcessingError();
            return false;
        }
        if (!doCopyFile(fileInfo, targetInfo)) {
            return false;
        }
    }
    return true;
}

bool DoCopyFilesWorker::doCopyFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo)
{
    AbstractFileInfoPointer newTargetInfo(nullptr);
    bool result = false;
    if (!doCheckFile(fromInfo, toInfo, newTargetInfo, result))
        return result;

    bool oldExist = newTargetInfo->exists();
    if (fromInfo->isSymLink()) {
        result = creatSystemLink(fromInfo, newTargetInfo);
    } else if (fromInfo->isDir()) {
        result = checkAndCopyDir(fromInfo, newTargetInfo);
    } else {
        result = checkAndCopyFile(fromInfo, newTargetInfo);
    }

    if (targetInfo == toInfo && !oldExist) {
        completeFiles.append(fromInfo->url());
        precompleteTargetFileInfo.append(newTargetInfo);
    }

    return result;
}

bool DoCopyFilesWorker::doCheckFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, AbstractFileInfoPointer &newTargetInfo, bool &result)
{
    // 检查源文件的文件信息
    if (!fromInfo) {
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(QUrl(), toInfo == nullptr ? QUrl() : toInfo->url(), AbstractJobHandler::JobErrorType::kProrogramError);
        cancelThreadProcessingError();
        result = AbstractJobHandler::SupportAction::kSkipAction != action;
        return false;
    }
    // 检查源文件是否存在
    if (!fromInfo->exists()) {
        AbstractJobHandler::JobErrorType errortype = (fromInfo->path().startsWith("/root/") && !toInfo->path().startsWith("/root/")) ? AbstractJobHandler::JobErrorType::kPermissionError
                                                                                                                                     : AbstractJobHandler::JobErrorType::kNonexistenceError;
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->url(), toInfo == nullptr ? QUrl() : toInfo->url(), errortype);
        cancelThreadProcessingError();

        result = AbstractJobHandler::SupportAction::kSkipAction != action;
        return false;
    }
    // 检查目标文件的文件信息
    if (!toInfo) {
        doHandleErrorAndWait(fromInfo->url(), QUrl(), AbstractJobHandler::JobErrorType::kProrogramError);
        cancelThreadProcessingError();
        return false;
    }
    // 检查目标文件是否存在
    if (!toInfo->exists()) {
        AbstractJobHandler::JobErrorType errortype = (fromInfo->path().startsWith("/root/") && !toInfo->path().startsWith("/root/")) ? AbstractJobHandler::JobErrorType::kPermissionError
                                                                                                                                     : AbstractJobHandler::JobErrorType::kNonexistenceError;
        doHandleErrorAndWait(fromInfo->url(), toInfo->url(), errortype);
        cancelThreadProcessingError();
        return false;
    }
    // 特殊文件判断
    switch (fromInfo->fileType()) {
    case AbstractFileInfo::kCharDevice:
    case AbstractFileInfo::kBlockDevice:
    case AbstractFileInfo::kFIFOFile:
    case AbstractFileInfo::kSocketFile: {
        skipWritSize += fromInfo->size() <= 0 ? dirSize : fromInfo->size();

        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kSpecialFileError);
        cancelThreadProcessingError();
        return AbstractJobHandler::SupportAction::kSkipAction == action;
    }
    default:
        break;
    }

    // 创建新的目标文件并做检查
    QString fileNewName = fromInfo->fileName();
    newTargetInfo.reset(nullptr);
    if (!doCheckNewFile(fromInfo, toInfo, newTargetInfo, fileNewName, result, true))
        return false;

    return true;
}

bool DoCopyFilesWorker::doCheckNewFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, AbstractFileInfoPointer &newTargetInfo, QString &fileNewName, bool &result, bool isCountSize)
{
    fileNewName = formatFileName(fileNewName);
    // 创建文件的名称
    QUrl newTargetUrl = toInfo->url();
    const QString &newTargetPath = newTargetUrl.path();

    QString newPath = newTargetPath.endsWith("/") ? newTargetPath + fileNewName
                                                  : newTargetPath + "/" + fileNewName;

    newTargetUrl.setPath(newPath);

    newTargetInfo.reset(nullptr);
    newTargetInfo = InfoFactory::create<AbstractFileInfo>(newTargetUrl);

    if (!newTargetInfo) {
        skipWritSize += (isCountSize && (fromInfo->isSymLink() || fromInfo->size() <= 0)) ? dirSize
                                                                                          : fromInfo->size();
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kProrogramError);
        cancelThreadProcessingError();
        result = AbstractJobHandler::SupportAction::kSkipAction != action;
        return false;
    }

    if (newTargetInfo->exists()) {
        if (!jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyToSelf) && FileOperationsUtils::isAncestorUrl(fromInfo->url(), newTargetUrl)) {
            AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kTargetIsSelfError);
            cancelThreadProcessingError();
            if (AbstractJobHandler::SupportAction::kSkipAction == action) {
                skipWritSize += isCountSize && (fromInfo->isSymLink() || fromInfo->size() <= 0) ? dirSize : fromInfo->size();
                result = AbstractJobHandler::SupportAction::kSkipAction != action;
                return false;
            }

            if (action != AbstractJobHandler::SupportAction::kEnforceAction) {
                skipWritSize += isCountSize && (fromInfo->isSymLink() || fromInfo->size() <= 0) ? dirSize : fromInfo->size();
                return false;
            }
        };
        bool fromIsFile = fromInfo->isFile() || fromInfo->isSymLink();
        bool newTargetIsFile = newTargetInfo->isFile() || newTargetInfo->isSymLink();
        AbstractJobHandler::JobErrorType errortype = newTargetIsFile ? AbstractJobHandler::JobErrorType::kFileExistsError
                                                                     : AbstractJobHandler::JobErrorType::kDirectoryExistsError;
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->url(), newTargetInfo->url(), errortype);
        switch (action) {
        case AbstractJobHandler::SupportAction::kReplaceAction:
            if (newTargetUrl == fromInfo->url()) {
                skipWritSize += isCountSize && (fromInfo->isSymLink() || fromInfo->size() <= 0) ? dirSize
                                                                                                : fromInfo->size();
                cancelThreadProcessingError();
                return true;
            }

            if (fromIsFile && fromIsFile == newTargetIsFile) {
                break;
            } else {
                // TODO:: something is doing here
                cancelThreadProcessingError();
                return false;
            }
        case AbstractJobHandler::SupportAction::kMergeAction:
            if (!fromIsFile && fromIsFile == newTargetIsFile) {
                break;
            } else {
                // TODO:: something is doing here
                cancelThreadProcessingError();
                return false;
            }
        case AbstractJobHandler::SupportAction::kSkipAction:
            skipWritSize += isCountSize && (fromInfo->isSymLink() || fromInfo->size() <= 0) ? dirSize
                                                                                            : fromInfo->size();
            cancelThreadProcessingError();
            return true;
        case AbstractJobHandler::SupportAction::kCoexistAction: {
            fileNewName = getNonExistFileName(fromInfo, toInfo);
            if (fileNewName.isEmpty()) {
                cancelThreadProcessingError();
                return false;
            }

            bool ok = doCheckNewFile(fromInfo, toInfo, newTargetInfo, fileNewName, result);
            cancelThreadProcessingError();
            return ok;
        }
        default:
            cancelThreadProcessingError();
            return false;
        }
        cancelThreadProcessingError();
        return true;
    }

    return true;
}

bool DoCopyFilesWorker::doCheckFileFreeSpace(const qint64 &size)
{
    if (!targetStorageInfo) {
        targetStorageInfo.reset(new StorageInfo(targetUrl.path()));
    } else {
        targetStorageInfo->refresh();
    }

    if (targetStorageInfo->bytesTotal() <= 0) {
        return false;
    }
    return targetStorageInfo->bytesAvailable() >= size;
}

bool DoCopyFilesWorker::creatSystemLink(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo)
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
                return checkAndCopyFile(fromInfo, toInfo);
            } else {
                return checkAndCopyDir(fromInfo, toInfo);
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
    return actionForlink == AbstractJobHandler::SupportAction::kSkipAction;
}

bool DoCopyFilesWorker::checkAndCopyFile(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

    while (!doCheckFileFreeSpace(fromInfo->size())) {
        action = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kNotEnoughSpaceError);
        if (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction) {
            continue;
        } else if (action == AbstractJobHandler::SupportAction::kSkipAction) {
            skipWritSize += fromInfo->size() <= 0 ? dirSize : fromInfo->size();
            cancelThreadProcessingError();
            return true;
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

    bool ok = doCopyOneFile(fromInfo, toInfo);
    FileOperationsUtils::removeUsingName(toInfo->fileName());
    return ok;
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
    bool ok = doCopyOneFile(threadInfo->fromInfo, threadInfo->toInfo);
    if (!ok)
        setStat(AbstractJobHandler::JobState::kStopState);
    FileOperationsUtils::removeUsingName(threadInfo->toInfo->fileName());
    return ok;
}

bool DoCopyFilesWorker::doCopyOneFile(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo)
{
    // 实现真实文件拷贝
    // do not check the args
    emitCurrentTaskNotify(fromInfo->url(), toInfo->url());
    //预先读取
    readAheadSourceFile(fromInfo);
    // 创建文件的divice
    QSharedPointer<DFile> fromDevice { nullptr }, toDevice { nullptr };
    bool reslut = false;
    if (!createFileDevices(fromInfo, toInfo, fromDevice, toDevice, reslut))
        return reslut;
    // 打开文件并创建
    if (!openFiles(fromInfo, toInfo, fromDevice, toDevice, reslut))
        return reslut;
    // 源文件大小如果为0
    if (fromInfo->size() <= 0) {
        // 对文件加权
        setTargetPermissions(fromInfo, toInfo);
        skipWritSize += dirSize;
        return true;
    }
    // resize target file
    if (jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyResizeDestinationFile) && !resizeTargetFile(fromInfo, toInfo, toDevice, reslut))
        return reslut;
    // 循环读取和写入文件，拷贝
    qint64 blockSize = fromInfo->size() > kMaxBufferLength ? kMaxBufferLength : fromInfo->size();
    char *data = new char[blockSize + 1];
    uLong sourceCheckSum = adler32(0L, nullptr, 0);
    qint64 sizeRead = 0;

    do {
        if (!doReadFile(fromInfo, toInfo, fromDevice, data, blockSize, sizeRead, reslut)) {
            delete[] data;
            data = nullptr;
            return reslut;
        }

        if (!doWriteFile(fromInfo, toInfo, toDevice, data, sizeRead, reslut)) {
            delete[] data;
            data = nullptr;
            return reslut;
        }

        if (Q_LIKELY(jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyIntegrityChecking))) {
            sourceCheckSum = adler32(sourceCheckSum, reinterpret_cast<Bytef *>(data), static_cast<uInt>(sizeRead));
        }
    } while (fromDevice->pos() != fromInfo->size() && toInfo->size() != fromInfo->size());

    delete[] data;
    data = nullptr;

    // 对文件加权
    setTargetPermissions(fromInfo, toInfo);
    if (Q_UNLIKELY(!stateCheck())) {
        return false;
    }

    // 校验文件完整性
    return verifyFileIntegrity(blockSize, sourceCheckSum, fromInfo, toInfo, toDevice);
}

bool DoCopyFilesWorker::createFileDevices(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, QSharedPointer<DFile> &fromeFile, QSharedPointer<DFile> &toFile, bool &result)
{
    if (!createFileDevice(fromInfo, toInfo, fromInfo, fromeFile, result))
        return false;
    if (!createFileDevice(fromInfo, toInfo, toInfo, toFile, result))
        return false;
    return true;
}

bool DoCopyFilesWorker::createFileDevice(const AbstractFileInfoPointer &fromInfo,
                                         const AbstractFileInfoPointer &toInfo,
                                         const AbstractFileInfoPointer &needOpenInfo,
                                         QSharedPointer<DFile> &file,
                                         bool &result)
{
    file.reset(nullptr);
    QUrl url = needOpenInfo->url();
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    QSharedPointer<DIOFactory> factory { nullptr };
    do {
        factory = produceQSharedIOFactory(url.scheme(), static_cast<QUrl>(url));
        if (!factory) {
            action = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kDfmIoError, QObject::tr("create dfm io factory failed!"));
        }
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);
    cancelThreadProcessingError();
    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        skipWritSize += action == AbstractJobHandler::SupportAction::kSkipAction ? (fromInfo->size() <= 0 ? dirSize : fromInfo->size()) : 0;
        result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    do {
        file = factory->createFile();
        if (!file) {
            action = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kDfmIoError, QObject::tr("create dfm io dfile failed!"));
        }
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    cancelThreadProcessingError();
    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        skipWritSize += action == AbstractJobHandler::SupportAction::kSkipAction ? (fromInfo->size() <= 0 ? dirSize : fromInfo->size()) : 0;
        result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    return true;
}

bool DoCopyFilesWorker::openFiles(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, const QSharedPointer<DFile> &fromeFile, const QSharedPointer<DFile> &toFile, bool &result)
{
    if (fromInfo->size() > 0 && !openFile(fromInfo, toInfo, fromeFile, DFile::OpenFlag::ReadOnly, result)) {
        return false;
    }

    if (!openFile(fromInfo, toInfo, toFile, DFile::OpenFlag::WriteOnly | DFile::OpenFlag::Truncate, result)) {
        return false;
    }

    return true;
}

bool DoCopyFilesWorker::openFile(const AbstractFileInfoPointer &fromInfo,
                                 const AbstractFileInfoPointer &toInfo,
                                 const QSharedPointer<DFile> &file,
                                 const DFMIO::DFile::OpenFlags &flags,
                                 bool &result)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    do {
        if (!file->open(flags)) {
            action = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kDfmIoError, QObject::tr("create dfm io dfile failed!"));
        }
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);
    cancelThreadProcessingError();
    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        skipWritSize += action == AbstractJobHandler::SupportAction::kSkipAction ? (fromInfo->size() <= 0 ? dirSize : fromInfo->size()) : 0;
        result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }
    return true;
}

bool DoCopyFilesWorker::resizeTargetFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, const QSharedPointer<DFile> &file, bool &result)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    do {
        if (!file->write(QByteArray())) {
            action = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kResizeError, QObject::tr("resize file failed!"));
        }
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);
    cancelThreadProcessingError();
    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        skipWritSize += action == AbstractJobHandler::SupportAction::kSkipAction ? (fromInfo->size() <= 0 ? dirSize : fromInfo->size()) : 0;
        result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }
    return true;
}

bool DoCopyFilesWorker::doReadFile(const AbstractFileInfoPointer &fromInfo,
                                   const AbstractFileInfoPointer &toInfo,
                                   const QSharedPointer<DFile> &fromDevice,
                                   char *data,
                                   const qint64 &blockSize, qint64 &readSize,
                                   bool &result)
{
    readSize = 0;
    qint64 currentPos = fromDevice->pos();
    AbstractJobHandler::SupportAction actionForRead = AbstractJobHandler::SupportAction::kNoAction;

    if (Q_UNLIKELY(!stateCheck())) {
        return false;
    }
    do {
        readSize = fromDevice->read(data, blockSize);
        if (Q_UNLIKELY(!stateCheck())) {
            return false;
        }

        if (Q_UNLIKELY(readSize <= 0)) {
            const auto &fromInfoSize = fromInfo->size();
            const auto &toInfoSize = toInfo->size();
            const auto &fromDevicePos = fromDevice->pos();
            if (readSize == 0 && (fromInfoSize == toInfoSize || fromInfoSize == fromDevicePos)) {
                return true;
            }
            fromInfo->refresh();
            AbstractJobHandler::JobErrorType errortype = fromInfo->exists() ? AbstractJobHandler::JobErrorType::kReadError : AbstractJobHandler::JobErrorType::kNonexistenceError;
            QString errorstr = fromInfo->exists() ? QString(QObject::tr("DFileCopyMoveJob", "Failed to read the file, cause: %1")).arg("to something!") : QString();

            actionForRead = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), errortype, errorstr);

            if (actionForRead == AbstractJobHandler::SupportAction::kRetryAction) {
                if (!fromDevice->seek(currentPos)) {
                    AbstractJobHandler::SupportAction actionForReadSeek = doHandleErrorAndWait(fromInfo->url(),
                                                                                               toInfo->url(),
                                                                                               AbstractJobHandler::JobErrorType::kSeekError);
                    result = actionForReadSeek == AbstractJobHandler::SupportAction::kSkipAction;
                    skipWritSize += result ? fromInfo->size() - currentPos : 0;
                    cancelThreadProcessingError();
                    return false;
                }
            }
        }
    } while (!isStopped() && actionForRead == AbstractJobHandler::SupportAction::kRetryAction);

    cancelThreadProcessingError();

    if (actionForRead != AbstractJobHandler::SupportAction::kNoAction) {
        result = actionForRead == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    return true;
}

bool DoCopyFilesWorker::doWriteFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, const QSharedPointer<DFile> &toDevice, const char *data, const qint64 &readSize, bool &result)
{
    qint64 currentPos = toDevice->pos();
    AbstractJobHandler::SupportAction actionForWrite { AbstractJobHandler::SupportAction::kNoAction };
    qint64 sizeWrite = 0;
    qint64 surplusSize = readSize;
    do {
        const char *surplusData = data;
        do {
            surplusData += sizeWrite;
            surplusSize -= sizeWrite;
            sizeWrite = toDevice->write(surplusData, surplusSize);
            if (Q_UNLIKELY(!stateCheck()))
                return false;
        } while (sizeWrite > 0 && sizeWrite < surplusSize);

        // 表示全部数据写入完成
        if (sizeWrite >= 0) {
            break;
        }
        QString errorstr = QString(QObject::tr("Failed to write the file, cause: %1")).arg("some thing to do!");

        actionForWrite = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kWriteError, errorstr);
        if (actionForWrite == AbstractJobHandler::SupportAction::kRetryAction) {
            if (!toDevice->seek(currentPos)) {
                AbstractJobHandler::SupportAction actionForWriteSeek = doHandleErrorAndWait(fromInfo->url(),
                                                                                            toInfo->url(),
                                                                                            AbstractJobHandler::JobErrorType::kSeekError);
                result = actionForWriteSeek == AbstractJobHandler::SupportAction::kSkipAction;
                currentWritSize += readSize - surplusSize;
                skipWritSize += result ? fromInfo->size() - (currentPos + readSize - surplusSize) : 0;
                cancelThreadProcessingError();
                return false;
            }
        }
    } while (!isStopped() && actionForWrite == AbstractJobHandler::SupportAction::kRetryAction);

    cancelThreadProcessingError();

    if (actionForWrite != AbstractJobHandler::SupportAction::kNoAction) {
        result = actionForWrite == AbstractJobHandler::SupportAction::kSkipAction;
        currentWritSize += readSize - surplusSize;
        skipWritSize += result ? fromInfo->size() - (currentPos + readSize - surplusSize) : 0;
        return false;
    }

    if (sizeWrite > 0) {
        toDevice->flush();
    }
    currentWritSize += readSize;

    return true;
}

bool DoCopyFilesWorker::checkAndCopyDir(const AbstractFileInfoPointer &fromInfo,
                                        const AbstractFileInfoPointer &toInfo)
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
            return AbstractJobHandler::SupportAction::kSkipAction == action;
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
        if (!doCopyFile(info, toInfo)) {
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
/*!
 * \brief DoCopyFilesWorker::setTargetPermissions Modify the last access time of the target file and add the target file permissions
 * \param fromInfo sourc file information
 * \param toInfo target file information
 */
void DoCopyFilesWorker::setTargetPermissions(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo)
{
    // 修改文件修改时间
    handler->setFileTime(toInfo->url(), fromInfo->lastRead(), fromInfo->lastModified());
    QFileDevice::Permissions permissions = fromInfo->permissions();
    QString path = fromInfo->url().path();
    //权限为0000时，源文件已经被删除，无需修改新建的文件的权限为0000
    if (permissions != 0000)
        handler->setPermissions(toInfo->url(), permissions);
}
/*!
 * \brief DoCopyFilesWorker::verifyFileIntegrity Verify the integrity of the target file
 * \param blockSize read buffer size
 * \param sourceCheckSum source file sum
 * \param fromInfo sourc file information
 * \param toInfo target file information
 * \param toDevice target file device
 * \return the reslut
 */
bool DoCopyFilesWorker::verifyFileIntegrity(const qint64 &blockSize,
                                            const ulong &sourceCheckSum,
                                            const AbstractFileInfoPointer &fromInfo,
                                            const AbstractFileInfoPointer &toInfo,
                                            QSharedPointer<DFile> &toDevice)
{
    if (!jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyIntegrityChecking))
        return true;
    char *data = new char[blockSize + 1];
    QTime t;
    ulong targetCheckSum = adler32(0L, nullptr, 0);
    Q_FOREVER {
        qint64 size = toDevice->read(data, blockSize);

        if (Q_UNLIKELY(size <= 0)) {
            if (size == 0 && toInfo->size() == toDevice->pos()) {
                break;
            }

            QString errorstr = QObject::tr("File integrity was damaged, cause: %1").arg("some error occ!");
            AbstractJobHandler::SupportAction actionForCheckRead = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kIntegrityCheckingError, errorstr);
            if (!isStopped() && AbstractJobHandler::SupportAction::kRetryAction == actionForCheckRead) {
                continue;
            } else {
                cancelThreadProcessingError();
                return actionForCheckRead == AbstractJobHandler::SupportAction::kSkipAction;
            }
        }

        cancelThreadProcessingError();

        targetCheckSum = adler32(targetCheckSum, reinterpret_cast<Bytef *>(data), static_cast<uInt>(size));

        if (Q_UNLIKELY(!stateCheck())) {
            delete[] data;
            data = nullptr;
            return false;
        }
    }
    delete[] data;

    qDebug("Time spent of integrity check of the file: %d", t.elapsed());

    if (sourceCheckSum != targetCheckSum) {
        qWarning("Failed on file integrity checking, source file: 0x%lx, target file: 0x%lx", sourceCheckSum, targetCheckSum);
        QString errorstr = QObject::tr("File integrity was damaged, cause: %1").arg("some error occ!");
        AbstractJobHandler::SupportAction actionForCheck = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kIntegrityCheckingError, errorstr);
        cancelThreadProcessingError();
        return actionForCheck == AbstractJobHandler::SupportAction::kSkipAction;
    }
    return true;
}

void DoCopyFilesWorker::setAllDirPermisson()
{
    for (auto info : dirPermissonList.list()) {
        handler->setPermissions(info->target, info->permission);
    }
}
/*!
 * \brief DoCopyFilesWorker::syncFilesToDevice Synchronize files to device
 * \return
 */
void DoCopyFilesWorker::syncFilesToDevice()
{
    if (CountWriteSizeType::kWriteBlockType != countWriteType)
        return;

    qint64 writeSize = getWriteDataSize() + skipWritSize;
    while (!isStopped() && sourceFilesTotalSize > 0 && writeSize < sourceFilesTotalSize) {
        QThread::msleep(100);
    }
}
/*!
 * \brief DoCopyFilesWorker::endCopy  Clean up the current copy task and send the copy task completion signal
 */
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
/*!
 * \brief DoCopyFilesWorker::emitSpeedUpdatedNotify send  speedUpdatedNotify signal
 * \param writSize current write data size
 */
void DoCopyFilesWorker::emitSpeedUpdatedNotify(const qint64 &writSize)
{
    JobInfoPointer info(new QMap<quint8, QVariant>);
    qint64 speed = writSize * 1000 / (time.elapsed() == 0 ? 1 : time.elapsed());
    info->insert(AbstractJobHandler::NotifyInfoKey::kSpeedKey, QVariant::fromValue(speed));
    info->insert(AbstractJobHandler::NotifyInfoKey::kRemindTimeKey, QVariant::fromValue(speed == 0 ? 0 : (sourceFilesTotalSize - writSize) / speed));

    emit stateChangedNotify(info);
}
/*!
 * \brief DoCopyFilesWorker::doHandleErrorAndWait Blocking handles errors and returns
 * actions supported by the operation
 * 注意：处理这个错误时，其他线程都要阻塞，这个错误处理完成了才能
 * \param from source information
 * \param to target information
 * \param error error type
 * \param needRetry is neef retry action
 * \param errorMsg error message
 * \return support action
 */
AbstractJobHandler::SupportAction DoCopyFilesWorker::doHandleErrorAndWait(const QUrl &from, const QUrl &to,
                                                                          const AbstractJobHandler::JobErrorType &error,
                                                                          const QString &errorMsg)
{
    setStat(AbstractJobHandler::JobState::kPauseState);

    // 判断是否有线程在处理错误,1.无，当前线程加入到处理队列，阻塞其他线程的错误处理，发送错误信号，阻塞自己。收到错误处理，恢复自己。
    // 判读是否有retry操作,当前错误处理线程就切换（错误处理线程不出队列），继续处理，外部判读处理完成就切换处理线程
    // 当前处理线程为队列的第一个对象
    errorThreadIdQueueMutex.lock();

    if (from == to) {
        currentAction = AbstractJobHandler::SupportAction::kCoexistAction;
        setStat(AbstractJobHandler::JobState::kRunningState);
        errorThreadIdQueueMutex.unlock();
        return currentAction;
    }

    errorThreadIdQueue.clear();
    errorThreadIdQueue.enqueue(QThread::currentThreadId());

    // 阻塞其他线程 当前不是停止状态，并且当前线程不是处理错误线程
    while (!isStopped() && errorThreadIdQueue.first() != QThread::currentThreadId()) {
        errorThreadIdQueueMutex.unlock();
        QMutex lock;
        errorCondition.wait(&lock);
        lock.unlock();
        errorThreadIdQueueMutex.lock();
    }

    errorThreadIdQueueMutex.unlock();
    if (isStopped())
        return AbstractJobHandler::SupportAction::kCancelAction;
    // 发送错误处理 阻塞自己
    emitErrorNotify(from, to, error, AbstractJobHandler::errorToString(error) + errorMsg);
    QMutex lock;
    handlingErrorCondition.wait(&lock);
    lock.unlock();
    if (isStopped())
        return AbstractJobHandler::SupportAction::kCancelAction;

    return currentAction;
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
 * \brief DoCopyFilesWorker::getWriteDataSize Gets the size of the data being written
 * \return write data size
 */
qint64 DoCopyFilesWorker::getWriteDataSize()
{
    if (CountWriteSizeType::kTidType == countWriteType) {
        return getTidWriteSize();
    } else if (CountWriteSizeType::kCustomizeType == countWriteType) {
        return currentWritSize;
    }

    if (targetDeviceStartSectorsWritten >= 0) {
        if ((getSectorsWritten() == 0) && (targetDeviceStartSectorsWritten > 0)) {
            return 0;
        } else {
            return (getSectorsWritten() - targetDeviceStartSectorsWritten) * targetLogSecionSize;
        }
    }

    return currentWritSize;
}
/*!
 * \brief DoCopyFilesWorker::getTidWriteSize Get the write data size by using the thread ID
 * \return write data size
 */
qint64 DoCopyFilesWorker::getTidWriteSize()
{
    QFile file(QStringLiteral("/proc/self/task/%1/io").arg(copyTid));

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed on open the" << file.fileName() << ", will be not update the job speed and progress";

        return 0;
    }

    const QByteArray &line_head = QByteArrayLiteral("write_bytes: ");
    const QByteArray &all_data = file.readAll();

    file.close();

    QTextStream text_stream(all_data);

    while (!text_stream.atEnd()) {
        const QByteArray &line = text_stream.readLine().toLatin1();

        if (line.startsWith(line_head)) {
            bool ok = false;
            qint64 size = line.mid(line_head.size()).toLongLong(&ok);

            if (!ok) {
                qWarning() << "Failed to convert to qint64, line string=" << line;

                return 0;
            }
            return size;
        }
    }

    qWarning() << "Failed to find \"" << line_head << "\" from the" << file.fileName();

    return 0;
}
/*!
 * \brief DoCopyFilesWorker::getSectorsWritten Get write Sector or writ block
 * \return write Sector or writ block
 */
qint64 DoCopyFilesWorker::getSectorsWritten()
{
    QByteArray data;
    QFile file(targetSysDevPath + "/stat");

    if (file.open(QIODevice::ReadOnly)) {
        data = file.readAll();
        file.close();
        return data.simplified().split(' ').value(6).toLongLong();
    } else {
        return 0;
    }
}
/*!
 * \brief DoCopyFilesWorker::readAheadSourceFile Read ahead file
 * \param fileInfo file information
 */
void DoCopyFilesWorker::readAheadSourceFile(const AbstractFileInfoPointer &fileInfo)
{
    if (fileInfo->size() <= 0)
        return;
    std::string stdStr = fileInfo->url().path().toUtf8().toStdString();
    int fromfd = open(stdStr.data(), O_RDONLY);
    if (-1 != fromfd) {
        readahead(fromfd, 0, static_cast<size_t>(fileInfo->size()));
        close(fromfd);
    }
}
/*!
 * \brief DoCopyFilesWorker::cancelThreadProcessingError
 * 注意：当前线程处理结束，转到其他线程处理错误
 */
void DoCopyFilesWorker::cancelThreadProcessingError()
{
    // 当前错误线程处理结束
    if (errorThreadIdQueue.isEmpty())
        return;

    QMutexLocker lk(&errorThreadIdQueueMutex);

    errorThreadIdQueue.removeOne(QThread::currentThreadId());
    if (errorThreadIdQueue.count() <= 0)
        resume();

    errorCondition.wakeAll();
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
