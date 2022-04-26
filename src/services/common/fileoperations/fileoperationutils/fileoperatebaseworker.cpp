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

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

static const quint32 kMoveMaxBufferLength { 1024 * 1024 * 1 };

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
AbstractJobHandler::SupportAction FileOperateBaseWorker::doHandleErrorAndWait(const QUrl &fromUrl, const QUrl &toUrl,
                                                                              const QUrl &errorUrl,
                                                                              const AbstractJobHandler::JobErrorType &error,
                                                                              const QString &errorMsg)
{
    setStat(AbstractJobHandler::JobState::kPauseState);
    emitErrorNotify(fromUrl, toUrl, error, errorUrl.toString() + errorMsg);

    handlingErrorQMutex.lock();
    handlingErrorCondition.wait(&handlingErrorQMutex);
    handlingErrorQMutex.unlock();

    return currentAction;
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
bool FileOperateBaseWorker::createFileDevice(const QUrl &fromUrl,
                                             const QUrl &toUrl,
                                             const AbstractFileInfoPointer &needOpenInfo,
                                             QSharedPointer<DFile> &file, bool *result)
{
    file.reset(nullptr);
    QUrl url = needOpenInfo->url();
    AbstractJobHandler::SupportAction actionForCreatDevice = AbstractJobHandler::SupportAction::kNoAction;
    QSharedPointer<DIOFactory> factory { nullptr };
    do {
        factory = produceQSharedIOFactory(url.scheme(), static_cast<QUrl>(url));
        if (!factory) {
            actionForCreatDevice = doHandleErrorAndWait(fromUrl, toUrl, needOpenInfo->url(), AbstractJobHandler::JobErrorType::kDfmIoError, QObject::tr("create dfm io factory failed!"));
        }
    } while (!isStopped() && actionForCreatDevice == AbstractJobHandler::SupportAction::kRetryAction);

    if (actionForCreatDevice != AbstractJobHandler::SupportAction::kNoAction) {
        *result = actionForCreatDevice == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    do {
        file = factory->createFile();
        if (!file) {
            actionForCreatDevice = doHandleErrorAndWait(fromUrl, toUrl, needOpenInfo->url(), AbstractJobHandler::JobErrorType::kDfmIoError, QObject::tr("create dfm io dfile failed!"));
        }
    } while (!isStopped() && actionForCreatDevice == AbstractJobHandler::SupportAction::kRetryAction);

    if (actionForCreatDevice != AbstractJobHandler::SupportAction::kNoAction) {
        *result = actionForCreatDevice == AbstractJobHandler::SupportAction::kSkipAction;
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
bool FileOperateBaseWorker::createFileDevices(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, QSharedPointer<DFile> &fromeFile, QSharedPointer<DFile> &toFile, bool *result)
{
    if (!createFileDevice(fromInfo->url(), toInfo->url(), fromInfo, fromeFile, result))
        return false;
    if (!createFileDevice(fromInfo->url(), toInfo->url(), toInfo, toFile, result))
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
bool FileOperateBaseWorker::openFiles(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, const QSharedPointer<DFile> &fromeFile, const QSharedPointer<DFile> &toFile, bool *result)
{
    if (fromInfo->size() > 0 && !openFile(fromInfo->url(), toInfo->url(), fromInfo, fromeFile, DFile::OpenFlag::ReadOnly, result)) {
        return false;
    }

    if (!openFile(fromInfo->url(), toInfo->url(), toInfo, toFile, DFile::OpenFlag::WriteOnly | DFile::OpenFlag::Truncate, result)) {
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
bool FileOperateBaseWorker::openFile(const QUrl &fromUrl, const QUrl &toUrl, const AbstractFileInfoPointer &fileInfo, const QSharedPointer<DFile> &file, const DFile::OpenFlags &flags, bool *result)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    do {
        if (!file->open(flags)) {
            action = doHandleErrorAndWait(fromUrl, toUrl, fileInfo->url(), AbstractJobHandler::JobErrorType::kDfmIoError, QObject::tr("create dfm io dfile failed!"));
        }
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);
    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        *result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
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
bool FileOperateBaseWorker::doReadFile(const QUrl &fromUrl, const QUrl &toUrl, const AbstractFileInfoPointer &fileInfo, const QSharedPointer<DFile> &fromDevice, char *data, const qint64 &blockSize, qint64 &readSize, bool *result)
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
            if (readSize == 0 && fileInfo->size() == fromDevice->pos()) {
                return true;
            }
            fileInfo->refresh();
            AbstractJobHandler::JobErrorType errortype = fileInfo->exists() ? AbstractJobHandler::JobErrorType::kReadError : AbstractJobHandler::JobErrorType::kNonexistenceError;
            QString errorstr = fileInfo->exists() ? QString(QObject::tr("DFileCopyMoveJob", "Failed to read the file, cause: %1")).arg("to something!") : QString();

            actionForRead = doHandleErrorAndWait(fromUrl, toUrl, fileInfo->url(), errortype, errorstr);

            if (actionForRead == AbstractJobHandler::SupportAction::kRetryAction) {
                if (!fromDevice->seek(currentPos)) {
                    AbstractJobHandler::SupportAction actionForReadSeek = doHandleErrorAndWait(fromUrl, toUrl, fileInfo->url(),
                                                                                               AbstractJobHandler::JobErrorType::kSeekError);
                    *result = actionForReadSeek == AbstractJobHandler::SupportAction::kSkipAction;
                    return false;
                }
            }
        }
    } while (!isStopped() && actionForRead == AbstractJobHandler::SupportAction::kRetryAction);

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
bool FileOperateBaseWorker::doWriteFile(const QUrl &fromUrl,
                                        const QUrl &toUrl,
                                        const AbstractFileInfoPointer &fileInfo,
                                        const QSharedPointer<DFile> &toDevice,
                                        const char *data, const qint64 &readSize, bool *result)
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

        actionForWrite = doHandleErrorAndWait(fromUrl, toUrl, fileInfo->url(), AbstractJobHandler::JobErrorType::kWriteError, errorstr);
        if (actionForWrite == AbstractJobHandler::SupportAction::kRetryAction) {
            if (!toDevice->seek(currentPos)) {
                AbstractJobHandler::SupportAction actionForWriteSeek = doHandleErrorAndWait(fromUrl, toUrl, fileInfo->url(),
                                                                                            AbstractJobHandler::JobErrorType::kSeekError);
                *result = actionForWriteSeek == AbstractJobHandler::SupportAction::kSkipAction;

                return false;
            }
        }
    } while (!isStopped() && actionForWrite == AbstractJobHandler::SupportAction::kRetryAction);

    if (actionForWrite != AbstractJobHandler::SupportAction::kNoAction) {
        *result = actionForWrite == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    if (sizeWrite > 0) {
        toDevice->flush();
    }

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
            action = doHandleErrorAndWait(fromUrl, toUrl, fromUrl, AbstractJobHandler::JobErrorType::kNotEnoughSpaceError);
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
            action = doHandleErrorAndWait(fromUrl, QUrl(), fromUrl, AbstractJobHandler::JobErrorType::kDeleteFileError, handler->errorString());
        }
    } while (isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

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
 * \brief FileOperateBaseWorker::copyFile Copy file
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 * \param reslut result Output parameter: whether skip
 * \return Whether the copied file is complete
 */
bool FileOperateBaseWorker::copyFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *result)
{
    //预先读取
    readAheadSourceFile(fromInfo);

    // 创建文件的divice
    QSharedPointer<DFile> fromDevice { nullptr }, toDevice { nullptr };
    if (!createFileDevices(fromInfo, toInfo, fromDevice, toDevice, result))
        return result;

    // 打开文件并创建
    if (!openFiles(fromInfo, toInfo, fromDevice, toDevice, result))
        return result;

    // 源文件大小如果为0
    if (fromInfo->size() <= 0) {
        // 对文件加权
        setTargetPermissions(fromInfo, toInfo);
        return true;
    }
    // 循环读取和写入文件，拷贝
    qint64 blockSize = fromInfo->size() > kMoveMaxBufferLength ? kMoveMaxBufferLength : fromInfo->size();
    char *data = new char[blockSize + 1];
    qint64 sizeRead = 0;
    do {
        if (!doReadFile(fromInfo->url(), toInfo->url(), fromInfo, fromDevice, data, blockSize, sizeRead, result)) {
            delete[] data;
            data = nullptr;
            return result;
        }

        if (!doWriteFile(fromInfo->url(), toInfo->url(), toInfo, toDevice, data, sizeRead, result)) {
            delete[] data;
            data = nullptr;
            return result;
        }
        toInfo->refresh(DFMIO::DFileInfo::AttributeID::StandardSize, toDevice->size());
    } while (fromDevice->pos() != fromInfo->size());

    delete[] data;
    data = nullptr;

    // 对文件加权
    setTargetPermissions(fromInfo, toInfo);
    if (Q_UNLIKELY(!stateCheck())) {
        return false;
    }

    FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::FileNotifyType::kFileAdded, toInfo->url());
    return true;
}

/*!
 * \brief FileOperateBaseWorker::copyDir Copy dir
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 * \param reslut result Output parameter: whether skip
 * \return Whether the copied dir is complete
 */
bool FileOperateBaseWorker::copyDir(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *result)
{
    // 检查文件的一些合法性，源文件是否存在，创建新的目标目录名称，检查新创建目标目录名称是否存在
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    QFileDevice::Permissions permissions = fromInfo->permissions();
    if (!toInfo->exists()) {
        do {
            if (handler->mkdir(toInfo->url())) {
                break;
            }
            action = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), fromInfo->url(), AbstractJobHandler::JobErrorType::kMkdirError, QString(QObject::tr("Fail to create symlink, cause: %1")).arg(handler->errorString()));
        } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);
        if (AbstractJobHandler::SupportAction::kNoAction != action) {
            // skip write size += all file size in sources dir
            *result = AbstractJobHandler::SupportAction::kSkipAction == action;
            return false;
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
        doHandleErrorAndWait(fromInfo->url(), toInfo->url(), fromInfo->url(), AbstractJobHandler::JobErrorType::kProrogramError, QString(QObject::tr("create dir's iterator failed, case : %1")).arg(error));
        return false;
    }

    while (iterator->hasNext()) {
        if (!stateCheck()) {
            return false;
        }

        const QUrl &url = iterator->next();
        Q_UNUSED(url);
        const AbstractFileInfoPointer &info = iterator->fileInfo();
        if (!copyFile(info, toInfo, result)) {
            if (result)
                continue;
            return false;
        }
    }

    bool ret = handler->setPermissions(toInfo->url(), permissions);
    return ret;
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
    if (!doCheckFile(fromInfo, toInfo, newTargetInfo, result))
        return ok;

    bool oldExist = newTargetInfo->exists();

    if (fromInfo->isSymLink()) {
        ok = creatSystemLink(fromInfo, newTargetInfo, result);
        if (ok)
            ok = deleteFile(fromInfo->url(), result);
    } else if (fromInfo->isDir()) {
        ok = copyDir(fromInfo, newTargetInfo, result);
        if (ok)
            ok = deleteDir(fromInfo->url(), result);
    } else {
        const QUrl &url = newTargetInfo->url();
        FileUtils::cacheCopyingFileUrl(url);
        ok = copyFile(fromInfo, newTargetInfo, result);
        if (ok)
            ok = deleteFile(fromInfo->url(), result);
        FileUtils::removeCopyingFileUrl(url);
    }

    if (!isConvert && !oldExist && newTargetInfo->exists() && targetInfo == toInfo) {
        completeFiles.append(fromInfo->url());
        completeFiles.append(newTargetInfo->url());
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
bool FileOperateBaseWorker::doCheckFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, AbstractFileInfoPointer &newTargetInfo, bool *result)
{
    // 检查目标文件的文件信息
    if (!toInfo) {
        doHandleErrorAndWait(fromInfo->url(), QUrl(), QUrl(), AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }

    AbstractFileInfoPointer toInfoDir = toInfo;
    if (!toInfo->isDir()) {
        toInfoDir = InfoFactory::create<AbstractFileInfo>(toInfo->parentUrl());
    }

    // 检查源文件的文件信息
    if (!fromInfo) {
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(QUrl(), toInfoDir->url(), QUrl(), AbstractJobHandler::JobErrorType::kProrogramError);
        *result = AbstractJobHandler::SupportAction::kSkipAction != action;
        return false;
    }
    // 检查源文件是否存在
    if (!fromInfo->exists()) {
        AbstractJobHandler::JobErrorType errortype = (fromInfo->path().startsWith("/root/") && !toInfoDir->path().startsWith("/root/")) ? AbstractJobHandler::JobErrorType::kPermissionError
                                                                                                                                        : AbstractJobHandler::JobErrorType::kNonexistenceError;
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->url(), toInfoDir->url(), fromInfo->url(), errortype);

        *result = AbstractJobHandler::SupportAction::kSkipAction != action;
        return false;
    }
    // 检查目标文件是否存在
    if (!toInfoDir->exists()) {
        AbstractJobHandler::JobErrorType errortype = (fromInfo->path().startsWith("/root/") && !toInfoDir->path().startsWith("/root/")) ? AbstractJobHandler::JobErrorType::kPermissionError
                                                                                                                                        : AbstractJobHandler::JobErrorType::kNonexistenceError;
        doHandleErrorAndWait(fromInfo->url(), toInfoDir->url(), toInfoDir->url(), errortype);
        return false;
    }
    // 特殊文件判断
    switch (fromInfo->fileType()) {
    case AbstractFileInfo::kCharDevice:
    case AbstractFileInfo::kBlockDevice:
    case AbstractFileInfo::kFIFOFile:
    case AbstractFileInfo::kSocketFile: {

        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->url(), toInfoDir->url(), fromInfo->url(), AbstractJobHandler::JobErrorType::kSpecialFileError);
        return AbstractJobHandler::SupportAction::kSkipAction == action;
    }
    default:
        break;
    }

    // 创建新的目标文件并做检查
    QString fileNewName = fromInfo->fileName();
    newTargetInfo.reset(nullptr);
    if (!doCheckNewFile(fromInfo, toInfoDir, newTargetInfo, fileNewName, result))
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
bool FileOperateBaseWorker::creatSystemLink(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *result)
{
    // 创建链接文件
    AbstractFileInfoPointer newFromInfo = fromInfo;

    AbstractJobHandler::SupportAction actionForlink { AbstractJobHandler::SupportAction::kNoAction };

    do {
        if (handler->createSystemLink(newFromInfo->url(), toInfo->url())) {
            return true;
        }
        actionForlink = doHandleErrorAndWait(fromInfo->url(), toInfo->url(), fromInfo->url(), AbstractJobHandler::JobErrorType::kSymlinkError, QString(QObject::tr("Fail to create symlink, cause: %1")).arg(handler->errorString()));
    } while (!isStopped() && actionForlink == AbstractJobHandler::SupportAction::kRetryAction);
    *result = actionForlink == AbstractJobHandler::SupportAction::kSkipAction;
    return actionForlink == AbstractJobHandler::SupportAction::kNoAction;
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
bool FileOperateBaseWorker::doCheckNewFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, AbstractFileInfoPointer &newTargetInfo, QString &fileNewName, bool *result)
{
    AbstractFileInfoPointer toInfoDir = toInfo;
    if (!toInfoDir->isDir()) {
        toInfoDir = InfoFactory::create<AbstractFileInfo>(toInfoDir->parentUrl());
    }

    fileNewName = formatFileName(fileNewName);
    // 创建文件的名称
    QUrl newTargetUrl = toInfoDir->url();
    const QString &newTargetPath = newTargetUrl.path();

    QString newPath = newTargetPath.endsWith("/") ? newTargetPath + fileNewName
                                                  : newTargetPath + "/" + fileNewName;

    newTargetUrl.setPath(newPath);

    newTargetInfo.reset(nullptr);
    newTargetInfo = InfoFactory::create<AbstractFileInfo>(newTargetUrl);

    if (!newTargetInfo) {
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->url(), toInfoDir->url(), newTargetUrl, AbstractJobHandler::JobErrorType::kProrogramError);

        *result = AbstractJobHandler::SupportAction::kSkipAction != action;
        return false;
    }
    newTargetInfo->refresh();

    if (newTargetInfo->exists()) {
        if (!jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyToSelf) && FileOperationsUtils::isAncestorUrl(fromInfo->url(), newTargetUrl)) {
            AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->url(), toInfoDir->url(), newTargetUrl, AbstractJobHandler::JobErrorType::kTargetIsSelfError);

            if (AbstractJobHandler::SupportAction::kSkipAction == action) {
                *result = AbstractJobHandler::SupportAction::kSkipAction != action;
                return false;
            }

            if (action != AbstractJobHandler::SupportAction::kEnforceAction) {
                return false;
            }
        };
        bool fromIsFile = fromInfo->isFile() || fromInfo->isSymLink();
        bool newTargetIsFile = newTargetInfo->isFile() || newTargetInfo->isSymLink();
        AbstractJobHandler::JobErrorType errortype = newTargetIsFile ? AbstractJobHandler::JobErrorType::kFileExistsError
                                                                     : AbstractJobHandler::JobErrorType::kDirectoryExistsError;
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->url(), newTargetInfo->url(), newTargetUrl, errortype);
        switch (action) {
        case AbstractJobHandler::SupportAction::kReplaceAction:
            if (newTargetUrl == fromInfo->url()) {
                return true;
            }

            if (fromIsFile && fromIsFile == newTargetIsFile) {
                break;
            } else {
                // TODO:: something is doing here

                return false;
            }
        case AbstractJobHandler::SupportAction::kMergeAction:
            if (!fromIsFile && fromIsFile == newTargetIsFile) {
                break;
            } else {
                // TODO:: something is doing here

                return false;
            }
        case AbstractJobHandler::SupportAction::kNoAction:
        case AbstractJobHandler::SupportAction::kSkipAction: {
            *result = true;
            return false;
        }
        case AbstractJobHandler::SupportAction::kCoexistAction: {
            fileNewName = getNonExistFileName(fromInfo, toInfoDir);
            if (fileNewName.isEmpty())
                return false;

            bool ok = doCheckNewFile(fromInfo, toInfoDir, newTargetInfo, fileNewName, result);

            return ok;
        }
        default:
            return false;
        }
        return true;
    }

    return true;
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
