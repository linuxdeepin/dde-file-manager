/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             liyigang<liyigang@uniontech.com>
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
#include "fileoperatebaseworker.h"
#include "fileoperations/fileoperationutils/fileoperationsutils.h"
#include "services/common/fileoperations/copyfiles/storageinfo.h"

#include "dfm-base/interfaces/abstractdiriterator.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/decorator/decoratorfileenumerator.h"
#include "dfm-base/utils/decorator/decoratorfileinfo.h"

#include <dfm-io/core/diofactory.h>
#include <dfm-io/dfmio_register.h>

#include <QMutex>
#include <QWaitCondition>
#include <QDateTime>
#include <QApplication>
#include <QProcess>
#include <QtConcurrent>

#include <fcntl.h>
#include <unistd.h>
#include <zlib.h>
#include <syscall.h>
#include <sys/stat.h>

static const quint32 kMaxBufferLength { 1024 * 1024 * 1 };
constexpr uint32_t kBigFileSize { 300 * 1024 * 1024 };

DSC_USE_NAMESPACE
USING_IO_NAMESPACE

FileOperateBaseWorker::FileOperateBaseWorker(QObject *parent)
    : AbstractWorker(parent)
{
}

FileOperateBaseWorker::~FileOperateBaseWorker()
{
}
/*!
 * \brief FileOperateBaseWorker::doHandleErrorAndWait Handle the error and block waiting for the error handling operation to return
 * \param fromUrl URL of the source file
 * \param toUrl Destination URL
 * \param errorUrl URL of the error file
 * \param error Wrong type
 * \param errorMsg Wrong message
 * \return AbstractJobHandler::SupportAction Current processing operation
 */
AbstractJobHandler::SupportAction FileOperateBaseWorker::doHandleErrorAndWait(const QUrl &urlFrom, const QUrl &urlTo,
                                                                              const AbstractJobHandler::JobErrorType &error,
                                                                              const QString &errorMsg)
{
    setStat(AbstractJobHandler::JobState::kPauseState);

    // 判断是否有线程在处理错误,1.无，当前线程加入到处理队列，阻塞其他线程的错误处理，发送错误信号，阻塞自己。收到错误处理，恢复自己。
    // 判读是否有retry操作,当前错误处理线程就切换（错误处理线程不出队列），继续处理，外部判读处理完成就切换处理线程
    // 当前处理线程为队列的第一个对象
    errorThreadIdQueueMutex.lock();

    if (urlFrom == urlTo) {
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

    if (rememberSelect.load() && currentAction != AbstractJobHandler::SupportAction::kNoAction)
        return currentAction;

    // 发送错误处理 阻塞自己
    const QString &errorMsgAll = errorMsg.isEmpty() ? AbstractJobHandler::errorToString(error) : (AbstractJobHandler::errorToString(error) + ": " + errorMsg);
    emitErrorNotify(urlFrom, urlTo, error, errorMsgAll);
    QMutex lock;
    handlingErrorCondition.wait(&lock);
    lock.unlock();
    if (isStopped())
        return AbstractJobHandler::SupportAction::kCancelAction;

    if (currentAction == AbstractJobHandler::SupportAction::kSkipAction)
        setStat(AbstractJobHandler::JobState::kRunningState);
    return currentAction;
}

void FileOperateBaseWorker::emitSpeedUpdatedNotify(const qint64 &writSize)
{
    JobInfoPointer info(new QMap<quint8, QVariant>);
    qint64 speed = writSize * 1000 / (time.elapsed() == 0 ? 1 : time.elapsed());
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(jobType));
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobStateKey, QVariant::fromValue(currentState));
    info->insert(AbstractJobHandler::NotifyInfoKey::kSpeedKey, QVariant::fromValue(speed));
    info->insert(AbstractJobHandler::NotifyInfoKey::kRemindTimeKey, QVariant::fromValue(speed == 0 ? 0 : (sourceFilesTotalSize - writSize) / speed));

    emit stateChangedNotify(info);
    emit speedUpdatedNotify(info);
}

/*!
 * \brief FileOperateBaseWorker::createFileDevice Device to create the file
 * \param fromUrl URL of the source file
 * \param toUrl Destination URL
 * \param needOpenInfo file information
 * \param file fromeFile Output parameter: file device
 * \param result result result Output parameter: whether skip
 * \return Is the device of the file created successfully
 */
bool FileOperateBaseWorker::createFileDevice(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                             const AbstractFileInfoPointer &needOpenInfo, QSharedPointer<DFMIO::DFile> &file,
                                             bool *result)
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
        *result = action == AbstractJobHandler::SupportAction::kSkipAction;
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
        *result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    return true;
}

/*!
 * \brief FileOperateBaseWorker::createFileDevices Device for creating source and directory files
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 * \param fromeFile Output parameter: device of source file
 * \param toFile Output parameter: device of target file
 * \param result result Output parameter: whether skip
 * \return Whether the device of source file and target file is created successfully
 */
bool FileOperateBaseWorker::createFileDevices(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                              QSharedPointer<DFile> &fromeFile, QSharedPointer<DFile> &toFile, bool *result)
{
    if (!createFileDevice(fromInfo, toInfo, fromInfo, fromeFile, result))
        return false;
    if (!createFileDevice(fromInfo, toInfo, toInfo, toFile, result))
        return false;
    return true;
}

/*!
 * \brief FileOperateBaseWorker::openFiles Open source and destination files
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 * \param fromeFile device of source file
 * \param toFile device of target file
 * \param result result Output parameter: whether skip
 * \return Open source and target files successfully
 */
bool FileOperateBaseWorker::openFiles(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                      const QSharedPointer<DFMIO::DFile> &fromeFile, const QSharedPointer<DFMIO::DFile> &toFile,
                                      bool *result)
{
    if (fromInfo->size() > 0 && !openFile(fromInfo, toInfo, fromeFile, DFile::OpenFlag::kReadOnly, result)) {
        return false;
    }

    if (!openFile(fromInfo, toInfo, toFile, DFile::OpenFlag::kWriteOnly | DFile::OpenFlag::kTruncate, result)) {
        return false;
    }

    return true;
}

/*!
 * \brief FileOperateBaseWorker::openFile
 * \param fromUrl URL of the source file
 * \param toUrl Destination URL
 * \param fileInfo file information
 * \param file file deivce
 * \param flags Flag for opening file
 * \param result result Output parameter: whether skip
 * \return wether open the file successfully
 */
bool FileOperateBaseWorker::openFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                     const QSharedPointer<DFMIO::DFile> &file, const DFMIO::DFile::OpenFlags &flags,
                                     bool *result)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    do {
        if (!file->open(flags)) {
            auto lastError = file->lastError();
            qWarning() << "file open error, url from: " << fromInfo->url() << " url to: " << toInfo->url() << " open flag: " << flags << " error code: " << lastError.code() << " error msg: " << lastError.errorMsg();

            action = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kOpenError, lastError.errorMsg());
        }
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);
    cancelThreadProcessingError();
    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        skipWritSize += action == AbstractJobHandler::SupportAction::kSkipAction ? (fromInfo->size() <= 0 ? dirSize : fromInfo->size()) : 0;
        *result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return *result;
    }
    return true;
}

/*!
 * \brief FileOperateBaseWorker::setTargetPermissions Set permissions on the target file
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 */
void FileOperateBaseWorker::setTargetPermissions(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo)
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
 * \brief FileOperateBaseWorker::doReadFile  Read file contents
 * \param fromUrl URL of the source file
 * \param toUrl Destination URL
 * \param fileInfo file information
 * \param fromDevice file device
 * \param data Data buffer
 * \param blockSize Data buffer size
 * \param readSize Read size
 * \param result result Output parameter: whether skip
 * \return Read successfully
 */
bool FileOperateBaseWorker::doReadFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                       const QSharedPointer<DFile> &fromDevice,
                                       char *data, const qint64 &blockSize,
                                       qint64 &readSize, bool *result)
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

            const qint64 fromFilePos = fromDevice->pos();
            const qint64 fromFileInfoSize = fromInfo->size();
            if (readSize == 0 && fromFilePos == fromFileInfoSize) {
                return true;
            }

            qWarning() << "read size <=0, size: " << readSize << " from file pos: " << fromFilePos << " from file info size: " << fromFileInfoSize;

            AbstractJobHandler::JobErrorType errortype = fromInfo->exists() ? AbstractJobHandler::JobErrorType::kReadError : AbstractJobHandler::JobErrorType::kNonexistenceError;
            QString errorstr = fromInfo->exists() ? QString(QObject::tr("DFileCopyMoveJob", "Failed to read the file, cause: %1")).arg("to something!") : QString();

            actionForRead = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), errortype, errorstr);

            if (actionForRead == AbstractJobHandler::SupportAction::kRetryAction) {
                if (!fromDevice->seek(currentPos)) {
                    AbstractJobHandler::SupportAction actionForReadSeek = doHandleErrorAndWait(fromInfo->url(),
                                                                                               toInfo->url(),
                                                                                               AbstractJobHandler::JobErrorType::kSeekError);
                    *result = actionForReadSeek == AbstractJobHandler::SupportAction::kSkipAction;
                    skipWritSize += result ? fromInfo->size() - currentPos : 0;
                    cancelThreadProcessingError();
                    return false;
                }
            }
        }
    } while (!isStopped() && actionForRead == AbstractJobHandler::SupportAction::kRetryAction);

    cancelThreadProcessingError();

    if (actionForRead != AbstractJobHandler::SupportAction::kNoAction) {
        *result = actionForRead == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    return true;
}

/*!
 * \brief FileOperateBaseWorker::doWriteFile  Write file contents
 * \param fromUrl URL of the source file
 * \param toUrl Destination URL
 * \param fileInfo file information
 * \param fromDevice file device
 * \param data Data buffer
 * \param blockSize Data buffer size
 * \param readSize Write size
 * \param result result Output parameter: whether skip
 * \return Write successfully
 */
bool FileOperateBaseWorker::doWriteFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                        const QSharedPointer<DFile> &toDevice,
                                        const char *data, const qint64 &readSize, bool *result)
{
    qint64 currentPos = toDevice->pos();
    AbstractJobHandler::SupportAction actionForWrite { AbstractJobHandler::SupportAction::kNoAction };
    qint64 sizeWrite = 0;
    qint64 surplusSize = readSize;
    do {
        bool writeFinishedOnce = true;
        const char *surplusData = data;
        do {
            if (!writeFinishedOnce)
                qDebug() << "write not finished once, current write size: " << sizeWrite << " remain size: " << surplusSize - sizeWrite << " read size: " << readSize;
            surplusData += sizeWrite;
            surplusSize -= sizeWrite;
            sizeWrite = toDevice->write(surplusData, surplusSize);
            if (Q_UNLIKELY(!stateCheck()))
                return false;
            writeFinishedOnce = false;
        } while (sizeWrite > 0 && sizeWrite < surplusSize);

        // 表示全部数据写入完成
        if (sizeWrite >= 0)
            break;
        if (sizeWrite == -1 && toDevice->lastError().code() == DFMIOErrorCode::DFM_IO_ERROR_NONE) {
            qWarning() << "write failed, but no error, maybe write empty";
            break;
        }

        QString errorStr = QString(QObject::tr("Failed to write the file, cause: %1")).arg(toDevice->lastError().errorMsg());

        actionForWrite = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kWriteError, errorStr);
        if (actionForWrite == AbstractJobHandler::SupportAction::kRetryAction) {
            if (!toDevice->seek(currentPos)) {
                AbstractJobHandler::SupportAction actionForWriteSeek = doHandleErrorAndWait(fromInfo->url(), toInfo->url(),
                                                                                            AbstractJobHandler::JobErrorType::kSeekError);
                *result = actionForWriteSeek == AbstractJobHandler::SupportAction::kSkipAction;
                currentWritSize += readSize - surplusSize;
                skipWritSize += result ? fromInfo->size() - (currentPos + readSize - surplusSize) : 0;
                cancelThreadProcessingError();
                return false;
            }
        }
    } while (!isStopped() && actionForWrite == AbstractJobHandler::SupportAction::kRetryAction);

    cancelThreadProcessingError();

    if (actionForWrite != AbstractJobHandler::SupportAction::kNoAction) {
        *result = actionForWrite == AbstractJobHandler::SupportAction::kSkipAction;
        currentWritSize += readSize - surplusSize;
        skipWritSize += result ? fromInfo->size() - (currentPos + readSize - surplusSize) : 0;
        return false;
    }

    if (needSyncEveryRW && sizeWrite > 0) {
        if (isFsTypeVfat) {
            // FAT and VFAT file systems ignore the "sync" option
            toDevice->close();
            if (!openFile(fromInfo, toInfo, toDevice, DFile::OpenFlag::kWriteOnly | DFile::OpenFlag::kAppend, result)) {
                return false;
            }
        } else {
            toDevice->flush();
        }
    }
    currentWritSize += readSize;

    return true;
}

/*!
 * \brief FileOperateBaseWorker::readAheadSourceFile Pre read source file content
 * \param fileInfo File information of source file
 */
void FileOperateBaseWorker::readAheadSourceFile(const AbstractFileInfoPointer &fileInfo)
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
 * \brief FileOperateBaseWorker::checkDiskSpaceAvailable Check
 * whether the disk where the recycle bin directory is located
 * has space of the size of the source file
 * \param fromUrl URL of the source file
 * \param toUrl Destination URL
 * \param file URL of the source file
 * \param result Output parameter: whether skip
 * \return Is space available
 */
bool FileOperateBaseWorker::checkDiskSpaceAvailable(const QUrl &fromUrl,
                                                    const QUrl &toUrl,
                                                    QSharedPointer<StorageInfo> targetStorageInfo,
                                                    bool *result)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

    do {
        targetStorageInfo->refresh();
        qint64 freeBytes = targetStorageInfo->bytesFree();

        if (FileOperationsUtils::isFilesSizeOutLimit(fromUrl, freeBytes))
            action = doHandleErrorAndWait(fromUrl, toUrl, AbstractJobHandler::JobErrorType::kNotEnoughSpaceError);
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        *result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    return true;
}

/*!
 * \brief FileOperateBaseWorker::deleteFile Delete file
 * \param fromUrl URL of the source file
 * \param toUrl Destination URL
 * \param fileInfo delete file information
 * \return Delete file successfully
 */
bool FileOperateBaseWorker::deleteFile(const QUrl &fromUrl, bool *result)
{
    if (!stateCheck())
        return false;

    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    do {
        if (!handler->deleteFile(fromUrl)) {
            action = doHandleErrorAndWait(fromUrl, QUrl(), AbstractJobHandler::JobErrorType::kDeleteFileError, handler->errorString());
        }
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    *result = action == AbstractJobHandler::SupportAction::kSkipAction
            || action == AbstractJobHandler::SupportAction::kNoAction;

    return result;
}

bool FileOperateBaseWorker::deleteDir(const QUrl &fromUrl, bool *result)
{
    DecoratorFileEnumerator enumerator(fromUrl);
    if (!enumerator.isValid())
        return false;

    bool succ = false;
    while (enumerator.hasNext()) {
        const QString &path = enumerator.next();

        const QUrl &urlNext = QUrl::fromLocalFile(path);
        if (DecoratorFileInfo(urlNext).isDir()) {
            succ = deleteDir(urlNext, result);
        } else {
            succ = deleteFile(urlNext, result);
        }
    }
    succ = deleteFile(fromUrl, result);
    return succ;
}
/*!
 * \brief FileOperateBaseWorker::doCopyFile Copy to a new file and delete the source file
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 * \param result Output parameter: whether skip
 * \return Is the copy successful
 */
bool FileOperateBaseWorker::copyAndDeleteFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *result)
{
    AbstractFileInfoPointer newTargetInfo(nullptr);
    bool ok = false;
    if (!doCheckFile(fromInfo, toInfo, fromInfo->fileName(), newTargetInfo, result))
        return ok;

    bool oldExist = newTargetInfo->exists();

    if (fromInfo->isSymLink()) {
        ok = createSystemLink(fromInfo, newTargetInfo, jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyFollowSymlink), true, result);
        if (ok)
            ok = deleteFile(fromInfo->url(), result);
    } else if (fromInfo->isDir()) {
        ok = checkAndCopyDir(fromInfo, newTargetInfo, result);
        if (ok)
            ok = deleteDir(fromInfo->url(), result);
    } else {
        const QUrl &url = newTargetInfo->url();

        FileUtils::cacheCopyingFileUrl(url);
        ok = doCopyFilePractically(fromInfo, newTargetInfo, result);
        if (ok)
            ok = deleteFile(fromInfo->url(), result);
        FileUtils::removeCopyingFileUrl(url);
    }

    if (!isConvert && !oldExist && newTargetInfo->exists() && targetInfo == toInfo) {
        completeSourceFiles.append(fromInfo->url());
        completeTargetFiles.append(newTargetInfo->url());
    }

    toInfo->refresh();

    return ok;
}
/*!
 * \brief FileOperateBaseWorker::doCheckFile Check and get the name of the target file
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 * \param newTargetInfo Output parameter: information of new file
 * \param result Output parameter: whether skip
 * \return Is it successful
 */
bool FileOperateBaseWorker::doCheckFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, const QString &fileName,
                                        AbstractFileInfoPointer &newTargetInfo, bool *workContinue)
{
    // 检查源文件的文件信息
    if (!fromInfo) {
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(QUrl(), toInfo == nullptr ? QUrl() : toInfo->url(), AbstractJobHandler::JobErrorType::kProrogramError);
        cancelThreadProcessingError();
        *workContinue = AbstractJobHandler::SupportAction::kSkipAction == action;
        return false;
    }
    // 检查源文件是否存在
    if (!fromInfo->exists()) {
        AbstractJobHandler::JobErrorType errortype = (fromInfo->path().startsWith("/root/") && !toInfo->path().startsWith("/root/")) ? AbstractJobHandler::JobErrorType::kPermissionError
                                                                                                                                     : AbstractJobHandler::JobErrorType::kNonexistenceError;
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->url(), toInfo == nullptr ? QUrl() : toInfo->url(), errortype);
        cancelThreadProcessingError();

        *workContinue = AbstractJobHandler::SupportAction::kSkipAction == action;
        return false;
    }
    // 检查目标文件的文件信息
    if (!toInfo) {
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->url(), QUrl(), AbstractJobHandler::JobErrorType::kProrogramError);
        cancelThreadProcessingError();
        *workContinue = AbstractJobHandler::SupportAction::kSkipAction == action;
        return false;
    }
    // 检查目标文件是否存在
    if (!toInfo->exists()) {
        AbstractJobHandler::JobErrorType errortype = (fromInfo->path().startsWith("/root/") && !toInfo->path().startsWith("/root/")) ? AbstractJobHandler::JobErrorType::kPermissionError
                                                                                                                                     : AbstractJobHandler::JobErrorType::kNonexistenceError;
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), errortype);
        cancelThreadProcessingError();
        *workContinue = AbstractJobHandler::SupportAction::kSkipAction == action;
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
        *workContinue = AbstractJobHandler::SupportAction::kSkipAction == action;
        return false;
    }
    default:
        break;
    }

    // 创建新的目标文件并做检查
    QString fileNewName = fileName;
    newTargetInfo.reset(nullptr);
    if (!doCheckNewFile(fromInfo, toInfo, newTargetInfo, fileNewName, workContinue, true))
        return false;

    return true;
}
/*!
 * \brief FileOperateBaseWorker::creatSystemLink Create system link file
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 * \param result Output parameter: whether skip
 * \return Was the linked file created successfully
 */
bool FileOperateBaseWorker::createSystemLink(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                             const bool followLink, const bool doCopy, bool *result)
{
    // 创建链接文件
    skipWritSize += dirSize;
    AbstractFileInfoPointer newFromInfo = fromInfo;
    if (followLink) {
        do {
            QUrl newUrl = newFromInfo->url();
            newUrl.setPath(newFromInfo->symLinkTarget());
            const AbstractFileInfoPointer &symlinkTarget = InfoFactory::create<AbstractFileInfo>(newUrl);

            if (!symlinkTarget || !symlinkTarget->exists()) {
                break;
            }

            newFromInfo = symlinkTarget;
        } while (newFromInfo->isSymLink());

        if (newFromInfo->exists() && doCopy) {
            // copy file here
            if (fromInfo->isFile()) {
                return checkAndCopyFile(fromInfo, toInfo, result);
            } else {
                return checkAndCopyDir(fromInfo, toInfo, result);
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
    *result = actionForlink == AbstractJobHandler::SupportAction::kSkipAction;
    return false;
}
/*!
 * \brief FileOperateBaseWorker::doCheckNewFile Check the legitimacy of the new file
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 * \param newTargetInfo Output parameter: information of new file
 * \param fileNewName File name
 * \param result Output parameter: whether skip
 * \return Is it successful
 */
bool FileOperateBaseWorker::doCheckNewFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                           AbstractFileInfoPointer &newTargetInfo, QString &fileNewName, bool *workContinue, bool isCountSize)
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
        *workContinue = AbstractJobHandler::SupportAction::kSkipAction == action;
        return false;
    }

    if (newTargetInfo->exists()) {
        if (!jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyToSelf) && FileOperationsUtils::isAncestorUrl(fromInfo->url(), newTargetUrl)) {
            AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), AbstractJobHandler::JobErrorType::kTargetIsSelfError);
            cancelThreadProcessingError();
            if (AbstractJobHandler::SupportAction::kSkipAction == action) {
                skipWritSize += isCountSize && (fromInfo->isSymLink() || fromInfo->size() <= 0) ? dirSize : fromInfo->size();
                *workContinue = AbstractJobHandler::SupportAction::kSkipAction == action;
                return false;
            }

            if (action != AbstractJobHandler::SupportAction::kEnforceAction) {
                skipWritSize += isCountSize && (fromInfo->isSymLink() || fromInfo->size() <= 0) ? dirSize : fromInfo->size();
                *workContinue = AbstractJobHandler::SupportAction::kSkipAction == action;
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
            if (newTargetInfo->isSymLink()) {
                LocalFileHandler handler;
                if (!handler.deleteFile(newTargetInfo->url()))
                    return false;
            }

            if (newTargetUrl == fromInfo->url()) {
                skipWritSize += (isCountSize && (fromInfo->isSymLink() || fromInfo->size() <= 0)) ? dirSize
                                                                                                  : fromInfo->size();
                cancelThreadProcessingError();
                return true;
            }

            if (fromIsFile && newTargetIsFile) {
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
            *workContinue = AbstractJobHandler::SupportAction::kSkipAction == action;
            return false;
        case AbstractJobHandler::SupportAction::kCoexistAction: {

            auto nameCheckFunc = [](const QString &name) -> bool {
                return FileOperationsUtils::fileNameUsing.contains(name);
            };
            fileNewName = FileUtils::nonExistFileName(newTargetInfo, toInfo, nameCheckFunc);
            FileOperationsUtils::addUsingName(fileNewName);

            if (fileNewName.isEmpty()) {
                cancelThreadProcessingError();
                return false;
            }

            bool ok = doCheckNewFile(fromInfo, toInfo, newTargetInfo, fileNewName, workContinue);
            cancelThreadProcessingError();
            return ok;
        }
        case AbstractJobHandler::SupportAction::kCancelAction: {
            stopWork.store(true);
            cancelThreadProcessingError();
            return false;
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

bool FileOperateBaseWorker::doCheckFileFreeSpace(const qint64 &size)
{
    if (!targetStorageInfo) {
        targetStorageInfo.reset(new StorageInfo(targetUrl.path()));
    } else {
        targetStorageInfo->refresh();
    }

    const qint64 sizeTotal = targetStorageInfo->bytesTotal();
    if (sizeTotal <= 0) {
        qWarning() << "get bytesTotal failed, size: " << sizeTotal;
        return true;   // invalid size, maybe can not read
    }
    return targetStorageInfo->bytesAvailable() >= size;
}

bool FileOperateBaseWorker::checkAndCopyFile(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo, bool *workContinue)
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
        qDebug() << "use pool copy, url from: " << fromInfo->url() << " url to: " << toInfo->url();

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
        QtConcurrent::run(threadPool.data(), this, static_cast<bool (FileOperateBaseWorker::*)()>(&FileOperateBaseWorker::doThreadPoolCopyFile));
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

bool FileOperateBaseWorker::checkAndCopyDir(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *workContinue)
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

bool FileOperateBaseWorker::doThreadPoolCopyFile()
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

bool FileOperateBaseWorker::doCopyFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *workContinue)
{
    AbstractFileInfoPointer newTargetInfo(nullptr);
    bool result = false;
    if (!doCheckFile(fromInfo, toInfo, fromInfo->fileName(), newTargetInfo, workContinue))
        return result;

    bool oldExist = newTargetInfo->exists();
    if (fromInfo->isSymLink()) {
        result = createSystemLink(fromInfo, newTargetInfo, jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyFollowSymlink), true, workContinue);
    } else if (fromInfo->isDir()) {
        result = checkAndCopyDir(fromInfo, newTargetInfo, workContinue);
        if (result || workContinue)
            currentWritSize += FileUtils::getMemoryPageSize();
    } else {
        result = checkAndCopyFile(fromInfo, newTargetInfo, workContinue);
    }

    if (targetInfo == toInfo && !oldExist) {
        completeSourceFiles.append(fromInfo->url());
        precompleteTargetFileInfo.append(newTargetInfo);
    }

    return result;
}

bool FileOperateBaseWorker::doCopyFilePractically(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo, bool *result)
{
    // 实现真实文件拷贝
    // do not check the args
    emitCurrentTaskNotify(fromInfo->url(), toInfo->url());
    //预先读取
    readAheadSourceFile(fromInfo);
    // 创建文件的divice
    QSharedPointer<DFile> fromDevice { nullptr }, toDevice { nullptr };
    if (!createFileDevices(fromInfo, toInfo, fromDevice, toDevice, result))
        return *result;
    // 打开文件并创建
    if (!openFiles(fromInfo, toInfo, fromDevice, toDevice, result))
        return *result;
    // 源文件大小如果为0
    if (fromInfo->size() <= 0) {
        // 对文件加权
        setTargetPermissions(fromInfo, toInfo);
        skipWritSize += dirSize;
        FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::Global::FileNotifyType::kFileAdded, toInfo->url());
        return true;
    }
    // resize target file
    if (jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyResizeDestinationFile) && !resizeTargetFile(fromInfo, toInfo, toDevice, result))
        return *result;
    // 循环读取和写入文件，拷贝
    qint64 blockSize = fromInfo->size() > kMaxBufferLength ? kMaxBufferLength : fromInfo->size();
    char *data = new char[blockSize + 1];
    uLong sourceCheckSum = adler32(0L, nullptr, 0);
    qint64 sizeRead = 0;

    do {
        if (!doReadFile(fromInfo, toInfo, fromDevice, data, blockSize, sizeRead, result)) {
            delete[] data;
            data = nullptr;
            return *result;
        }

        if (!doWriteFile(fromInfo, toInfo, toDevice, data, sizeRead, result)) {
            delete[] data;
            data = nullptr;
            return *result;
        }

        if (Q_LIKELY(jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyIntegrityChecking))) {
            sourceCheckSum = adler32(sourceCheckSum, reinterpret_cast<Bytef *>(data), static_cast<uInt>(sizeRead));
        }

        toInfo->refresh(DFMIO::DFileInfo::AttributeID::kStandardSize, toDevice->size());

    } while (fromDevice->pos() != fromInfo->size());

    delete[] data;
    data = nullptr;

    // 对文件加权
    setTargetPermissions(fromInfo, toInfo);
    if (Q_UNLIKELY(!stateCheck())) {
        return false;
    }

    // 校验文件完整性
    *result = verifyFileIntegrity(blockSize, sourceCheckSum, fromInfo, toInfo, toDevice);
    toInfo->refresh();

    if (*result)
        FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::Global::FileNotifyType::kFileAdded, toInfo->url());
    return *result;
}

bool FileOperateBaseWorker::canWriteFile(const QUrl &url) const
{
    // root user return true direct
    if (getuid() == 0)
        return true;

    DecoratorFileInfo info(url);
    if (!info.isValid())
        return false;

    DecoratorFileInfo parentInfo(info.parentUrl());
    if (!parentInfo.isValid())
        return false;

    bool isFolderWritable = parentInfo.isWritable();
    if (!isFolderWritable)
        return false;

#ifdef Q_OS_LINUX
    struct stat statBuffer;
    if (::lstat(parentInfo.parentPath().toLocal8Bit().data(), &statBuffer) == 0) {
        // 如果父目录拥有t权限，则判断当前用户是不是文件的owner，不是则无法操作文件
        const auto &fileOwnerId = info.ownerId();
        const auto &uid = info.ownerId();
        const bool hasTRight = (statBuffer.st_mode & S_ISVTX) == S_ISVTX;
        if (hasTRight && fileOwnerId != uid) {
            return false;
        }
    }
#endif

    return true;
}

bool FileOperateBaseWorker::resizeTargetFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, const QSharedPointer<DFile> &file, bool *result)
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
        *result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }
    return true;
}

bool FileOperateBaseWorker::verifyFileIntegrity(const qint64 &blockSize, const ulong &sourceCheckSum,
                                                const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, QSharedPointer<DFile> &toDevice)
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

void FileOperateBaseWorker::setAllDirPermisson()
{
    for (auto info : dirPermissonList.list()) {
        handler->setPermissions(info->target, info->permission);
    }
}

void FileOperateBaseWorker::cancelThreadProcessingError()
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
qint64 FileOperateBaseWorker::getWriteDataSize()
{
    qint64 writeSize = currentWritSize;

    if (CountWriteSizeType::kTidType == countWriteType) {
        writeSize = getTidWriteSize();
    } else if (CountWriteSizeType::kCustomizeType == countWriteType) {
        writeSize = currentWritSize;
    } else {
        if (targetDeviceStartSectorsWritten >= 0) {
            if ((getSectorsWritten() == 0) && (targetDeviceStartSectorsWritten > 0)) {
                writeSize = 0;
            } else {
                writeSize = (getSectorsWritten() - targetDeviceStartSectorsWritten) * targetLogSecionSize;
            }
        }
    }

    if (writeSize > currentWritSize && currentWritSize > 0) {
        writeSize = currentWritSize;
    }

    writeSize += currentWritSize;

    writeSize += skipWritSize;

    return writeSize;
}

qint64 FileOperateBaseWorker::getTidWriteSize()
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

qint64 FileOperateBaseWorker::getSectorsWritten()
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

void FileOperateBaseWorker::determineCountProcessType()
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

void FileOperateBaseWorker::syncFilesToDevice()
{
    if (CountWriteSizeType::kWriteBlockType != countWriteType)
        return;

    qDebug() << __FUNCTION__ << "syncFilesToDevice begin";
    qint64 writeSize = getWriteDataSize();
    while (!isStopped() && sourceFilesTotalSize > 0 && writeSize < sourceFilesTotalSize) {
        QThread::msleep(100);
        writeSize = getWriteDataSize();
    }
    qDebug() << __FUNCTION__ << "syncFilesToDevice end";
}
