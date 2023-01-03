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

#include "docopyfileworker.h"
#include "utils/fileutils.h"

#include <QDebug>
#include <QTime>
#include <QWaitCondition>
#include <QMutex>

#include <dfm-io/core/diofactory.h>
#include <dfm-io/dfmio_register.h>
#include <dfm-io/dfmio_utils.h>

#include <fcntl.h>
#include <zlib.h>

static const quint32 kMaxBufferLength { 1024 * 1024 * 1 };

DPFILEOPERATIONS_USE_NAMESPACE
USING_IO_NAMESPACE
DFMBASE_USE_NAMESPACE

DoCopyFileWorker::DoCopyFileWorker(const QSharedPointer<WorkerData> &data, QObject *parent)
    : QObject(parent), workData(data)
{
    waitCondition.reset(new QWaitCondition);
    mutex.reset(new QMutex);
    localFileHandler.reset(new LocalFileHandler);
    connect(this, &DoCopyFileWorker::copyFile, this, &DoCopyFileWorker::doFileCopy, Qt::QueuedConnection);
}

DoCopyFileWorker::~DoCopyFileWorker()
{
}
// main thread using
void DoCopyFileWorker::pause()
{
    state = kPasued;
}
// main thread using
void DoCopyFileWorker::resume()
{
    state = kNormal;
    waitCondition->wakeAll();
}
// main thread using// main thread using
void DoCopyFileWorker::stop()
{
    state = kStoped;
    waitCondition->wakeAll();
}
// main thread using
void DoCopyFileWorker::operateAction(const AbstractJobHandler::SupportAction action)
{
    retry = !workData->signalThread && AbstractJobHandler::SupportAction::kRetryAction == action;
    currentAction = action;
    resume();
}
void DoCopyFileWorker::doFileCopy(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo)
{
    doCopyFilePractically(fromInfo, toInfo, nullptr);
    workData->completeFileCount++;
}
// copy thread using
bool DoCopyFileWorker::doCopyFilePractically(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo, bool *skip)
{
    if (isStoped())
        return false;
    // emit current task url
    emit currentTask(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl));
    // read ahead source file
    readAheadSourceFile(fromInfo);
    // 创建文件的divice
    QSharedPointer<DFMIO::DFile> fromDevice { nullptr }, toDevice { nullptr };
    if (!createFileDevices(fromInfo, toInfo, fromDevice, toDevice, skip))
        return false;
    // 打开文件并创建
    if (!openFiles(fromInfo, toInfo, fromDevice, toDevice, skip))
        return false;
    // 源文件大小如果为0
    if (fromInfo->size() <= 0) {
        // 对文件加权
        setTargetPermissions(fromInfo, toInfo);
        workData->zeroOrlinkOrDirWriteSize += FileUtils::getMemoryPageSize();
        FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::Global::FileNotifyType::kFileAdded, toInfo->urlOf(UrlInfoType::kUrl));
        return true;
    }
    // resize target file
    if (workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyResizeDestinationFile) && !resizeTargetFile(fromInfo, toInfo, toDevice, skip))
        return false;
    // 循环读取和写入文件，拷贝
    qint64 blockSize = fromInfo->size() > kMaxBufferLength ? kMaxBufferLength : fromInfo->size();
    char *data = new char[static_cast<uint>(blockSize + 1)];
    uLong sourceCheckSum = adler32(0L, nullptr, 0);
    qint64 sizeRead = 0;

    do {
        if (!doReadFile(fromInfo, toInfo, fromDevice, data, blockSize, sizeRead, skip)) {
            delete[] data;
            data = nullptr;
            return false;
        }

        if (!doWriteFile(fromInfo, toInfo, toDevice, data, sizeRead, skip)) {
            delete[] data;
            data = nullptr;
            return false;
        }

        if (Q_LIKELY(workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyIntegrityChecking))) {
            sourceCheckSum = adler32(sourceCheckSum, reinterpret_cast<Bytef *>(data), static_cast<uInt>(sizeRead));
        }

        toInfo->cacheAttribute(DFMIO::DFileInfo::AttributeID::kStandardSize, toDevice->size());

    } while (fromDevice->pos() != fromInfo->size());

    delete[] data;
    data = nullptr;

    // 对文件加权
    setTargetPermissions(fromInfo, toInfo);
    if (!stateCheck())
        return false;

    // 校验文件完整性
    if (skip)
        *skip = verifyFileIntegrity(blockSize, sourceCheckSum, fromInfo, toInfo, toDevice);
    toInfo->refresh();

    if (skip && *skip)
        FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::Global::FileNotifyType::kFileAdded, toInfo->urlOf(UrlInfoType::kUrl));

    return true;
}

bool DoCopyFileWorker::stateCheck()
{
    if (state == kPasued)
        workerWait();
    return state == kNormal;
}

void DoCopyFileWorker::workerWait()
{
    waitCondition->wait(mutex.data());
    mutex->unlock();
}

bool DoCopyFileWorker::actionOperating(const AbstractJobHandler::SupportAction action, const qint64 size, bool *skip)
{
    if (isStoped())
        return false;

    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        if (action == AbstractJobHandler::SupportAction::kSkipAction) {
            if (skip)
                *skip = true;
            workData->skipWriteSize += size;
        }
        return false;
    }

    return true;
}
/*!
 * \brief DoCopyFileWorker::doHandleErrorAndWait Handle the error and block waiting for the error handling operation to return
 * \param fromUrl URL of the source file
 * \param toUrl Destination URL
 * \param errorUrl URL of the error file
 * \param error Wrong type
 * \param errorMsg Wrong message
 * \return AbstractJobHandler::SupportAction Current processing operation
 */
AbstractJobHandler::SupportAction DoCopyFileWorker::doHandleErrorAndWait(const QUrl &urlFrom, const QUrl &urlTo,
                                                                         const AbstractJobHandler::JobErrorType &error,
                                                                         const QString &errorMsg)
{
    if (workData->errorOfAction.contains(error))
        return workData->errorOfAction.value(error);

    if (FileUtils::isSameFile(urlFrom, urlTo, false)) {
        currentAction = AbstractJobHandler::SupportAction::kCoexistAction;
        return currentAction;
    }

    if (isStoped())
        return AbstractJobHandler::SupportAction::kCancelAction;

    // 发送错误处理 阻塞自己
    const QString &errorMsgAll = errorMsg.isEmpty() ? AbstractJobHandler::errorToString(error) : (AbstractJobHandler::errorToString(error) + ": " + errorMsg);
    emit errorNotify(urlFrom, urlTo, error, quintptr(this), errorMsgAll);
    workerWait();

    if (isStoped())
        return AbstractJobHandler::SupportAction::kCancelAction;

    return currentAction;
}

/*!
 * \brief FileOperateBaseWorker::readAheadSourceFile Pre read source file content
 * \param fileInfo File information of source file
 */
void DoCopyFileWorker::readAheadSourceFile(const AbstractFileInfoPointer &fileInfo)
{
    if (fileInfo->size() <= 0)
        return;
    std::string stdStr = fileInfo->urlOf(UrlInfoType::kUrl).path().toUtf8().toStdString();
    int fromfd = open(stdStr.data(), O_RDONLY);
    if (-1 != fromfd) {
        readahead(fromfd, 0, static_cast<size_t>(fileInfo->size()));
        close(fromfd);
    }
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
bool DoCopyFileWorker::createFileDevice(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                        const AbstractFileInfoPointer &needOpenInfo, QSharedPointer<DFMIO::DFile> &file,
                                        bool *skip)
{
    file.reset();
    QUrl url = needOpenInfo->urlOf(UrlInfoType::kUrl);
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    QSharedPointer<DIOFactory> factory { nullptr };
    do {
        factory = produceQSharedIOFactory(url.scheme(), static_cast<QUrl>(url));
        if (!factory) {
            qCritical() << "create dfm io factory failed! url = " << url;
            action = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl), AbstractJobHandler::JobErrorType::kDfmIoError);
        }
    } while (!isStoped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    checkRetry();

    if (!actionOperating(action, fromInfo->size() <= 0 ? workData->dirSize : fromInfo->size(), skip))
        return false;

    do {
        file = factory->createFile();
        if (!file) {
            qCritical() << "create dfm io dfile failed! url = " << url;
            action = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl), AbstractJobHandler::JobErrorType::kDfmIoError);
        }
    } while (!isStoped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    checkRetry();

    if (!actionOperating(action, fromInfo->size() <= 0 ? workData->dirSize : fromInfo->size(), skip))
        return false;

    return true;
}
/*!
 * \brief DoCopyFileWorker::createFileDevices Device for creating source and directory files
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 * \param fromeFile Output parameter: device of source file
 * \param toFile Output parameter: device of target file
 * \param result result Output parameter: whether skip
 * \return Whether the device of source file and target file is created successfully
 */
bool DoCopyFileWorker::createFileDevices(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                         QSharedPointer<DFMIO::DFile> &fromeFile, QSharedPointer<DFMIO::DFile> &toFile, bool *skip)
{
    if (!createFileDevice(fromInfo, toInfo, fromInfo, fromeFile, skip))
        return false;
    if (!createFileDevice(fromInfo, toInfo, toInfo, toFile, skip))
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
bool DoCopyFileWorker::openFiles(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                 const QSharedPointer<DFMIO::DFile> &fromeFile, const QSharedPointer<DFMIO::DFile> &toFile,
                                 bool *skip)
{
    if (fromInfo->size() > 0 && !openFile(fromInfo, toInfo, fromeFile, DFMIO::DFile::OpenFlag::kReadOnly, skip)) {
        return false;
    }

    if (!openFile(fromInfo, toInfo, toFile, DFMIO::DFile::OpenFlag::kWriteOnly | DFMIO::DFile::OpenFlag::kTruncate, skip)) {
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
bool DoCopyFileWorker::openFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                const QSharedPointer<DFMIO::DFile> &file, const DFMIO::DFile::OpenFlags &flags,
                                bool *skip)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    do {
        if (!file->open(flags)) {
            auto lastError = file->lastError();
            qWarning() << "file open error, url from: " << fromInfo->urlOf(UrlInfoType::kUrl)
                       << " url to: " << toInfo->urlOf(UrlInfoType::kUrl) << " open flag: " << flags
                       << " error code: " << lastError.code() << " error msg: " << lastError.errorMsg();

            action = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl), AbstractJobHandler::JobErrorType::kOpenError, lastError.errorMsg());
        }
    } while (!isStoped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    checkRetry();

    if (!actionOperating(action, fromInfo->size() <= 0 ? FileUtils::getMemoryPageSize() : fromInfo->size(), skip))
        return false;
    return true;
}

bool DoCopyFileWorker::resizeTargetFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                        const QSharedPointer<DFMIO::DFile> &file, bool *skip)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    do {
        if (!file->write(QByteArray())) {
            action = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl), AbstractJobHandler::JobErrorType::kResizeError, file->lastError().errorMsg());
        }
    } while (!isStoped() && action == AbstractJobHandler::SupportAction::kRetryAction);
    if (!actionOperating(action, fromInfo->size() <= 0 ? workData->dirSize : fromInfo->size(), skip))
        return false;
    return true;
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
bool DoCopyFileWorker::doReadFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                  const QSharedPointer<DFMIO::DFile> &fromDevice,
                                  char *data, const qint64 &blockSize,
                                  qint64 &readSize, bool *skip)
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

            fromInfo->refresh();
            const bool fromInfoExist = fromInfo->exists();
            AbstractJobHandler::JobErrorType errortype = fromInfoExist ? AbstractJobHandler::JobErrorType::kReadError : AbstractJobHandler::JobErrorType::kNonexistenceError;
            QString errorstr = fromInfoExist ? QString(QObject::tr("Failed to read the file, cause: %1")).arg("to something!") : QString();

            actionForRead = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl), errortype, errorstr);

            if (actionForRead == AbstractJobHandler::SupportAction::kRetryAction) {
                if (!fromDevice->seek(currentPos)) {
                    // 优先处理
                    AbstractJobHandler::SupportAction actionForReadSeek = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl),
                                                                                               toInfo->urlOf(UrlInfoType::kUrl),
                                                                                               AbstractJobHandler::JobErrorType::kSeekError);
                    checkRetry();
                    if (!actionOperating(actionForReadSeek, fromInfo->size() - currentPos, skip))
                        return false;
                    return false;
                }
            }
        }
    } while (!isStoped() && actionForRead == AbstractJobHandler::SupportAction::kRetryAction);
    checkRetry();

    if (!actionOperating(actionForRead, fromInfo->size() - currentPos, skip))
        return false;

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
bool DoCopyFileWorker::doWriteFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                   const QSharedPointer<DFMIO::DFile> &toDevice,
                                   const char *data, const qint64 readSize, bool *skip)
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
            if (sizeWrite > 0)
                workData->currentWriteSize += sizeWrite;
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

        actionForWrite = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl), AbstractJobHandler::JobErrorType::kWriteError, errorStr);
        if (actionForWrite == AbstractJobHandler::SupportAction::kRetryAction) {
            if (!toDevice->seek(currentPos)) {
                AbstractJobHandler::SupportAction actionForWriteSeek = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl),
                                                                                            AbstractJobHandler::JobErrorType::kSeekError);
                checkRetry();
                actionOperating(actionForWriteSeek, fromInfo->size() - (currentPos + readSize - surplusSize), skip);
                return false;
            }
        }
    } while (!isStoped() && actionForWrite == AbstractJobHandler::SupportAction::kRetryAction);

    checkRetry();

    if (!actionOperating(actionForWrite, fromInfo->size() - (currentPos + readSize - surplusSize), skip))
        return false;

    if (workData->needSyncEveryRW && sizeWrite > 0) {
        if (workData->isFsTypeVfat) {
            // FAT and VFAT file systems ignore the "sync" option
            toDevice->close();
            if (!openFile(fromInfo, toInfo, toDevice, DFMIO::DFile::OpenFlag::kWriteOnly | DFMIO::DFile::OpenFlag::kAppend, skip)) {
                return false;
            }
        } else {
            toDevice->flush();
        }
    }
    return true;
}

bool DoCopyFileWorker::verifyFileIntegrity(const qint64 &blockSize, const ulong &sourceCheckSum,
                                           const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                           QSharedPointer<DFMIO::DFile> &toDevice)
{
    if (!workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyIntegrityChecking))
        return true;
    char *data = new char[static_cast<uint>(blockSize + 1)];
    QTime t;
    ulong targetCheckSum = adler32(0L, nullptr, 0);
    Q_FOREVER {
        qint64 size = toDevice->read(data, blockSize);

        if (Q_UNLIKELY(size <= 0)) {
            if (size == 0 && toInfo->size() == toDevice->pos()) {
                break;
            }

            QString errorstr = QObject::tr("File integrity was damaged, cause: %1").arg("some error occ!");
            AbstractJobHandler::SupportAction actionForCheckRead = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl), AbstractJobHandler::JobErrorType::kIntegrityCheckingError, errorstr);
            if (!isStoped() && AbstractJobHandler::SupportAction::kRetryAction == actionForCheckRead) {
                continue;
            } else {
                checkRetry();
                return actionForCheckRead == AbstractJobHandler::SupportAction::kSkipAction;
            }
        }

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
        AbstractJobHandler::SupportAction actionForCheck = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl), AbstractJobHandler::JobErrorType::kIntegrityCheckingError, errorstr);
        return actionForCheck == AbstractJobHandler::SupportAction::kSkipAction;
    }

    return true;
}

void DoCopyFileWorker::checkRetry()
{
    if (!workData->signalThread && retry) {
        retry = false;
        emit retryErrSuccess(quintptr(this));
    }
}

bool DoCopyFileWorker::isStoped()
{
    return state == kStoped;
}

/*!
 * \brief FileOperateBaseWorker::setTargetPermissions Set permissions on the target file
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 */
void DoCopyFileWorker::setTargetPermissions(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo)
{
    // 修改文件修改时间
    localFileHandler->setFileTime(toInfo->urlOf(UrlInfoType::kUrl), fromInfo->timeOf(TimeInfoType::kLastRead).value<QDateTime>(), fromInfo->timeOf(TimeInfoType::kLastModified).value<QDateTime>());
    QFileDevice::Permissions permissions = fromInfo->permissions();
    QString path = fromInfo->urlOf(UrlInfoType::kUrl).path();
    //权限为0000时，源文件已经被删除，无需修改新建的文件的权限为0000
    if (permissions != 0000)
        localFileHandler->setPermissions(toInfo->urlOf(UrlInfoType::kUrl), permissions);
}
