// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileoperatebaseworker.h"
#include "fileoperations/fileoperationutils/fileoperationsutils.h"
#include "workerdata.h"

#include <dfm-base/interfaces/abstractdiriterator.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/file/local/localfilehandler.h>

#include <dfm-io/dfmio_utils.h>
#include <dfm-io/denumerator.h>

#include <QMutex>
#include <QDateTime>
#include <QApplication>
#include <QProcess>
#include <QtConcurrent>

#include <fcntl.h>
#include <unistd.h>
#include <zlib.h>
#include <syscall.h>
#include <sys/stat.h>
#include <sys/mman.h>

DPFILEOPERATIONS_USE_NAMESPACE
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
                                                                              const AbstractJobHandler::JobErrorType &error, const bool isTo,
                                                                              const QString &errorMsg, const bool errorMsgAll)
{
    if (workData->errorOfAction.contains(error)) {
        currentAction = workData->errorOfAction.value(error);
        return currentAction;
    }

    if (FileUtils::isSameFile(urlFrom, urlTo, Global::CreateFileInfoType::kCreateFileInfoSync)) {
        currentAction = AbstractJobHandler::SupportAction::kCoexistAction;
        return currentAction;
    }

    // 发送错误处理 阻塞自己
    emitErrorNotify(urlFrom, urlTo, error, isTo, quintptr(this), errorMsg, errorMsgAll);
    pause();
    waitCondition.wait(&mutex);
    if (isStopped())
        return AbstractJobHandler::SupportAction::kCancelAction;

    return currentAction;
}

void FileOperateBaseWorker::emitSpeedUpdatedNotify(const qint64 &writSize)
{
    JobInfoPointer info(new QMap<quint8, QVariant>);
    qint64 elTime = 1;
    if (speedtimer) {
        elTime = speedtimer->elapsed() == 0 ? 1 : speedtimer->elapsed();
        elTime += elapsed;
    }

    qint64 speed = currentState == AbstractJobHandler::JobState::kRunningState
            ? writSize * 1000 / (elTime) : 0;
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(jobType));
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobStateKey, QVariant::fromValue(currentState));
    info->insert(AbstractJobHandler::NotifyInfoKey::kSpeedKey, QVariant::fromValue(speed));
    info->insert(AbstractJobHandler::NotifyInfoKey::kRemindTimeKey, QVariant::fromValue(speed == 0 ? -1 : (sourceFilesTotalSize - writSize) / speed));

    emit stateChangedNotify(info);
    emit speedUpdatedNotify(info);
}

/*!
 * \brief FileOperateBaseWorker::setTargetPermissions Set permissions on the target file
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 */
void FileOperateBaseWorker::setTargetPermissions(const QUrl &fromUrl, const QUrl &toUrl)
{
    // 修改文件修改时间
    const auto &fromInfo = InfoFactory::create<FileInfo>(fromUrl, Global::CreateFileInfoType::kCreateFileInfoSync);
    const auto &toInfo = InfoFactory::create<FileInfo>(toUrl, Global::CreateFileInfoType::kCreateFileInfoSync);
    localFileHandler->setFileTime(toInfo->urlOf(UrlInfoType::kUrl),
                                  fromInfo->timeOf(TimeInfoType::kLastRead).value<QDateTime>(),
                                  fromInfo->timeOf(TimeInfoType::kLastModified).value<QDateTime>());
    QFileDevice::Permissions permissions = fromInfo->permissions();
    QString path = fromInfo->urlOf(UrlInfoType::kUrl).path();
    //权限为0000时，源文件已经被删除，无需修改新建的文件的权限为0000
    if (permissions != 0000 && !FileUtils::isMtpFile(toInfo->urlOf(UrlInfoType::kUrl)))
        localFileHandler->setPermissions(toInfo->urlOf(UrlInfoType::kUrl), permissions);
}

/*!
 * \brief FileOperateBaseWorker::readAheadSourceFile Pre read source file content
 * \param fileInfo File information of source file
 */
void FileOperateBaseWorker::readAheadSourceFile(const DFileInfoPointer &fileInfo)
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
                                                    bool *skip)
{

    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

    do {
        action = AbstractJobHandler::SupportAction::kNoAction;

        qint64 freeBytes = DeviceUtils::deviceBytesFree(targetOrgUrl);
        action = AbstractJobHandler::SupportAction::kNoAction;

        if (FileOperationsUtils::isFilesSizeOutLimit(fromUrl, freeBytes))
            action = doHandleErrorAndWait(fromUrl, toUrl, AbstractJobHandler::JobErrorType::kNotEnoughSpaceError);
    } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());

    checkRetry();

    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        setSkipValue(skip, action);
        return false;
    }

    return true;
}

bool FileOperateBaseWorker::checkFileSize(qint64 size, const QUrl &fromUrl,
                                          const QUrl &toUrl, bool *skip)
{
    if (!targetUrl.isValid()) {
        return true;
    }

    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

    const QString &fs_type = dfmio::DFMUtils::fsTypeFromUrl(targetUrl);

    if (fs_type != "vfat")
        return true;
    if (size < 4l * 1024 * 1024 * 1024)
        return true;

    action = doHandleErrorAndWait(fromUrl, toUrl, AbstractJobHandler::JobErrorType::kFileSizeTooBigError);

    if (action == AbstractJobHandler::SupportAction::kEnforceAction)
        return true;

    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        setSkipValue(skip, action);
        workData->skipWriteSize += size;
        return false;
    }

    return true;
}

bool FileOperateBaseWorker::checkTotalDiskSpaceAvailable(const QUrl &fromUrl, const QUrl &toUrl, bool *skip)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        qint64 freeBytes = DeviceUtils::deviceBytesFree(toUrl);
        fmInfo() << "current free bytes = " << freeBytes << ", write size = " << sourceFilesTotalSize;
        action = AbstractJobHandler::SupportAction::kNoAction;
        if (sourceFilesTotalSize >= freeBytes)
            action = doHandleErrorAndWait(fromUrl, toUrl, AbstractJobHandler::JobErrorType::kNotEnoughSpaceError);
    } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());

    checkRetry();

    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        setSkipValue(skip, action);
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
bool FileOperateBaseWorker::deleteFile(const QUrl &fromUrl, const QUrl &toUrl, bool *workContinue, const bool force)
{
    bool ret = false;

    if (!stateCheck())
        return false;

    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        if (force)
            localFileHandler->setPermissions(fromUrl, QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser);
        ret = localFileHandler->deleteFile(fromUrl);
        if (!ret) {
            fmWarning() << "delete file error, case: " << localFileHandler->errorString();
            action = doHandleErrorAndWait(fromUrl, toUrl, AbstractJobHandler::JobErrorType::kDeleteFileError, false,
                                          localFileHandler->errorString());
        }
    } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());

    if (workContinue)
        *workContinue = action == AbstractJobHandler::SupportAction::kSkipAction || action == AbstractJobHandler::SupportAction::kNoAction;

    return ret;
}

bool FileOperateBaseWorker::deleteDir(const QUrl &fromUrl, const QUrl &toUrl, bool *skip, const bool force)
{
    DFMIO::DEnumerator enumerator(fromUrl);

    bool succ = false;
    while (enumerator.hasNext()) {
        const QUrl &url = enumerator.next();
        bool isDir { DFMIO::DFileInfo(url).attribute(DFMIO::DFileInfo::AttributeID::kStandardIsDir).toBool() };
        if (isDir) {
            if (force)
                localFileHandler->setPermissions(url, QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser);
            succ = deleteDir(url, toUrl, skip, force);
        } else {
            succ = deleteFile(url, toUrl, skip, force);
        }
        if (!succ)
            return false;
    }
    succ = deleteFile(fromUrl, toUrl, skip, force);
    return succ;
}

bool FileOperateBaseWorker::copyFileFromTrash(const QUrl &urlSource, const QUrl &urlTarget, DFile::CopyFlag flag)
{
    auto fileinfo = InfoFactory::create<FileInfo>(urlSource, Global::CreateFileInfoType::kCreateFileInfoSync);
    if (fileinfo->isAttributes(OptInfoType::kIsDir)) {
        if (!DFMIO::DFile(urlTarget).exists()) {
            DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
            if (!fileHandler.mkdir(urlTarget))
                return false;
        }

        QString error;
        const AbstractDirIteratorPointer &iterator = DirIteratorFactory::create<AbstractDirIterator>(urlSource, &error);
        if (!iterator)
            return false;
        while (iterator->hasNext()) {
            const QUrl &url = iterator->next();
            DFileInfoPointer fileinfoNext(new DFileInfo(url));

            bool ok = false;
            DFileInfoPointer toInfo(new DFileInfo(urlTarget));
            if (!toInfo) {
                // pause and emit error msg
                fmCritical() << "sorce file Info or target file info is nullptr : source file info is nullptr = " << (toInfo == nullptr) << ", source file info is nullptr = " << (targetInfo == nullptr);
                const AbstractJobHandler::SupportAction action = doHandleErrorAndWait(url, targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
                if (AbstractJobHandler::SupportAction::kSkipAction != action) {
                    return false;
                } else {
                    continue;
                }
            }
            DFileInfoPointer newTargetInfo = doCheckFile(fileinfoNext, toInfo,
                                                         fileinfoNext->attribute(DFileInfo::AttributeID::kStandardFileName).toString(),
                                                         &ok);
            if (newTargetInfo.isNull())
                continue;

            if (fileinfoNext->attribute(DFileInfo::AttributeID::kStandardIsDir).toBool()) {
                bool succ = copyFileFromTrash(url, newTargetInfo->uri(), flag);
                if (!succ)
                    return false;
            } else {
                DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
                bool trashSucc = fileHandler.copyFile(url, newTargetInfo->uri(), flag);
                if (!trashSucc)
                    return false;
            }
        }
        return true;
    } else {
        DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
        bool trashSucc = fileHandler.copyFile(urlSource, urlTarget, flag);
        return trashSucc;
    }
}
/*!
 * \brief FileOperateBaseWorker::doCopyFile Copy to a new file and delete the source file
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 * \param result Output parameter: whether skip
 * \return Is the copy successful
 */
bool FileOperateBaseWorker::copyAndDeleteFile(const DFileInfoPointer &fromInfo, const DFileInfoPointer &targetPathInfo, const DFileInfoPointer &toInfo, bool *skip)
{
    // 检查磁盘空间
    if (!checkDiskSpaceAvailable(fromInfo->uri(), targetOrgUrl, skip))
        return false;

    bool ok = false;
    if (!toInfo)
        return false;

    if (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool()) {
        ok = createSystemLink(fromInfo, toInfo, workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyFollowSymlink), true, skip);
        if (ok) {
            workData->zeroOrlinkOrDirWriteSize += FileUtils::getMemoryPageSize();
            cutAndDeleteFiles.append(fromInfo);
        }
    } else if (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsDir).toBool()) {
        ok = checkAndCopyDir(fromInfo, toInfo, skip);
        if (ok)
            workData->zeroOrlinkOrDirWriteSize += workData->dirSize;
    } else {
        const QUrl &url = toInfo->uri();
        auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();

        // check file file size bigger than 4 GB
        if (!checkFileSize(fromSize, fromInfo->uri(), url, skip))
            return ok;

        FileUtils::cacheCopyingFileUrl(url);
        initSignalCopyWorker();
        DoCopyFileWorker::NextDo nextDo { DoCopyFileWorker::NextDo::kDoCopyNext };
        if (fromSize > bigFileSize || !supportDfmioCopy || workData->exBlockSyncEveryWrite) {
            do {
                nextDo = copyOtherFileWorker->doCopyFilePractically(fromInfo, toInfo, skip);
            } while( nextDo == DoCopyFileWorker::NextDo::kDoCopyReDoCurrentFile && !isStopped());
            ok = nextDo != DoCopyFileWorker::NextDo::kDoCopyErrorAddCancel;
        } else {
            ok = copyOtherFileWorker->doDfmioFileCopy(fromInfo, toInfo, skip);
        }
        if (ok)
            cutAndDeleteFiles.append(fromInfo);
        FileUtils::removeCopyingFileUrl(url);
    }

    toInfo->initQuerier();
    if (ok && toInfo->exists() && targetInfo == targetPathInfo) {
        completeSourceFiles.append(fromInfo->uri());
        completeTargetFiles.append(toInfo->uri());
    }

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
DFileInfoPointer FileOperateBaseWorker::doCheckFile(const DFileInfoPointer &fromInfo,
                                                    const DFileInfoPointer &toInfo,
                                                    const QString &fileName,
                                                    bool *skip)
{
    fromInfo->initQuerier();
    // 检查源文件是否存在
    if (!fromInfo->exists()) {
        auto fromUrl = fromInfo->uri();
        fmCritical() << " check file from file is  not exists !!!!!!!" << fromUrl;
        fromUrl.setPath(fromUrl.path().replace("\\", "/"));
        AbstractJobHandler::JobErrorType errortype = (fromInfo->attribute(DFileInfo::AttributeID::kStandardFilePath).toString().startsWith("/root/")
                                                      && !toInfo->attribute(DFileInfo::AttributeID::kStandardFilePath).toString().startsWith("/root/"))
                ? AbstractJobHandler::JobErrorType::kPermissionError
                : AbstractJobHandler::JobErrorType::kNonexistenceError;
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromUrl,
                                                                        toInfo == nullptr ? QUrl() : toInfo->uri(), errortype);

        setSkipValue(skip, action);
        return nullptr;
    }
    // 检查目标文件是否存在
    toInfo->initQuerier();
    if (!toInfo->exists()) {
        fmCritical() << " check file to file perant file is  not exists !!!!!!!";
        AbstractJobHandler::JobErrorType errortype = (fromInfo->attribute(DFileInfo::AttributeID::kStandardFilePath).toString().startsWith("/root/")
                                                      && !toInfo->attribute(DFileInfo::AttributeID::kStandardFilePath).toString().startsWith("/root/"))
                ? AbstractJobHandler::JobErrorType::kPermissionError
                : AbstractJobHandler::JobErrorType::kNonexistenceError;
        auto fromUrl = fromInfo->uri();
        fromUrl.setPath(fromUrl.path().replace("\\", "/"));
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromUrl,
                                                                        toInfo->uri(), errortype, true);
        setSkipValue(skip, action);
        return nullptr;
    }
    // 特殊文件判断
    switch (fileType(fromInfo)) {
    case FileInfo::FileType::kCharDevice:
    case FileInfo::FileType::kBlockDevice:
    case FileInfo::FileType::kFIFOFile:
    case FileInfo::FileType::kSocketFile: {
        auto fromUrl = fromInfo->uri();
        fromUrl.setPath(fromUrl.path().replace("\\", "/"));
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromUrl,
                                                                        toInfo->uri(),
                                                                        AbstractJobHandler::JobErrorType::kSpecialFileError);
        setSkipValue(skip, action);
        if (skip && *skip)
            workData->skipWriteSize += fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong() <= 0
                    ? workData->dirSize : fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();

        return nullptr;
    }
    default:
        break;
    }

    // 创建新的目标文件并做检查
    QString fileNewName = fileName;
    // bug 205732, 回收站文件找到源文件名称
    bool isTrashFile = FileUtils::isTrashFile(fromInfo->uri());
    if (isTrashFile) {
        auto trashInfoUrl = trashInfo(fromInfo);
        fileNewName = trashInfoUrl.isValid() ? fileOriginName(trashInfoUrl) : fileName;
    }
    DFileInfoPointer newTargetInfo = doCheckNewFile(fromInfo, toInfo, fileNewName, skip, true);

    return newTargetInfo;
}
/*!
 * \brief FileOperateBaseWorker::creatSystemLink Create system link file
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 * \param result Output parameter: whether skip
 * \return Was the linked file created successfully
 */
bool FileOperateBaseWorker::createSystemLink(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                                             const bool followLink, const bool doCopy, bool *skip)
{
    // 创建链接文件
    DFileInfoPointer newFromInfo = fromInfo;
    if (followLink) {
        QStringList pathList;
        QString pathValue = newFromInfo->uri().path();
        pathValue = pathValue.endsWith(QDir::separator()) && pathValue != QDir::separator()
                ? QString(pathValue).left(pathValue.length() - 1)
                : pathValue;
        pathList.append(pathValue);
        do {
            QUrl newUrl = newFromInfo->uri();
            pathValue = newFromInfo->attribute(DFileInfo::AttributeID::kStandardSymlinkTarget).toString();
            pathValue = pathValue.endsWith(QDir::separator()) && pathValue != QDir::separator()
                    ? QString(pathValue).left(pathValue.length() - 1)
                    : pathValue;
            newUrl.setPath(pathValue);
            DFileInfoPointer symlinkTarget(new DFileInfo(newUrl));

            symlinkTarget->initQuerier();
            if (!symlinkTarget->exists() || pathList.contains(pathValue)) {
                break;
            }
            pathList.append(pathValue);

            newFromInfo = symlinkTarget;
        } while (newFromInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool());

        newFromInfo->initQuerier();
        if (newFromInfo->exists() && doCopy) {
            // copy file here
            if (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsFile).toBool()) {
                return checkAndCopyFile(fromInfo, toInfo, skip);
            } else {
                return checkAndCopyDir(fromInfo, toInfo, skip);
            }
        }
    }

    AbstractJobHandler::SupportAction actionForlink { AbstractJobHandler::SupportAction::kNoAction };

    do {
        actionForlink = AbstractJobHandler::SupportAction::kNoAction;
        if (localFileHandler->createSystemLink(newFromInfo->uri(), toInfo->uri())) {
            return true;
        }
        actionForlink = doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(),
                                             AbstractJobHandler::JobErrorType::kSymlinkError, false,
                                             localFileHandler->errorString());
    } while (actionForlink == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());
    checkRetry();
    setSkipValue(skip, actionForlink);
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
DFileInfoPointer FileOperateBaseWorker::doCheckNewFile(const DFileInfoPointer &fromInfo,
                                                       const DFileInfoPointer &toInfo,
                                                       QString &fileNewName, bool *skip,
                                                       bool isCountSize)
{
    auto newTargetUrl = createNewTargetUrl(toInfo, fileNewName);
    DFileInfoPointer newTargetInfo { new DFileInfo(newTargetUrl)};
    newTargetInfo->initQuerier();
    if (!newTargetInfo->exists())
        return newTargetInfo;

    if (!workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyToSelf) && FileOperationsUtils::isAncestorUrl(fromInfo->uri(), newTargetUrl)) {
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->uri(),
                                                                        toInfo->uri(),
                                                                        AbstractJobHandler::JobErrorType::kTargetIsSelfError);
        if (AbstractJobHandler::SupportAction::kSkipAction == action) {
            setSkipValue(skip, action);
            if (skip && *skip)
                workData->skipWriteSize += isCountSize
                        && (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool()
                            || fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong() <= 0)
                        ? workData->dirSize : fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
            return nullptr;
        }

        if (action != AbstractJobHandler::SupportAction::kEnforceAction) {
            setSkipValue(skip, action);
            if (skip && *skip)
                workData->skipWriteSize += isCountSize
                        && (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool()
                            || fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong() <= 0)
                        ? workData->dirSize : fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
            return nullptr;
        }
    };

    bool newTargetIsFile = newTargetInfo->attribute(DFileInfo::AttributeID::kStandardIsFile).toBool()
            || newTargetInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool();
    AbstractJobHandler::JobErrorType errortype = newTargetIsFile ? AbstractJobHandler::JobErrorType::kFileExistsError
                                                                 : AbstractJobHandler::JobErrorType::kDirectoryExistsError;
    AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->uri(),
                                                                    newTargetInfo->uri(), errortype, true);
    switch (action) {
    case AbstractJobHandler::SupportAction::kReplaceAction: {
        const QVariant &var = doActionReplace(fromInfo, newTargetInfo, isCountSize);
        if (var.isValid())
            return var.toBool() ? newTargetInfo : nullptr;
        break;
    }
    case AbstractJobHandler::SupportAction::kMergeAction: {
        const QVariant &var = doActionMerge(fromInfo, newTargetInfo, isCountSize);
        if (var.isValid())
            return var.toBool() ? newTargetInfo : nullptr;
        break;
    }
    case AbstractJobHandler::SupportAction::kSkipAction: {
        workData->skipWriteSize += (isCountSize &&
                                    (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool()
                                     || fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong() <= 0))
                ? workData->dirSize : fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
        setSkipValue(skip, action);
        return nullptr;
    }
    case AbstractJobHandler::SupportAction::kCoexistAction: {
        const auto ne = InfoFactory::create<FileInfo>(newTargetInfo->uri(), Global::CreateFileInfoType::kCreateFileInfoSync);
        const auto to = InfoFactory::create<FileInfo>(toInfo->uri(), Global::CreateFileInfoType::kCreateFileInfoSync);
        fileNewName = FileUtils::nonExistFileName(ne, to);
        if (fileNewName.isEmpty()) {
            return nullptr;
        }
        auto newTargetUrl = createNewTargetUrl(toInfo, fileNewName);
        newTargetInfo.reset(new DFileInfo(newTargetUrl));
        newTargetInfo->initQuerier();
        return !newTargetInfo->exists() ? newTargetInfo : nullptr;
    }
    case AbstractJobHandler::SupportAction::kCancelAction: {
        stopWork.store(true);
        return nullptr;
    }
    default:
        return nullptr;
    }
    return newTargetInfo;
}

bool FileOperateBaseWorker::checkAndCopyFile(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip)
{
    auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
    // check file file size bigger than 4 GB
    if (!checkFileSize(fromSize, fromInfo->uri(),
                       toInfo->uri(), skip))
        return false;

    if (jobType == AbstractJobHandler::JobType::kCutType)
        return doCopyOtherFile(fromInfo, toInfo, skip);

    if (isSourceFileLocal && isTargetFileLocal && !workData->signalThread) {
        while (bigFileCopy && !isStopped()) {
            QThread::msleep(10);
        }
        if (fromSize > bigFileSize) {
            bigFileCopy = true;
            auto result = doCopyLocalBigFile(fromInfo, toInfo, skip);
            bigFileCopy = false;
            return result;
        }
        return doCopyLocalFile(fromInfo, toInfo);
    }

    // copy other file or cut file
    return doCopyOtherFile(fromInfo, toInfo, skip);
}

bool FileOperateBaseWorker::checkAndCopyDir(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo, bool *skip)
{
    emitCurrentTaskNotify(fromInfo->uri(), toInfo->uri());
    // 检查文件的一些合法性，源文件是否存在，创建新的目标目录名称，检查新创建目标目录名称是否存在
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    QFileDevice::Permissions permissions = QFileDevice::Permissions(uint(fromInfo->permissions()));
    toInfo->initQuerier();
    if (!toInfo->exists()) {
        do {
            action = AbstractJobHandler::SupportAction::kNoAction;
            if (localFileHandler->mkdir(toInfo->uri()))
                break;
            // 特殊处理
            auto errstr = localFileHandler->errorString();
            auto fileUrl= toInfo->uri();
            if (localFileHandler->errorCode() == DFMIOErrorCode::DFM_IO_ERROR_FAILED
                    && fileUrl.path().toLocal8Bit().size() > 255
                    && FileUtils::isMtpFile(fileUrl))
                errstr = tr("The file name or the path is too long!");

            action = doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(),
                                          AbstractJobHandler::JobErrorType::kMkdirError, true,
                                          errstr);
        } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

        checkRetry();
        if (AbstractJobHandler::SupportAction::kNoAction != action) {
            // skip write size += all file size in sources dir
            // todo liyigang
            // if from dir is not null,skip write size count error
            setSkipValue(skip, action);
            if (skip && *skip)
                workData->skipWriteSize += workData->dirSize;

            return false;
        }

        if (workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyToSelf)) {
            QSharedPointer<DirSetPermissonInfo> dirinfo(new DirSetPermissonInfo);
            dirinfo->target = toInfo->uri();
            dirinfo->permission = permissions;
            dirPermissonList.appendByLock(dirinfo);
            return true;
        }
    }

    // 遍历源文件，执行一个一个的拷贝
    QString error;
    const AbstractDirIteratorPointer &iterator = DirIteratorFactory::create<AbstractDirIterator>(fromInfo->uri(), &error);
    if (!iterator) {
        fmCritical() << "create dir's iterator failed, case : " << error;
        doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(), AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }

    bool self = true;
    while (iterator->hasNext()) {
        if (!stateCheck()) {
            return false;
        }

        const QUrl &url = iterator->next();
        DFileInfoPointer info(new DFileInfo(url));
        bool ok = doCopyFile(info, toInfo, skip);
        if (!ok && (!skip || !*skip)) {
            return false;
        }

        if (jobType == AbstractJobHandler::JobType::kCutType) {
            if (!ok) {
                self = false;
                continue;
            }

            if (info->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool()
                    || info->attribute(DFileInfo::AttributeID::kStandardIsFile).toBool()) {
                cutAndDeleteFiles.append(info);
            } else if (self && !cutAndDeleteFiles.contains(info)) {
                self = false;
            }
        }
    }

    if (jobType == AbstractJobHandler::JobType::kCutType && self)
        cutAndDeleteFiles.append(fromInfo);

    if (isTargetFileLocal && isSourceFileLocal) {
        DirPermsissonPointer dirinfo(new DirSetPermissonInfo);
        dirinfo->target = toInfo->uri();
        dirinfo->permission = permissions;
        dirPermissonList.appendByLock(dirinfo);
    } else {
        if (permissions && !FileUtils::isMtpFile(toInfo->uri()))
            localFileHandler->setPermissions(toInfo->uri(), permissions);
    }

    return true;
}

void FileOperateBaseWorker::waitThreadPoolOver()
{
    // wait all thread start
    if (!isStopped() && threadPool) {
        QThread::msleep(10);
    }
    // wait thread pool copy local file or copy big file over
    while (threadPool && threadPool->activeThreadCount() > 0) {
        QThread::msleep(10);
    }
}

void FileOperateBaseWorker::initCopyWay()
{
    // local file useing least 8 thread
    if (isSourceFileLocal && isTargetFileLocal) {
        countWriteType = CountWriteSizeType::kCustomizeType;
        workData->signalThread = (sourceFilesCount > 1 || sourceFilesTotalSize > FileOperationsUtils::bigFileSize()) && FileUtils::getCpuProcessCount() > 4
                ? false
                : true;
        if (!workData->signalThread)
            threadCount = FileUtils::getCpuProcessCount() >= 8 ? FileUtils::getCpuProcessCount() : 8;
    }

    if (DeviceUtils::isSamba(targetUrl)
        || DeviceUtils::isFtp(targetUrl)
        || workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCountProgressCustomize))
        countWriteType = CountWriteSizeType::kCustomizeType;

    if (!workData->signalThread) {
        initThreadCopy();
    }

    copyTid = (countWriteType == CountWriteSizeType::kTidType) ? syscall(SYS_gettid) : -1;
}

QUrl FileOperateBaseWorker::trashInfo(const DFileInfoPointer &fromInfo)
{
    auto parentPath = parentUrl(fromInfo->uri()).path();
    if (!parentPath.endsWith("files"))
        return QUrl();
    auto fileName = fromInfo->attribute(DFileInfo::AttributeID::kStandardFileName).toString();
    auto trashInfoUrl = QUrl::fromLocalFile(parentPath.replace("files", "info/") + fileName + ".trashinfo");
    return trashInfoUrl;
}

QString FileOperateBaseWorker::fileOriginName(const QUrl &trashInfoUrl)
{
    if (!trashInfoUrl.isValid())
        return QString();
    DFile file(trashInfoUrl);
    if (!file.open(dfmio::DFile::OpenFlag::kReadOnly)) {
        fmWarning() << "open trash file info err : " << file.lastError().errorMsg() << " ,trashInfoUrl = " << trashInfoUrl;
        return QString();
    }
    auto data = file.readAll().simplified().split(' ');
    // trash info file readAll() = "[Trash Info] Path=%E6%96%B0%E5%BB%BAWord%E6%96%87%E6%A1%A3.doc DeletionDate=2023-05-05T11:19:06";
    // has three char " ", so data has 4 item, the 3th is the "Path=%E6%96%B0%E5%BB%BAWord%E6%96%87%E6%A1%A3.doc"
    if (data.size() <= 3) {
        fmWarning() << "reade trash file info err,trashInfoUrl = " << trashInfoUrl;
        return QString();
    }
    QString filePath(data.at(2));
    filePath.replace(0, 5, "");
    const QUrl &url = QUrl::fromLocalFile(QByteArray::fromPercentEncoding(filePath.toLocal8Bit()));
    return url.fileName();
}

void FileOperateBaseWorker::removeTrashInfo(const QUrl &trashInfoUrl)
{
    if (!localFileHandler || !trashInfoUrl.isValid())
        return;
    fmDebug() << "delete trash file info. trashInfoUrl = " << trashInfoUrl;
    localFileHandler->deleteFile(trashInfoUrl);
}

void FileOperateBaseWorker::setSkipValue(bool *skip, AbstractJobHandler::SupportAction action)
{
    if (skip)
        *skip = action == AbstractJobHandler::SupportAction::kSkipAction;
}

void FileOperateBaseWorker::initThreadCopy()
{
    for (int i = 0; i < threadCount; i++) {
        QSharedPointer<DoCopyFileWorker> copy(new DoCopyFileWorker(workData));
        // todo init new
        connect(copy.data(), &DoCopyFileWorker::errorNotify, this, &FileOperateBaseWorker::emitErrorNotify, Qt::DirectConnection);
        connect(copy.data(), &DoCopyFileWorker::currentTask, this, &FileOperateBaseWorker::emitCurrentTaskNotify, Qt::DirectConnection);
        connect(copy.data(), &DoCopyFileWorker::retryErrSuccess, this, &FileOperateBaseWorker::retryErrSuccess, Qt::DirectConnection);
        connect(copy.data(), &DoCopyFileWorker::skipCopyLocalBigFile, this, &FileOperateBaseWorker::skipMemcpyBigFile, Qt::DirectConnection);
        threadCopyWorker.append(copy);
    }

    threadPool.reset(new QThreadPool);
    threadPool->setMaxThreadCount(threadCount);
}

void FileOperateBaseWorker::initSignalCopyWorker()
{
    if (!copyOtherFileWorker) {
        copyOtherFileWorker.reset(new DoCopyFileWorker(workData));
        connect(copyOtherFileWorker.data(), &DoCopyFileWorker::errorNotify, this, &FileOperateBaseWorker::emitErrorNotify);
        connect(copyOtherFileWorker.data(), &DoCopyFileWorker::currentTask, this, &FileOperateBaseWorker::emitCurrentTaskNotify);
    }
}

QUrl FileOperateBaseWorker::createNewTargetUrl(const DFileInfoPointer &toInfo, const QString &fileName)
{
    QString fileNewName = formatFileName(fileName);
    // 创建文件的名称
    QUrl newTargetUrl = toInfo->uri();
    const QString &newTargetPath = newTargetUrl.path();
    const QString &newPath = DFMIO::DFMUtils::buildFilePath(newTargetPath.toStdString().c_str(), fileNewName.toStdString().c_str(), nullptr);
    newTargetUrl.setPath(newPath);
    return newTargetUrl;
}

bool FileOperateBaseWorker::doCopyLocalFile(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo)
{
    if (!stateCheck())
        return false;

    QtConcurrent::run(threadPool.data(), threadCopyWorker[threadCopyFileCount % threadCount].data(),
                      static_cast<void (DoCopyFileWorker::*)(const DFileInfoPointer, const DFileInfoPointer)>(&DoCopyFileWorker::doFileCopy),
                      fromInfo, toInfo);

    threadCopyFileCount++;
    return true;
}

bool FileOperateBaseWorker::doCopyLocalBigFile(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip)
{
    waitThreadPoolOver();
    // open file
    auto fromFd = doOpenFile(fromInfo, toInfo, false, O_RDONLY, skip);
    if (fromFd < 0)
        return false;
    auto toFd = doOpenFile(fromInfo, toInfo, true, O_CREAT | O_RDWR, skip);
    if (toFd < 0) {
        close(fromFd);
        return false;
    }
    // resize target file
    if (!doCopyLocalBigFileResize(fromInfo, toInfo, toFd, skip)) {
        close(fromFd);
        close(toFd);
        return false;
    }
    // mmap file
    auto fromPoint = doCopyLocalBigFileMap(fromInfo, toInfo, fromFd, PROT_READ, skip);
    if (!fromPoint) {
        close(fromFd);
        close(toFd);
        return false;
    }
    auto toPoint = doCopyLocalBigFileMap(fromInfo, toInfo, toFd, PROT_WRITE, skip);
    if (!toPoint) {
        munmap(fromPoint, static_cast<size_t>(fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong()));
        close(fromFd);
        close(toFd);
        return false;
    }
    // memcpy file in other thread
    memcpyLocalBigFile(fromInfo, toInfo, fromPoint, toPoint);
    // wait copy
    waitThreadPoolOver();
    // clear
    doCopyLocalBigFileClear(static_cast<size_t>(fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong()), fromFd, toFd, fromPoint, toPoint);
    // set permissions
    setTargetPermissions(fromInfo->uri(), toInfo->uri());
    return true;
}

bool FileOperateBaseWorker::doCopyLocalBigFileResize(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, int toFd, bool *skip)
{
    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    __off_t length = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        if (-1 == ftruncate(toFd, length)) {
            auto lastError = strerror(errno);
            fmWarning() << "file resize error, url from: " << fromInfo->uri()
                        << " url to: " << toInfo->uri() << " open flag: " << O_RDONLY
                        << " error code: " << errno << " error msg: " << lastError;

            action = doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(),
                                          AbstractJobHandler::JobErrorType::kResizeError, true, lastError);
        }
    } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());

    checkRetry();

    if (!actionOperating(action, length <= 0 ? FileUtils::getMemoryPageSize() : length, skip))
        return false;

    return true;
}

char *FileOperateBaseWorker::doCopyLocalBigFileMap(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, int fd, const int per, bool *skip)
{
    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    void *point = nullptr;
    auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        point = mmap(nullptr, static_cast<size_t>(fromSize),
                     per, MAP_SHARED, fd, 0);
        if (!point || point == MAP_FAILED) {
            auto lastError = strerror(errno);
            fmWarning() << "file mmap error, url from: " << fromInfo->uri()
                        << " url to: " << fromInfo->uri()
                        << " error code: " << errno << " error msg: " << lastError;

            action = doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(),
                                          AbstractJobHandler::JobErrorType::kOpenError, fd == PROT_WRITE, lastError);
        }
    } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());

    checkRetry();

    if (!actionOperating(action, fromSize <= 0 ? FileUtils::getMemoryPageSize() : fromSize, skip))
        return nullptr;

    return static_cast<char *>(point);
}

void FileOperateBaseWorker::memcpyLocalBigFile(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, char *fromPoint, char *toPoint)
{
    auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
    auto offset = fromSize / threadCount;
    char *fromPointStart = fromPoint;
    char *toPointStart = toPoint;
    for (int i = 0; i < threadCount; i++) {
        offset = (i == (threadCount - 1) ? fromSize - (threadCount - 1) * offset : offset);

        char *tempfFromPointStart = fromPointStart;
        char *tempfToPointStart = toPointStart;
        size_t tempOffet = static_cast<size_t>(offset);
        QtConcurrent::run(threadPool.data(), threadCopyWorker[i].data(),
                          static_cast<void (DoCopyFileWorker::*)(const DFileInfoPointer fromInfo,
                                                                 const DFileInfoPointer toInfo,
                                                                 char *dest, char *source, size_t size)>(&DoCopyFileWorker::doMemcpyLocalBigFile),
                          fromInfo, toInfo, tempfToPointStart, tempfFromPointStart, tempOffet);

        fromPointStart += offset;
        toPointStart += offset;
    }
}

void FileOperateBaseWorker::doCopyLocalBigFileClear(const size_t size,
                                                    const int fromFd, const int toFd, char *fromPoint, char *toPoint)
{
    munmap(fromPoint, size);
    munmap(toPoint, size);
    close(fromFd);
    close(toFd);
}

int FileOperateBaseWorker::doOpenFile(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo,
                                      const bool isTo, const int openFlag, bool *skip)
{
    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    emitCurrentTaskNotify(fromInfo->uri(), toInfo->uri());
    int fd = -1;
    do {
        QUrl url = isTo ? toInfo->uri() : fromInfo->uri();
        std::string path = url.path().toStdString();
        fd = open(path.c_str(), openFlag, 0666);
        action = AbstractJobHandler::SupportAction::kNoAction;
        if (fd < 0) {
            auto lastError = strerror(errno);
            fmWarning() << "file open error, url from: " << fromInfo->uri()
                        << " url to: " << fromInfo->uri() << " open flag: " << openFlag
                        << " error code: " << errno << " error msg: " << lastError;

            action = doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(),
                                          AbstractJobHandler::JobErrorType::kOpenError, isTo, lastError);
        }
    } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());

    checkRetry();

    auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
    if (!actionOperating(action, fromSize <= 0 ? FileUtils::getMemoryPageSize() : fromSize, skip)) {
        if (fd >= 0)
            close(fd);
        return -1;
    }

    return fd;
}

bool FileOperateBaseWorker::doCopyOtherFile(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip)
{
    initSignalCopyWorker();
    const QString &targetUrl = toInfo->uri().toString();

    FileUtils::cacheCopyingFileUrl(targetUrl);
    bool ok { false };
    const auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
    DoCopyFileWorker::NextDo nextDo { DoCopyFileWorker::NextDo::kDoCopyNext };
    if (fromSize > bigFileSize || !supportDfmioCopy || workData->exBlockSyncEveryWrite) {
        do {
            nextDo = copyOtherFileWorker->doCopyFilePractically(fromInfo, toInfo, skip);
        } while( nextDo == DoCopyFileWorker::NextDo::kDoCopyReDoCurrentFile && !isStopped());
        ok = nextDo != DoCopyFileWorker::NextDo::kDoCopyErrorAddCancel;
    } else {
        ok = copyOtherFileWorker->doDfmioFileCopy(fromInfo, toInfo, skip);
    }
    if (ok)
        syncFiles.append(targetUrl);
    FileUtils::removeCopyingFileUrl(targetUrl);

    return ok;
}

bool FileOperateBaseWorker::actionOperating(const AbstractJobHandler::SupportAction action, const qint64 size, bool *skip)
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

void FileOperateBaseWorker::emitErrorNotify(const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error,
                                            const bool isTo, const quint64 id, const QString &errorMsg,
                                            const bool allUsErrorMsg)
{
    AbstractWorker::emitErrorNotify(from, to, error, isTo, id, errorMsg, allUsErrorMsg);
}

void FileOperateBaseWorker::emitCurrentTaskNotify(const QUrl &from, const QUrl &to)
{
    AbstractWorker::emitCurrentTaskNotify(from, to);
}

void FileOperateBaseWorker::skipMemcpyBigFile(const QUrl url)
{
    for (const auto &worker : threadCopyWorker) {
        worker->skipMemcpyBigFile(url);
    }
}

QVariant FileOperateBaseWorker::checkLinkAndSameUrl(const DFileInfoPointer &fromInfo,
                                                    const DFileInfoPointer &newTargetInfo,
                                                    const bool isCountSize)
{
    if (newTargetInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool()) {
        LocalFileHandler handler;
        if (!handler.deleteFile(newTargetInfo->uri()))
            return false;
    }

    const QUrl &newTargetUrl = newTargetInfo->uri();
    if (newTargetUrl == fromInfo->uri()) {
        workData->skipWriteSize += (isCountSize &&
                                    (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool()
                                     || fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong() <= 0))
                ? workData->dirSize : fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
        return true;
    }

    return QVariant();
}

QVariant FileOperateBaseWorker::doActionReplace(const DFileInfoPointer &fromInfo,
                                                const DFileInfoPointer &newTargetInfo,
                                                const bool isCountSize)
{
    const QVariant &var = checkLinkAndSameUrl(fromInfo, newTargetInfo, isCountSize);
    if (var.isValid())
        return var;

    const bool fromIsFile = fromInfo->attribute(DFileInfo::AttributeID::kStandardIsFile).toBool()
            || fromInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool();
    const bool newTargetIsFile = newTargetInfo->attribute(DFileInfo::AttributeID::kStandardIsFile).toBool()
            || newTargetInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool();

    if (fromIsFile == newTargetIsFile) {
        return QVariant();
    } else {
        return false;
    }
}

QVariant FileOperateBaseWorker::doActionMerge(const DFileInfoPointer &fromInfo, const DFileInfoPointer &newTargetInfo, const bool isCountSize)
{
    const bool fromIsFile = fromInfo->attribute(DFileInfo::AttributeID::kStandardIsFile).toBool()
            || fromInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool();
    const bool newTargetIsFile = newTargetInfo->attribute(DFileInfo::AttributeID::kStandardIsFile).toBool()
            || newTargetInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool();

    if (!fromIsFile && !newTargetIsFile) {
        // target is dir, do merged
        return QVariant();
    } else if (fromIsFile && newTargetIsFile) {
        return checkLinkAndSameUrl(fromInfo, newTargetInfo, isCountSize);
    } else {
        return false;
    }
}

bool FileOperateBaseWorker::doCopyFile(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo, bool *skip)
{
    bool result = false;
    DFileInfoPointer newTargetInfo = doCheckFile(fromInfo, toInfo,
                                                 fromInfo->attribute(DFileInfo::AttributeID::kStandardFileName).toString(), skip);
    if (newTargetInfo.isNull())
        return result;

    if (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool()) {
        result = createSystemLink(fromInfo, newTargetInfo, workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyFollowSymlink), true, skip);
        if (result)
            workData->zeroOrlinkOrDirWriteSize +=
                    (newTargetInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong() > 0 ?
                         newTargetInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong()
                       : FileUtils::getMemoryPageSize());
    } else if (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsDir).toBool()) {
        result = checkAndCopyDir(fromInfo, newTargetInfo, skip);
        if (result || skip)
            workData->zeroOrlinkOrDirWriteSize += workData->dirSize <= 0 ? FileUtils::getMemoryPageSize() : workData->dirSize;
    } else {
        result = checkAndCopyFile(fromInfo, newTargetInfo, skip);
    }

    if (targetInfo == toInfo) {
        completeSourceFiles.append(fromInfo->uri());
        precompleteTargetFileInfo.append(newTargetInfo);
    }

    return result;
}

bool FileOperateBaseWorker::canWriteFile(const QUrl &url) const
{
    // root user return true direct
    if (getuid() == 0)
        return true;

    auto info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);

    if (info.isNull())
        return false;

    auto parentInfo = InfoFactory::create<FileInfo>(info->urlOf(UrlInfoType::kParentUrl), Global::CreateFileInfoType::kCreateFileInfoSync);
    if (parentInfo.isNull())
        return false;

    bool isFolderWritable = parentInfo->isAttributes(OptInfoType::kIsWritable);
    if (!isFolderWritable)
        return false;

#ifdef Q_OS_LINUX
    struct stat statBuffer;
    if (::lstat(parentInfo->urlOf(UrlInfoType::kParentUrl).path().toLocal8Bit().data(), &statBuffer) == 0) {
        // 如果父目录拥有t权限，则判断当前用户是不是文件的owner，不是则无法操作文件
        const auto &fileOwnerId = info->extendAttributes(ExtInfoType::kOwnerId);
        const auto &uid = getuid();
        const bool hasTRight = (statBuffer.st_mode & S_ISVTX) == S_ISVTX;
        if (hasTRight && fileOwnerId != uid) {
            return false;
        }
    }
#endif

    return true;
}

void FileOperateBaseWorker::setAllDirPermisson()
{
    for (auto info : dirPermissonList.list()) {
        if (info->permission && supportSetPermission)
            localFileHandler->setPermissions(info->target, info->permission);
    }
}

qint64 FileOperateBaseWorker::getWriteDataSize()
{
    qint64 writeSize = 0;

    if (CountWriteSizeType::kTidType == countWriteType) {
        writeSize = getTidWriteSize();

        if (writeSize > workData->currentWriteSize && workData->currentWriteSize > 0) {
            writeSize = workData->currentWriteSize;
        }
        if (writeSize <= 0)
            writeSize = workData->currentWriteSize;
    } else if (CountWriteSizeType::kCustomizeType == countWriteType) {
        writeSize = workData->currentWriteSize;
    } else if (CountWriteSizeType::kWriteBlockType == countWriteType) {
        qint64 currentSectorsWritten = getSectorsWritten() + workData->blockRenameWriteSize;
        if (currentSectorsWritten > targetDeviceStartSectorsWritten)
            writeSize = (currentSectorsWritten - targetDeviceStartSectorsWritten) * targetLogSecionSize;
    }

    writeSize += (workData->skipWriteSize + workData->zeroOrlinkOrDirWriteSize);

    return writeSize;
}

qint64 FileOperateBaseWorker::getTidWriteSize()
{
    QFile file(QStringLiteral("/proc/self/task/%1/io").arg(copyTid));

    if (!file.open(QIODevice::ReadOnly)) {
        fmWarning() << "Failed on open the" << file.fileName() << ", will be not update the job speed and progress";

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
                fmWarning() << "Failed to convert to qint64, line string=" << line;

                return 0;
            }
            return size;
        }
    }

    fmWarning() << "Failed to find \"" << line_head << "\" from the" << file.fileName();

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
    auto rootPath = DFMUtils::mountPathFromUrl(targetOrgUrl);
    auto device = DFMUtils::deviceNameFromUrl(targetOrgUrl);
    if (device.startsWith("/dev/")) {
        isTargetFileLocal = FileOperationsUtils::isFileOnDisk(targetOrgUrl);
        isTargetFileExBlock = false;
        fmDebug("Target block device: \"%s\", Root Path: \"%s\"", device.toStdString().data(), qPrintable(rootPath));
        const bool isFileSystemTypeExt = DFMUtils::fsTypeFromUrl(targetOrgUrl).startsWith("ext");
        if (!isFileSystemTypeExt) {
            blocakTargetRootPath = rootPath;
            QProcess process;
            process.start("lsblk", { "-niro", "MAJ:MIN,HOTPLUG,LOG-SEC", device }, QIODevice::ReadOnly);

            if (process.waitForFinished(3000)) {
                if (process.exitCode() == 0) {
                    const QByteArray &data = process.readAllStandardOutput();
                    const QByteArrayList &list = data.split(' ');

                    fmDebug("lsblk result data: \"%s\"", data.constData());

                    if (list.size() == 3) {
                        targetSysDevPath = "/sys/dev/block/" + list.first();
                        targetIsRemovable = list.at(1) == "1";

                        bool ok = false;
                        targetLogSecionSize = static_cast<qint16>(list.at(2).toInt(&ok));

                        if (!ok) {
                            targetLogSecionSize = 512;

                            fmWarning() << "get target log secion size failed!";
                        }

                        if (targetIsRemovable) {
                            workData->exBlockSyncEveryWrite = FileOperationsUtils::blockSync();
                            countWriteType = workData->exBlockSyncEveryWrite ? CountWriteSizeType::kCustomizeType
                                                                             : CountWriteSizeType::kWriteBlockType;
                            targetDeviceStartSectorsWritten = workData->exBlockSyncEveryWrite ? 0 : getSectorsWritten();

                            workData->isBlockDevice = true;
                        }

                        fmDebug("Block device path: \"%s\", Sys dev path: \"%s\", Is removable: %d, Log-Sec: %d",
                                qPrintable(device), qPrintable(targetSysDevPath), bool(targetIsRemovable), targetLogSecionSize);
                    } else {
                        fmWarning("Failed on parse the lsblk result data, data: \"%s\"", data.constData());
                    }
                } else {
                    fmWarning("Failed on exec lsblk command, exit code: %d, error message: \"%s\"", process.exitCode(), process.readAllStandardError().constData());
                }
            }
        }
        fmDebug("targetIsRemovable = %d", bool(targetIsRemovable));
    }
}

void FileOperateBaseWorker::syncFilesToDevice()
{

    if (isTargetFileLocal)
        return;

    fmInfo() << "start sync all file to extend block device!!!!! target : " << targetUrl;
    for (const auto &url : syncFiles) {
        std::string stdStr = url.path().toUtf8().toStdString();
        int tofd = open(stdStr.data(), O_RDONLY);
        if (-1 != tofd) {
            syncfs(tofd);
            close(tofd);
        }
    }
    fmInfo() << "end sync all file to extend block device!!!!! target : " << targetUrl;
    // 这里本来是拷贝到了手动分区的盘，不需要后面去等待同步计算进度结果
}
