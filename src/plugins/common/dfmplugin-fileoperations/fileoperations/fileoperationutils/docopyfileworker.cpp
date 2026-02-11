// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "docopyfileworker.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/protocolutils.h>

#include <dfm-io/dfmio_utils.h>

#include <QDebug>
#include <QTime>
#include <QWaitCondition>
#include <QMutex>
#include <QThread>

#include <fcntl.h>
#include <zlib.h>
#include <sys/mman.h>
#include <unistd.h>

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
}

DoCopyFileWorker::~DoCopyFileWorker()
{
    // Ensure all waiting threads are woken up before destruction
    if (waitCondition) {
        waitCondition->wakeAll();
    }
}
// main thread using
void DoCopyFileWorker::pause()
{
    if (state == kPaused || state == kStopped)
        return;
    state = kPaused;
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
    state = kStopped;
    waitCondition->wakeAll();
    auto fileOpsAll = fileOps.listByLock();
    for (auto op : fileOpsAll) {
        op->cancel();
    }
}

// main thread using
void DoCopyFileWorker::operateAction(const AbstractJobHandler::SupportAction action)
{
    retry = !workData->singleThread && AbstractJobHandler::SupportAction::kRetryAction == action;
    currentAction = action;
    resume();
}
void DoCopyFileWorker::doFileCopy(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo)
{
    doDfmioFileCopy(fromInfo, toInfo, nullptr);
    workData->completeFileCount++;
}

bool DoCopyFileWorker::doDfmioFileCopy(const DFileInfoPointer fromInfo,
                                       const DFileInfoPointer toInfo, bool *skip)
{
    assert(!fromInfo.isNull());
    assert(!toInfo.isNull());
    if (isStopped())
        return false;
    // read ahead source file
    readAheadSourceFile(fromInfo);

    if (!stateCheck())
        return false;
    // emit current task url
    auto fromUrl = fromInfo->uri();
    auto toUrl = toInfo->uri();
    emit currentTask(fromUrl, toUrl);
    // 创建doperator类
    QSharedPointer<dfmio::DOperator> op { new dfmio::DOperator(fromUrl) };
    fileOps.appendByLock(op);
    ProgressData *data { new ProgressData() };
    data->data = workData;
    data->copyFile = fromUrl;
    bool ret { false };

    DFile::CopyFlags flag = DFile::CopyFlag::kNoFollowSymlinks | DFile::CopyFlag::kOverwrite;
    if (!DeviceUtils::supportSetPermissionsDevice(toUrl))
        flag |= DFile::CopyFlag::kTargetDefaultPerms;
    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    do {
        // Check if stopped before potentially blocking copyFile operation
        if (isStopped()) {
            fmDebug() << "Operation stopped before copyFile - from:" << fromUrl << "to:" << toUrl;
            ret = false;
            break;
        }

        ret = op->copyFile(toUrl, flag, progressCallback, data);
        action = AbstractJobHandler::SupportAction::kNoAction;
        if (!ret) {
            auto lastError = op->lastError().errorMsg();
            fmWarning() << "DFMIO copy failed - from:" << fromUrl << "to:" << toUrl << "error:" << lastError;

            action = doHandleErrorAndWait(fromUrl, toUrl,
                                          AbstractJobHandler::JobErrorType::kDfmIoError, false, lastError);
        }
    } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());

    checkRetry();

    fileOps.removeOneByLock(op);
    auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
    if (!actionOperating(action, fromSize <= 0 ? FileUtils::getMemoryPageSize() : fromSize, skip))
        workData->currentWriteSize -= workData->everyFileWriteSize.value(fromUrl);

    workData->everyFileWriteSize.remove(fromUrl);
    delete data;
    toInfo->initQuerier();
    if (toInfo->exists())
        FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::Global::FileNotifyType::kFileAdded, toUrl);

    return ret;
}

void DoCopyFileWorker::progressCallback(int64_t current, int64_t total, void *progressData)
{
    assert(progressData);
    auto data = static_cast<ProgressData *>(progressData);
    assert(data);
    assert(data->data);
    if (total <= 0)
        data->data->zeroOrlinkOrDirWriteSize += FileUtils::getMemoryPageSize();
    data->data->currentWriteSize += (current - data->data->everyFileWriteSize.value(data->copyFile));
    data->data->everyFileWriteSize.insert(data->copyFile, current);
}

/*!
 * \brief DoCopyFileWorker::openFile
 * \param fromInfo
 * \param toInfo
 * \param flags
 * \param skip
 * \param isSource
 * \return
 */
int DoCopyFileWorker::openFileBySys(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                                    const int flags, bool *skip, const bool isSource)
{
    int fd = -1;
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    auto openUrl = isSource ? fromInfo->uri() : toInfo->uri();
    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        if (flags & O_CREAT) {
            fd = open(openUrl.path().toStdString().c_str(), flags, 0666);
        } else {
            fd = open(openUrl.path().toStdString().c_str(), flags);
        }
        if (fd < 0) {
            auto lastError = strerror(errno);
            fmWarning() << "file open error, url from: " << fromInfo->uri()
                        << " url to: " << toInfo->uri() << " open flag: " << flags
                        << " open url : " << openUrl << " error msg: " << lastError;
            action = doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(),
                                          AbstractJobHandler::JobErrorType::kOpenError,
                                          !isSource, lastError);
        }
    } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());
    checkRetry();
    auto fileSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
    if (!actionOperating(action, fileSize <= 0 ? FileUtils::getMemoryPageSize() : fileSize, skip)) {
        close(fd);
        return -1;
    }
    if (isSource && fileSize > 100 * 1024 * 1024)
        readahead(fd, 0, static_cast<size_t>(fileSize));
    return fd;
}

// copy thread using
DoCopyFileWorker::NextDo DoCopyFileWorker::doCopyFilePractically(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip)
{
    if (isStopped())
        return NextDo::kDoCopyErrorAddCancel;

    // emit current task url
    emit currentTask(fromInfo->uri(), toInfo->uri());

    // read ahead source file
    readAheadSourceFile(fromInfo);

    // Check if we should use O_DIRECT mode (safe sync mode for local to external device)
    // Use the existing isSourceFileLocal and isTargetFileLocal from base worker
    bool useDirectMode = workData->exBlockSyncEveryWrite && !workData->isTargetFileLocal;
    if (useDirectMode) {
        // Use new O_DIRECT implementation for safe sync mode
        return doCopyFileWithDirectIO(fromInfo, toInfo, skip);
    } else {
        // Use traditional DFMIO implementation for other cases
        return doCopyFileTraditional(fromInfo, toInfo, skip);
    }
}

/*!
 * \brief DoCopyFileWorker::doCopyFileWithDirectIO Copy file using O_DIRECT mode for safe sync
 * \param fromInfo Source file info
 * \param toInfo Target file info
 * \param skip Skip flag
 * \return NextDo status
 */
DoCopyFileWorker::NextDo DoCopyFileWorker::doCopyFileWithDirectIO(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip)
{
    QString sourcePath = fromInfo->uri().path();
    QString destPath = toInfo->uri().path();

    // Open source file
    int srcFd = open(sourcePath.toLocal8Bit().constData(), O_RDONLY);
    if (srcFd < 0) {
        fmWarning() << "Failed to open source file with O_DIRECT mode - file:" << sourcePath << "error:" << strerror(errno);
        auto action = doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(),
                                           AbstractJobHandler::JobErrorType::kOpenError);
        return actionToNextDo(action, fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong(), skip);
    }

    auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();

    // Open destination file with O_DIRECT
    WriteMode preferredMode = WriteMode::Direct;
    FileWriter writer = openDestinationFile(destPath, preferredMode);
    if (writer.fd < 0) {
        close(srcFd);
        auto action = doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(),
                                           AbstractJobHandler::JobErrorType::kOpenError, true);
        return actionToNextDo(action, fromSize, skip);
    }

    // Get optimal chunk size and ensure alignment
    qint64 baseChunkSize = 1024 * 1024;   // 1M
    qint64 chunkSize = ((baseChunkSize + writer.alignment - 1) / writer.alignment) * writer.alignment;

    // Allocate aligned buffer
    char *buffer = allocateAlignedBuffer(chunkSize, writer.alignment);
    if (!buffer) {
        close(srcFd);
        close(writer.fd);
        fmWarning() << "Failed to allocate aligned buffer for O_DIRECT copy - from:" << sourcePath << "to:" << destPath;
        auto action = doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(),
                                           AbstractJobHandler::JobErrorType::kProrogramError, false,
                                           QString("Failed to allocate aligned buffer"));
        return actionToNextDo(action, fromSize, skip);
    }

    qint64 copied = 0;
    bool directModeActive = (writer.mode == WriteMode::Direct);
    bool success = true;

    while (copied < fromSize && !isStopped()) {
        // Handle pause/resume
        if (state == kPaused) {
            if (!handlePauseResume(writer, destPath, skip)) {
                success = false;
                break;
            }
            // Seek source file to correct position after resume
            if (lseek(srcFd, copied, SEEK_SET) < 0) {
                fmWarning() << "Failed to seek source file after resume - file:" << sourcePath << "error:" << strerror(errno);
                success = false;
                break;
            }
        }

        if (isStopped()) {
            success = false;
            break;
        }

        qint64 remaining = fromSize - copied;
        qint64 toRead = qMin(chunkSize, remaining);

        // For O_DIRECT, ensure read size is aligned (except for the very last read)
        if (writer.mode == WriteMode::Direct && toRead < chunkSize && toRead % writer.alignment != 0) {
            toRead = ((toRead + writer.alignment - 1) / writer.alignment) * writer.alignment;
            toRead = qMin(toRead, remaining);
        }

        ssize_t bytesRead = read(srcFd, buffer, toRead);
        if (bytesRead < 0) {
            if (errno == EINTR) {
                continue;
            }
            // Save errno immediately before any other function calls
            int savedErrno = errno;
            fmWarning() << "Read error during O_DIRECT copy - file:" << sourcePath << "error:" << strerror(savedErrno);

            // Handle read error with UI dialog
            auto jobError = mapSystemErrorToJobError(savedErrno, false);
            AbstractJobHandler::SupportAction action = doHandleErrorAndWait(
                fromInfo->uri(), toInfo->uri(), jobError, false);

            if (action == AbstractJobHandler::SupportAction::kRetryAction) {
                // Seek back to retry reading this chunk
                if (lseek(srcFd, copied, SEEK_SET) < 0) {
                    success = false;
                    break;
                }
                continue;   // Retry the read
            }

            // For Skip or Cancel, set success to false and break
            success = false;
            break;
        }

        if (bytesRead == 0) {
            break;   // EOF
        }

        // Only write the actual bytes we need
        qint64 actualBytesToWrite = qMin((qint64)bytesRead, fromSize - copied);
        ssize_t bytesWritten = 0;

        while (bytesWritten < actualBytesToWrite && !isStopped()) {
            ssize_t written = write(writer.fd, buffer + bytesWritten,
                                    actualBytesToWrite - bytesWritten);

            if (written < 0) {
                if (errno == EINTR) {
                    continue;
                }

                // Save errno immediately before any other function calls
                int savedErrno = errno;

                // Fallback: If O_DIRECT write fails, remove O_DIRECT flag and continue
                if (directModeActive && savedErrno == EINVAL) {
                    fmDebug() << "O_DIRECT write failed, removing O_DIRECT flag - file:" << destPath;
                    int flags = fcntl(writer.fd, F_GETFL);
                    if (flags != -1) {
                        flags &= ~O_DIRECT;
                        if (fcntl(writer.fd, F_SETFL, flags) == 0) {
                            directModeActive = false;
                            fmDebug() << "Successfully removed O_DIRECT, continuing with regular I/O";
                            continue;   // Retry the write without O_DIRECT
                        }
                    }
                }

                fmWarning() << "Write error during O_DIRECT copy - file:" << destPath << "error:" << strerror(savedErrno);

                // Handle write error with UI dialog
                auto jobError = mapSystemErrorToJobError(savedErrno, true);
                AbstractJobHandler::SupportAction action = doHandleErrorAndWait(
                    fromInfo->uri(), toInfo->uri(), jobError, true);

                if (action == AbstractJobHandler::SupportAction::kRetryAction) {
                    // Seek to retry writing this chunk
                    if (lseek(writer.fd, copied + bytesWritten, SEEK_SET) < 0) {
                        success = false;
                        break;
                    }
                    continue;   // Retry the write
                }

                // For Skip or Cancel, set success to false and break
                success = false;
                break;
            }
            bytesWritten += written;
        }

        if (!success) {
            break;
        }

        copied += actualBytesToWrite;
        workData->currentWriteSize += actualBytesToWrite;
    }

    // Cleanup
    free(buffer);
    close(srcFd);
    close(writer.fd);

    if (!success) {
        fmWarning() << "O_DIRECT copy failed - from:" << fromInfo->uri() << "to:" << toInfo->uri();
        return NextDo::kDoCopyErrorAddCancel;
    }

    // Set file permissions and timestamps
    setTargetPermissions(fromInfo->uri(), toInfo->uri());

    if (!stateCheck())
        return NextDo::kDoCopyErrorAddCancel;

    toInfo->refresh();
    FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::Global::FileNotifyType::kFileAdded, toInfo->uri());

    return NextDo::kDoCopyNext;
}

/*!
 * \brief DoCopyFileWorker::doCopyFileTraditional Traditional copy implementation using DFMIO
 * \param fromInfo Source file info
 * \param toInfo Target file info
 * \param skip Skip flag
 * \return NextDo status
 */
DoCopyFileWorker::NextDo DoCopyFileWorker::doCopyFileTraditional(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip)
{
    // 创建文件的device
    QSharedPointer<DFMIO::DFile> fromDevice { nullptr }, toDevice { nullptr };
    if (!createFileDevices(fromInfo, toInfo, fromDevice, toDevice, skip))
        return NextDo::kDoCopyErrorAddCancel;

    // 打开文件并创建
    if (!openFiles(fromInfo, toInfo, fromDevice, toDevice, skip))
        return NextDo::kDoCopyErrorAddCancel;

    auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();

    // resize target file
    if (workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyResizeDestinationFile) && !resizeTargetFile(fromInfo, toInfo, toDevice, skip))
        return NextDo::kDoCopyErrorAddCancel;

    // 循环读取和写入文件，拷贝
    qint64 blockSize = fromSize > kMaxBufferLength ? kMaxBufferLength : fromSize;
    char *data = new char[static_cast<uint>(blockSize + 1)];
    uLong sourceCheckSum = adler32(0L, nullptr, 0);
    qint64 sizeRead = 0;

    do {
        auto nextReadDo = doReadFile(fromInfo, toInfo, fromDevice, data, blockSize, sizeRead, skip);
        if (nextReadDo != NextDo::kDoCopyCurrentFile) {
            delete[] data;
            return nextReadDo;
        }
        auto nextDo = doWriteFile(fromInfo, toInfo, toDevice, fromDevice, data, sizeRead, skip);
        if (nextDo != NextDo::kDoCopyCurrentFile) {
            delete[] data;
            return nextDo;
        }

        if (Q_LIKELY(workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyIntegrityChecking))) {
            sourceCheckSum = adler32(sourceCheckSum, reinterpret_cast<Bytef *>(data), static_cast<uInt>(sizeRead));
        }

    } while (fromDevice->pos() != fromSize);

    delete[] data;

    // 对文件加权
    setTargetPermissions(fromInfo->uri(), toInfo->uri());
    if (!stateCheck())
        return NextDo::kDoCopyErrorAddCancel;

    // 校验文件完整性
    if (skip)
        *skip = verifyFileIntegrity(blockSize, sourceCheckSum, fromInfo, toInfo, toDevice);
    toInfo->refresh();

    if (skip && *skip)
        FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::Global::FileNotifyType::kFileAdded, toInfo->uri());

    return NextDo::kDoCopyNext;
}

/*!
 * \brief DoCopyFileWorker::actionToNextDo Convert action to NextDo enum
 * \param action Support action
 * \param size File size for skip calculation
 * \param skip Skip flag
 * \return NextDo status
 */
DoCopyFileWorker::NextDo DoCopyFileWorker::actionToNextDo(AbstractJobHandler::SupportAction action, qint64 size, bool *skip)
{
    if (!actionOperating(action, size, skip)) {
        return NextDo::kDoCopyErrorAddCancel;
    }
    return NextDo::kDoCopyNext;
}

/*!
 * \brief DoCopyFileWorker::doCopyFileByRange
 * \param fromInfo
 * \param toInfo
 * \param skip
 * \return
 */
DoCopyFileWorker::NextDo DoCopyFileWorker::doCopyFileByRange(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip)
{
    if (isStopped())
        return NextDo::kDoCopyErrorAddCancel;
    // emit current task url
    emit currentTask(fromInfo->uri(), toInfo->uri());
    // open source file
    int sourcFd = openFileBySys(fromInfo, toInfo, O_RDONLY, skip);
    if (sourcFd < 0)
        return NextDo::kDoCopyErrorAddCancel;
    FinallyUtil releaseSc([&] {
        close(sourcFd);
    });
    int targetFd = openFileBySys(fromInfo, toInfo, O_CREAT | O_WRONLY | O_TRUNC, skip, false);
    if (targetFd < 0) {
        return NextDo::kDoCopyErrorAddCancel;
    }
    FinallyUtil releaseTg([&] {
        close(targetFd);
    });
    // 源文件大小如果为0
    auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
    if (fromSize <= 0) {
        // 对文件加权
        setTargetPermissions(fromInfo->uri(), toInfo->uri());
        workData->zeroOrlinkOrDirWriteSize += FileUtils::getMemoryPageSize();
        FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::Global::FileNotifyType::kFileAdded, toInfo->uri());
        return NextDo::kDoCopyNext;
    }
    // 循环读取和写入文件，拷贝
    size_t blockSize = static_cast<size_t>(fromSize > kMaxBufferLength ? kMaxBufferLength : fromSize);
    off_t offset_in = 0;
    off_t offset_out = 0;
    size_t total = static_cast<size_t>(fromSize);
    ssize_t result = -1;
    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    do {
        if (Q_UNLIKELY(!stateCheck()))
            return NextDo::kDoCopyErrorAddCancel;
        do {
            if (Q_UNLIKELY(!stateCheck()))
                return NextDo::kDoCopyErrorAddCancel;
            blockSize = total < blockSize ? total : blockSize;
            result = copy_file_range(sourcFd, &offset_in, targetFd, &offset_out, blockSize, 0);
            if (result < 0) {
                // Check if this is a "should fallback" error vs a real error
                if (shouldFallbackFromCopyFileRange(errno)) {
                    // Silent fallback for unsupported scenarios
                    fmWarning() << "copy_file_range fallback needed - error:" << strerror(errno);
                    return NextDo::kDoCopyFallback;   // Signal fallback needed
                }

                // Real error - show dialog
                auto lastError = strerror(errno);
                fmWarning() << "copy_file_range error - from:" << fromInfo->uri() << "to:" << toInfo->uri() << "error:" << lastError;
                action = doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(),
                                              AbstractJobHandler::JobErrorType::kWriteError,
                                              false, lastError);
                offset_in = qMin(offset_in, offset_out);
                offset_out = offset_in;
            } else {
                workData->currentWriteSize += result;
                total -= static_cast<size_t>(result);
            }
        } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());
        checkRetry();
        if (!actionOperating(action, fromSize - offset_out, skip))
            return NextDo::kDoCopyErrorAddCancel;
    } while (offset_out != fromSize);
    // 对文件加权
    setTargetPermissions(fromInfo->uri(), toInfo->uri());
    if (!stateCheck())
        return NextDo::kDoCopyErrorAddCancel;
    if (skip && *skip)
        FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::Global::FileNotifyType::kFileAdded, toInfo->uri());
    return NextDo::kDoCopyNext;
}

bool DoCopyFileWorker::stateCheck()
{
    if (state == kPaused)
        workerWait();

    return state == kNormal;
}

void DoCopyFileWorker::workerWait()
{
    QMutexLocker locker(mutex.data());
    waitCondition->wait(mutex.data());
}

bool DoCopyFileWorker::actionOperating(const AbstractJobHandler::SupportAction action, const qint64 size, bool *skip)
{
    if (isStopped())
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
                                                                         const bool isTo,
                                                                         const QString &errorMsg)
{
    if (workData->errorOfAction.contains(error))
        return workData->errorOfAction.value(error);

    bool isSame = false, checkAgain = true;
    if (urlFrom.scheme() == Global::Scheme::kFile
        && urlFrom.scheme() == Global::Scheme::kFile) {
        auto tmF = FileUtils::bindUrlTransform(urlFrom);
        auto tmT = FileUtils::bindUrlTransform(urlTo);
        isSame = UniversalUtils::urlEquals(tmF, tmT);
        checkAgain = false;
    }

    if (isSame || (checkAgain && FileUtils::isSameFile(urlFrom, urlTo, Global::CreateFileInfoType::kCreateFileInfoSync))) {
        currentAction = AbstractJobHandler::SupportAction::kCoexistAction;
        return currentAction;
    }

    if (isStopped())
        return AbstractJobHandler::SupportAction::kCancelAction;

    // 发送错误处理 阻塞自己
    emit errorNotify(urlFrom, urlTo, error, isTo, quintptr(this), errorMsg, false);
    workerWait();

    if (isStopped())
        return AbstractJobHandler::SupportAction::kCancelAction;

    return currentAction;
}

/*!
 * \brief FileOperateBaseWorker::readAheadSourceFile Pre read source file content
 * \param fileInfo File information of source file
 */
void DoCopyFileWorker::readAheadSourceFile(const DFileInfoPointer &fileInfo)
{
    auto fromSize = fileInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
    if (fromSize <= 0)
        return;
    std::string stdStr = fileInfo->uri().path().toUtf8().toStdString();
    int fromfd = open(stdStr.data(), O_RDONLY);
    if (-1 != fromfd) {
        readahead(fromfd, 0, static_cast<size_t>(fromSize));
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
bool DoCopyFileWorker::createFileDevice(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                                        const DFileInfoPointer &needOpenInfo, QSharedPointer<DFMIO::DFile> &file,
                                        bool *skip)
{
    file.reset();
    QUrl url = needOpenInfo->uri();
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        file.reset(new DFile(url));
        if (!file) {
            fmCritical() << "Failed to create DFile object - url:" << url;
            action = doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(),
                                          AbstractJobHandler::JobErrorType::kProrogramError,
                                          url == toInfo->uri());
        }
    } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());

    checkRetry();

    auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
    if (!actionOperating(action, fromSize <= 0 ? workData->dirSize : fromSize, skip))
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
bool DoCopyFileWorker::createFileDevices(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
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
bool DoCopyFileWorker::openFiles(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                                 const QSharedPointer<DFMIO::DFile> &fromeFile, const QSharedPointer<DFMIO::DFile> &toFile,
                                 bool *skip)
{
    if (!openFile(fromInfo, toInfo, fromeFile, DFMIO::DFile::OpenFlag::kReadOnly, skip)) {
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
bool DoCopyFileWorker::openFile(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                                const QSharedPointer<DFMIO::DFile> &file, const DFMIO::DFile::OpenFlags &flags,
                                bool *skip)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        if (!file->open(flags)) {
            auto lastError = file->lastError();
            fmWarning() << "file open error, url from: " << fromInfo->uri()
                        << " url to: " << toInfo->uri() << " open flag: " << flags
                        << " error code: " << lastError.code() << " error msg: " << lastError.errorMsg();

            action = doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(),
                                          AbstractJobHandler::JobErrorType::kOpenError,
                                          file->uri() != fromInfo->uri(), lastError.errorMsg());
        }
    } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());

    checkRetry();

    auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
    if (!actionOperating(action, fromSize <= 0 ? FileUtils::getMemoryPageSize() : fromSize, skip))
        return false;
    return true;
}

bool DoCopyFileWorker::resizeTargetFile(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                                        const QSharedPointer<DFMIO::DFile> &file, bool *skip)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        if (!file->write(QByteArray())) {
            fmWarning() << "Resize target file failed - from:" << fromInfo->uri() << "to:" << toInfo->uri() << "error:" << file->lastError().errorMsg();
            action = doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(),
                                          AbstractJobHandler::JobErrorType::kResizeError, true,
                                          file->lastError().errorMsg());
        }
    } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());
    const auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
    if (!actionOperating(action, fromSize <= 0 ? workData->dirSize : fromSize, skip))
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

DoCopyFileWorker::NextDo DoCopyFileWorker::doReadFile(const DFileInfoPointer &fromInfo,
                                                      const DFileInfoPointer &toInfo,
                                                      const QSharedPointer<DFMIO::DFile> &fromDevice,
                                                      char *data, const qint64 &blockSize,
                                                      qint64 &readSize, bool *skip)
{
    readSize = 0;
    qint64 currentPos = fromDevice->pos();
    AbstractJobHandler::SupportAction actionForRead = AbstractJobHandler::SupportAction::kNoAction;
    auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();

    if (Q_UNLIKELY(!stateCheck())) {
        return NextDo::kDoCopyErrorAddCancel;
    }
    do {
        actionForRead = AbstractJobHandler::SupportAction::kNoAction;
        readSize = fromDevice->read(data, blockSize);
        if (Q_UNLIKELY(!stateCheck())) {
            return NextDo::kDoCopyErrorAddCancel;
        }

        if (Q_UNLIKELY(readSize <= 0)) {

            const qint64 fromFilePos = fromDevice->pos();
            const qint64 fromFileInfoSize = fromSize;
            if (readSize == 0 && fromFilePos == fromFileInfoSize) {
                return NextDo::kDoCopyCurrentFile;
            }

            fmWarning() << "Read error - size:" << readSize << "pos:" << fromFilePos << "expected:" << fromFileInfoSize << "file:" << fromInfo->uri();
            fromInfo->initQuerier();
            const bool fromInfoExist = fromInfo->exists();
            AbstractJobHandler::JobErrorType errortype = fromInfoExist ? AbstractJobHandler::JobErrorType::kReadError : AbstractJobHandler::JobErrorType::kNonexistenceError;
            QString errorstr = fromInfoExist ? fromDevice->lastError().errorMsg() : QString();

            actionForRead = doHandleErrorAndWait(fromInfo->uri(),
                                                 toInfo->uri(), errortype, false, errorstr);
            if (actionForRead == AbstractJobHandler::SupportAction::kRetryAction && !isStopped()) {
                // 检查当前文件是否可以访问
                AbstractJobHandler::SupportAction actionForCheck = AbstractJobHandler::SupportAction::kNoAction;
                do {
                    actionForCheck = AbstractJobHandler::SupportAction::kNoAction;
                    if (!NetworkUtils::instance()->checkFtpOrSmbBusy(fromInfo->uri())) {
                        break;
                    }
                    actionForCheck = doHandleErrorAndWait(
                            fromInfo->uri(),
                            toInfo->uri(),
                            AbstractJobHandler::JobErrorType::kCanNotAccessFile,
                            true,
                            tr("Can't access file!"));
                } while (actionForCheck == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());
                if (actionForCheck != AbstractJobHandler::SupportAction::kNoAction) {
                    if (skip)
                        *skip = actionForCheck == AbstractJobHandler::SupportAction::kSkipAction;
                    return NextDo::kDoCopyErrorAddCancel;
                }
                checkRetry();
                workData->currentWriteSize -= currentPos;
                return NextDo::kDoCopyReDoCurrentFile;
            }
        }
    } while (actionForRead == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());
    checkRetry();

    if (!actionOperating(actionForRead, fromSize - currentPos, skip))
        return NextDo::kDoCopyErrorAddCancel;

    return NextDo::kDoCopyCurrentFile;
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
DoCopyFileWorker::NextDo DoCopyFileWorker::doWriteFile(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                                                       const QSharedPointer<DFMIO::DFile> &toDevice,
                                                       const QSharedPointer<DFile> &fromDevice,
                                                       const char *data, const qint64 readSize, bool *skip)
{
    qint64 currentPos = fromDevice->pos() - readSize;
    AbstractJobHandler::SupportAction actionForWrite { AbstractJobHandler::SupportAction::kNoAction };
    qint64 sizeWrite = 0;
    qint64 surplusSize = readSize;
    const char *surplusData = data;

    do {
        actionForWrite = AbstractJobHandler::SupportAction::kNoAction;
        do {
            surplusData += sizeWrite;
            surplusSize -= sizeWrite;
            sizeWrite = toDevice->write(surplusData, surplusSize);
            if (sizeWrite > 0)
                workData->currentWriteSize += sizeWrite;
            if (Q_UNLIKELY(!stateCheck()))
                return NextDo::kDoCopyErrorAddCancel;
        } while (sizeWrite > 0 && sizeWrite < surplusSize);

        if (toDevice->lastError().code() != DFMIOErrorCode::DFM_IO_ERROR_NONE)
            fmWarning() << "Write error - size:" << sizeWrite << "error:" << toDevice->lastError().errorMsg() << "file:" << toInfo->uri();

        // 表示全部数据写入完成
        if (sizeWrite >= 0 && sizeWrite == surplusSize && toDevice->lastError().code() == DFMIOErrorCode::DFM_IO_ERROR_NONE)
            break;
        if (sizeWrite == -1 && toDevice->lastError().code() == DFMIOErrorCode::DFM_IO_ERROR_NONE) {
            fmWarning() << "write failed, but no error, maybe write empty";
            break;
        }

        actionForWrite = doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(),
                                              AbstractJobHandler::JobErrorType::kWriteError, true,
                                              toDevice->lastError().errorMsg());
        if (actionForWrite == AbstractJobHandler::SupportAction::kRetryAction && !isStopped()) {
            qint64 curWrite = 0;
            auto nextDo = doWriteFileErrorRetry(fromInfo, toInfo, toDevice, fromDevice, readSize, skip, currentPos, surplusSize, curWrite);
            checkRetry();
            return nextDo;
        }
    } while (actionForWrite == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());

    checkRetry();

    if (!actionOperating(actionForWrite, fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong() - (currentPos + readSize - surplusSize), skip))
        return NextDo::kDoCopyErrorAddCancel;

    return NextDo::kDoCopyCurrentFile;
}

DoCopyFileWorker::NextDo DoCopyFileWorker::doWriteFileErrorRetry(const DFileInfoPointer &fromInfo,
                                                                 const DFileInfoPointer &toInfo,
                                                                 const QSharedPointer<DFile> &toDevice,
                                                                 const QSharedPointer<DFile> &fromDevice,
                                                                 const qint64 readSize,
                                                                 bool *skip,
                                                                 const qint64 currentPos,
                                                                 const qint64 &surplusSize, qint64 &curWrite)
{
    Q_UNUSED(toDevice);
    Q_UNUSED(fromDevice);
    Q_UNUSED(curWrite);
    // 检查当前文件是否可以访问
    AbstractJobHandler::SupportAction actionForWrite = AbstractJobHandler::SupportAction::kNoAction;
    do {
        actionForWrite = AbstractJobHandler::SupportAction::kNoAction;
        if (!NetworkUtils::instance()->checkFtpOrSmbBusy(toInfo->uri())) {
            break;
        }
        actionForWrite = doHandleErrorAndWait(
                fromInfo->uri(),
                toInfo->uri(),
                AbstractJobHandler::JobErrorType::kCanNotAccessFile,
                true,
                tr("Can't access file!"));
    } while (actionForWrite == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());
    if (actionForWrite != AbstractJobHandler::SupportAction::kNoAction) {
        actionOperating(actionForWrite, fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong() - (currentPos + readSize - surplusSize), skip);
        return NextDo::kDoCopyErrorAddCancel;
    }

    workData->currentWriteSize -= currentPos;
    return NextDo::kDoCopyReDoCurrentFile;
}

bool DoCopyFileWorker::verifyFileIntegrity(const qint64 &blockSize, const ulong &sourceCheckSum,
                                           const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                                           QSharedPointer<DFMIO::DFile> &toDevice)
{
    if (!workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyIntegrityChecking))
        return true;
    char *data = new char[static_cast<uint>(blockSize + 1)];
    QElapsedTimer t;
    ulong targetCheckSum = adler32(0L, nullptr, 0);
    Q_FOREVER {
        qint64 size = toDevice->read(data, blockSize);

        if (Q_UNLIKELY(size <= 0)) {
            if (size == 0 && toInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong() == toDevice->pos()) {
                break;
            }

            fmWarning() << "Integrity check read failed - size:" << size << "pos:" << toDevice->pos() << "file:" << toInfo->uri() << "error:" << toDevice->lastError().errorMsg();
            AbstractJobHandler::SupportAction actionForCheckRead = doHandleErrorAndWait(fromInfo->uri(),
                                                                                        toInfo->uri(),
                                                                                        AbstractJobHandler::JobErrorType::kIntegrityCheckingError,
                                                                                        true,
                                                                                        toDevice->lastError().errorMsg());
            if (!isStopped() && AbstractJobHandler::SupportAction::kRetryAction == actionForCheckRead) {
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

    fmDebug("Time spent of integrity check of the file: %d", t.elapsed());

    if (sourceCheckSum != targetCheckSum) {
        fmWarning("Integrity check failed - source checksum: 0x%lx, target checksum: 0x%lx, file: %s",
                  sourceCheckSum, targetCheckSum, qPrintable(toInfo->uri().toString()));
        AbstractJobHandler::SupportAction actionForCheck = doHandleErrorAndWait(fromInfo->uri(),
                                                                                toInfo->uri(),
                                                                                AbstractJobHandler::JobErrorType::kIntegrityCheckingError,
                                                                                true);
        return actionForCheck == AbstractJobHandler::SupportAction::kSkipAction;
    }

    return true;
}

void DoCopyFileWorker::checkRetry()
{
    if (!workData->singleThread && retry && !isStopped()) {
        retry = false;
        emit retryErrSuccess(quintptr(this));
    }
}

bool DoCopyFileWorker::isStopped()
{
    return state == kStopped;
}

/*!
 * \brief FileOperateBaseWorker::setTargetPermissions Set permissions on the target file
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 */
void DoCopyFileWorker::setTargetPermissions(const QUrl &fromUrl, const QUrl &toUrl)
{
    const auto &fromInfo = InfoFactory::create<FileInfo>(fromUrl, Global::CreateFileInfoType::kCreateFileInfoSync);
    const auto &toInfo = InfoFactory::create<FileInfo>(toUrl, Global::CreateFileInfoType::kCreateFileInfoSync);
    if (!DeviceUtils::supportSetPermissionsDevice(toInfo->urlOf(UrlInfoType::kUrl)))
        return;
    // 修改文件修改时间
    localFileHandler->setFileTime(toInfo->urlOf(UrlInfoType::kUrl), fromInfo->timeOf(TimeInfoType::kLastRead).value<QDateTime>(), fromInfo->timeOf(TimeInfoType::kLastModified).value<QDateTime>());
    QFileDevice::Permissions permissions = fromInfo->permissions();
    QString path = fromInfo->urlOf(UrlInfoType::kUrl).path();
    // 权限为0000时，源文件已经被删除，无需修改新建的文件的权限为0000
    if (permissions != 0000)
        localFileHandler->setPermissions(toInfo->urlOf(UrlInfoType::kUrl), permissions);
}

/*!
 * \brief DoCopyFileWorker::openDestinationFile Open destination file with preferred write mode
 * \param dest Destination file path
 * \param preferredMode Preferred write mode (Normal or Direct)
 * \return FileWriter structure with file descriptor and actual mode
 */
DoCopyFileWorker::FileWriter DoCopyFileWorker::openDestinationFile(const QString &dest, WriteMode preferredMode)
{
    int destFd = -1;
    WriteMode actualMode = preferredMode;

    if (preferredMode == WriteMode::Direct) {
        // Try O_DIRECT first
        destFd = open(dest.toLocal8Bit().constData(), O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT, 0666);

        if (destFd < 0 && (errno == EINVAL || errno == ENOTSUP)) {
            // O_DIRECT not supported, fallback to normal mode
            fmDebug() << "O_DIRECT not supported for destination, falling back to normal mode - file:" << dest << "error:" << strerror(errno);
            actualMode = WriteMode::Normal;
        }
    }

    if (destFd < 0) {
        // Open in normal mode
        destFd = open(dest.toLocal8Bit().constData(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (destFd < 0) {
            fmWarning() << "Failed to open destination file - file:" << dest << "error:" << strerror(errno);
        }
        actualMode = WriteMode::Normal;
    }

    fmDebug() << "Using" << (actualMode == WriteMode::Direct ? "O_DIRECT" : "normal")
              << "mode for file:" << dest;

    return FileWriter(destFd, actualMode);
}

/*!
 * \brief DoCopyFileWorker::reopenDestinationFileForResume Reopen destination file for resume
 * \param dest Destination file path
 * \param preferredMode Preferred write mode
 * \return FileWriter structure with reopened file descriptor
 */
DoCopyFileWorker::FileWriter DoCopyFileWorker::reopenDestinationFileForResume(const QString &dest, WriteMode preferredMode)
{
    int destFd = -1;
    WriteMode actualMode = preferredMode;

    if (preferredMode == WriteMode::Direct) {
        // Try O_DIRECT first - use O_WRONLY (no truncate) and seek to end
        destFd = open(dest.toLocal8Bit().constData(), O_WRONLY | O_DIRECT, 0666);

        if (destFd < 0 && (errno == EINVAL || errno == ENOTSUP)) {
            // O_DIRECT not supported, fallback to normal mode
            fmDebug() << "O_DIRECT not supported for resume, falling back to normal mode - file:" << dest << "error:" << strerror(errno);
            actualMode = WriteMode::Normal;
        }
    }

    if (destFd < 0) {
        // Open in normal mode (no truncate)
        destFd = open(dest.toLocal8Bit().constData(), O_WRONLY, 0666);
        if (destFd < 0) {
            fmWarning() << "Failed to reopen destination file for resume - file:" << dest << "error:" << strerror(errno);
        }
        actualMode = WriteMode::Normal;
    }

    if (destFd >= 0) {
        // Seek to end of file to continue writing where we left off
        if (lseek(destFd, 0, SEEK_END) < 0) {
            fmWarning() << "Failed to seek to end of file for resume - file:" << dest << "error:" << strerror(errno);
            close(destFd);
            return FileWriter(-1, actualMode);
        }
    }

    fmDebug() << "Reopened for resume using" << (actualMode == WriteMode::Direct ? "O_DIRECT" : "normal")
              << "mode for file:" << dest;

    return FileWriter(destFd, actualMode);
}

/*!
 * \brief DoCopyFileWorker::allocateAlignedBuffer Allocate aligned buffer for O_DIRECT
 * \param size Buffer size
 * \param alignment Alignment requirement
 * \return Aligned buffer pointer or nullptr on failure
 */
char *DoCopyFileWorker::allocateAlignedBuffer(size_t size, size_t alignment)
{
    char *buffer = nullptr;
    if (posix_memalign((void **)&buffer, alignment, size) != 0) {
        fmWarning() << "Failed to allocate aligned buffer - size:" << size << "alignment:" << alignment << "error:" << strerror(errno);
        return nullptr;
    }
    return buffer;
}

/*!
 * \brief DoCopyFileWorker::handlePauseResume Handle pause and resume during copy
 * \param writer FileWriter to handle
 * \param dest Destination file path
 * \param skip Skip flag
 * \return true if successful, false otherwise
 */
bool DoCopyFileWorker::handlePauseResume(FileWriter &writer, const QString &dest, bool *skip)
{
    // Sync data before pausing
    if (writer.fd >= 0) {
        // Determine target filesystem type for appropriate sync strategy
        const QUrl &destUrl = QUrl::fromLocalFile(dest);
        const QString &targetFsType = dfmio::DFMUtils::fsTypeFromUrl(destUrl);

        if (targetFsType.toLower().contains("fuse")) {
            // For fuse filesystems, avoid fsync as it may cause performance issues
            // or hang in some fuse implementations
            fmInfo() << "Skipping fsync for fuse filesystem:" << targetFsType;
        } else {
            // For non-fuse filesystems, use fsync to ensure data is written to device
            // before pausing, providing better data integrity
            fmInfo() << "Performing fsync for non-fuse filesystem:" << targetFsType;
            if (syncfs(writer.fd) != 0) {
                fmWarning() << "fsync failed for file:" << dest << "error:" << strerror(errno);
                // Continue anyway, as this is not a fatal error
            }
        }
        close(writer.fd);
    }

    // Wait while paused
    workerWait();

    if (isStopped()) {
        return false;
    }

    // Reopen for resume
    FileWriter newWriter = reopenDestinationFileForResume(dest, writer.mode);
    if (newWriter.fd < 0) {
        return false;
    }

    writer = newWriter;
    return true;
}

/*!
 * \brief DoCopyFileWorker::shouldFallbackFromCopyFileRange Check if copy_file_range error should trigger fallback
 * \param errorCode errno from copy_file_range
 * \return true if should fallback silently, false if should show error dialog
 */
bool DoCopyFileWorker::shouldFallbackFromCopyFileRange(int errorCode) const
{
    // These errors indicate copy_file_range is not supported or not suitable
    // and we should fallback to other methods silently
    switch (errorCode) {
    case ENOSYS:   // System call not implemented
    case EXDEV:   // Cross-device copy (different filesystems)
    case EINVAL:   // Invalid arguments (often filesystem doesn't support it)
    case EBADF:   // Bad file descriptor (sometimes indicates unsupported scenario)
    case EOPNOTSUPP:   // Operation not supported (ENOTSUP is often the same value)
        return true;
    default:
        // Other errors (ENOSPC, EACCES, EIO, etc.) are real errors that should be reported
        return false;
    }
}

/*!
 * \brief DoCopyFileWorker::mapSystemErrorToJobError Map system errno to JobErrorType
 * \param systemErrno System error number
 * \param isWriteError Whether this is a write error (vs read error)
 * \return Corresponding JobErrorType
 */
AbstractJobHandler::JobErrorType DoCopyFileWorker::mapSystemErrorToJobError(int systemErrno, bool isWriteError)
{
    switch (systemErrno) {
    case ENOSPC:   // No space left on device
        return AbstractJobHandler::JobErrorType::kNotEnoughSpaceError;
    case EACCES:   // Permission denied
    case EPERM:   // Operation not permitted
        return AbstractJobHandler::JobErrorType::kPermissionDeniedError;
    case EIO:   // I/O error
        return isWriteError ? AbstractJobHandler::JobErrorType::kWriteError
                            : AbstractJobHandler::JobErrorType::kReadError;
    case EBADF:   // Bad file descriptor
    case EINVAL:   // Invalid argument
        return AbstractJobHandler::JobErrorType::kProrogramError;
    case ENOENT:   // No such file or directory
        return AbstractJobHandler::JobErrorType::kNonexistenceError;
    default:
        // Default to read/write error based on operation type
        return isWriteError ? AbstractJobHandler::JobErrorType::kWriteError
                            : AbstractJobHandler::JobErrorType::kReadError;
    }
}
