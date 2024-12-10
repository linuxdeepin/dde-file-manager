// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
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
    auto fileOpsAll = fileOps.listByLock();
    for (auto op : fileOpsAll) {
        op->cancel();
    }
}

void DoCopyFileWorker::skipMemcpyBigFile(const QUrl url)
{
    memcpySkipUrl = url;
}
// main thread using
void DoCopyFileWorker::operateAction(const AbstractJobHandler::SupportAction action)
{
    retry = !workData->signalThread && AbstractJobHandler::SupportAction::kRetryAction == action;
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
        ret = op->copyFile(toUrl, flag, progressCallback, data);
        action = AbstractJobHandler::SupportAction::kNoAction;
        if (!ret) {
            auto lastError = op->lastError().errorMsg();
            fmWarning() << "file copy error, url from: " << fromUrl
                        << " url to: " << toUrl
                        << " error code: " << op->lastError().code() << " error msg: " << lastError;

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

void DoCopyFileWorker::syncBlockFile(const DFileInfoPointer toInfo)
{
    if (!workData->isBlockDevice)
        return;
    std::string stdStr = toInfo->uri().path().toUtf8().toStdString();
    int tofd = open(stdStr.data(), O_RDONLY);
    if (-1 != tofd) {
        syncfs(tofd);
        close(tofd);
    }
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
    // 创建文件的divice
    QSharedPointer<DFMIO::DFile> fromDevice { nullptr }, toDevice { nullptr };
    if (!createFileDevices(fromInfo, toInfo, fromDevice, toDevice, skip))
        return NextDo::kDoCopyErrorAddCancel;
    // 打开文件并创建
    if (!openFiles(fromInfo, toInfo, fromDevice, toDevice, skip))
        return NextDo::kDoCopyErrorAddCancel;
    // 源文件大小如果为0
    auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
    if (fromSize <= 0) {
        // 对文件加权
        setTargetPermissions(fromInfo->uri(), toInfo->uri());
        workData->zeroOrlinkOrDirWriteSize += FileUtils::getMemoryPageSize();
        FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::Global::FileNotifyType::kFileAdded, toInfo->uri());
        if (workData->exBlockSyncEveryWrite || ProtocolUtils::isSMBFile(toInfo->uri()))
            syncBlockFile(toInfo);
        return NextDo::kDoCopyNext;
    }
    // resize target file
    if (workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyResizeDestinationFile) && !resizeTargetFile(fromInfo, toInfo, toDevice, skip))
        return NextDo::kDoCopyErrorAddCancel;
    // 循环读取和写入文件，拷贝
    int toFd = -1;
    auto toIsSmb = ProtocolUtils::isSMBFile(toInfo->uri());
    if (workData->exBlockSyncEveryWrite || toIsSmb)
        toFd = open(toInfo->uri().path().toUtf8().toStdString().data(), O_RDONLY);
    qint64 blockSize = fromSize > kMaxBufferLength ? kMaxBufferLength : fromSize;
    char *data = new char[static_cast<uint>(blockSize + 1)];
    uLong sourceCheckSum = adler32(0L, nullptr, 0);
    qint64 sizeRead = 0;

    do {
        auto nextReadDo = doReadFile(fromInfo, toInfo, fromDevice, data, blockSize, sizeRead, skip);
        if (nextReadDo != NextDo::kDoCopyCurrentFile) {
            delete[] data;
            data = nullptr;
            if (toFd > 0)
                close(toFd);
            return nextReadDo;
        }
        auto nextDo = doWriteFile(fromInfo, toInfo, toDevice, fromDevice, data, sizeRead, skip);
        if (nextDo != NextDo::kDoCopyCurrentFile) {
            delete[] data;
            data = nullptr;
            if (toFd > 0)
                close(toFd);
            return nextDo;
        }

        if (Q_LIKELY(workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyIntegrityChecking))) {
            sourceCheckSum = adler32(sourceCheckSum, reinterpret_cast<Bytef *>(data), static_cast<uInt>(sizeRead));
        }

        // 执行同步策略
        if ((workData->exBlockSyncEveryWrite || toIsSmb) && toFd > 0)
            syncfs(toFd);

    } while (fromDevice->pos() != fromSize);

    delete[] data;
    data = nullptr;

    // 执行同步策略
    if ((workData->exBlockSyncEveryWrite || toIsSmb) && toFd > 0)
        syncfs(toFd);

    if (toFd > 0)
        close(toFd);

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
        if (workData->exBlockSyncEveryWrite || ProtocolUtils::isSMBFile(toInfo->uri()))
            syncfs(targetFd);
        return NextDo::kDoCopyNext;
    }
    // 循环读取和写入文件，拷贝
    auto toIsSmb = ProtocolUtils::isSMBFile(toInfo->uri());
    size_t blockSize = static_cast<size_t>(fromSize > kMaxBufferLength ? kMaxBufferLength : fromSize);
    off_t offset_in = 0;
    off_t offset_out = 0;
    ssize_t result = -1;
    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    do {
        if (Q_UNLIKELY(!stateCheck()))
            return NextDo::kDoCopyErrorAddCancel;
        do {
            if (Q_UNLIKELY(!stateCheck()))
                return NextDo::kDoCopyErrorAddCancel;
            result = copy_file_range(sourcFd, &offset_in, targetFd, &offset_out, blockSize, 0);
            if (result < 0) {
                auto lastError = strerror(errno);
                fmWarning() << "copy file range error, url from: " << fromInfo->uri()
                            << " url to: " << toInfo->uri() << " error msg: " << lastError;
                action = doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(),
                                              AbstractJobHandler::JobErrorType::kWriteError,
                                              false, lastError);
                offset_in = qMin(offset_in, offset_out);
                offset_out = offset_in;
            } else {
                workData->currentWriteSize += result;
            }
        } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());
        checkRetry();
        if (!actionOperating(action, fromSize - offset_out, skip))
            return NextDo::kDoCopyErrorAddCancel;
        // 执行同步策略
        if (workData->exBlockSyncEveryWrite || toIsSmb)
            syncfs(targetFd);
    } while (offset_out != fromSize);
    // 执行同步策略
    if (workData->exBlockSyncEveryWrite || toIsSmb)
        syncfs(targetFd);
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
            fmCritical() << "create dfm io dfile failed! url = " << url;
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

            fmWarning() << "read size <=0, size: " << readSize << " from file pos: " << fromFilePos << " from file info size: " << fromFileInfoSize;
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
                            "Can't access file!");
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
            qCritical() << " ====== write size = " << sizeWrite << toDevice->lastError().errorMsg();

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

    if (workData->needSyncEveryRW && sizeWrite > 0) {
        toDevice->flush();
    }

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
                "Can't access file!");
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
        fmWarning("Failed on file integrity checking, source file: 0x%lx, target file: 0x%lx", sourceCheckSum, targetCheckSum);
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
    if (!workData->signalThread && retry && !isStopped()) {
        retry = false;
        emit retryErrSuccess(quintptr(this));
    }
}

bool DoCopyFileWorker::isStopped()
{
    return state == kStoped;
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
    //权限为0000时，源文件已经被删除，无需修改新建的文件的权限为0000
    if (permissions != 0000)
        localFileHandler->setPermissions(toInfo->urlOf(UrlInfoType::kUrl), permissions);
}
