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
    qint64 speed = writSize * 1000 / (time.elapsed() == 0 ? 1 : time.elapsed());
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(jobType));
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobStateKey, QVariant::fromValue(currentState));
    info->insert(AbstractJobHandler::NotifyInfoKey::kSpeedKey, QVariant::fromValue(speed));
    info->insert(AbstractJobHandler::NotifyInfoKey::kRemindTimeKey, QVariant::fromValue(speed == 0 ? 0 : (sourceFilesTotalSize - writSize) / speed));

    emit stateChangedNotify(info);
    emit speedUpdatedNotify(info);
}

/*!
 * \brief FileOperateBaseWorker::setTargetPermissions Set permissions on the target file
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 */
void FileOperateBaseWorker::setTargetPermissions(const FileInfoPointer &fromInfo, const FileInfoPointer &toInfo)
{
    // 修改文件修改时间
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
void FileOperateBaseWorker::readAheadSourceFile(const FileInfoPointer &fileInfo)
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

        qint64 freeBytes = DFMIO::DFMUtils::deviceBytesFree(targetOrgUrl);
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
            auto fileinfoNext = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);

            FileInfoPointer newTargetInfo(nullptr);
            bool ok = false;
            FileInfoPointer toInfo = InfoFactory::create<FileInfo>(urlTarget, Global::CreateFileInfoType::kCreateFileInfoSync);
            if (!toInfo) {
                // pause and emit error msg
                qCritical() << "sorce file Info or target file info is nullptr : source file info is nullptr = " << (toInfo == nullptr) << ", source file info is nullptr = " << (targetInfo == nullptr);
                const AbstractJobHandler::SupportAction action = doHandleErrorAndWait(url, targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
                if (AbstractJobHandler::SupportAction::kSkipAction != action) {
                    return false;
                } else {
                    continue;
                }
            }
            if (!doCheckFile(fileinfoNext, toInfo, fileinfoNext->nameOf(NameInfoType::kFileCopyName), newTargetInfo, &ok))
                continue;

            if (fileinfoNext->isAttributes(OptInfoType::kIsDir)) {
                bool succ = copyFileFromTrash(url, newTargetInfo->urlOf(UrlInfoType::kUrl), flag);
                if (!succ)
                    return false;
            } else {
                DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
                bool trashSucc = fileHandler.copyFile(url, newTargetInfo->urlOf(UrlInfoType::kUrl), flag);
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
bool FileOperateBaseWorker::copyAndDeleteFile(const FileInfoPointer &fromInfo, const FileInfoPointer &targetPathInfo, const FileInfoPointer &toInfo, bool *skip)
{
    // 检查磁盘空间
    if (!checkDiskSpaceAvailable(fromInfo->urlOf(UrlInfoType::kUrl), targetOrgUrl, skip))
        return false;

    bool ok = false;
    if (!toInfo)
        return false;

    if (fromInfo->isAttributes(OptInfoType::kIsSymLink)) {
        ok = createSystemLink(fromInfo, toInfo, workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyFollowSymlink), true, skip);
        if (ok) {
            workData->zeroOrlinkOrDirWriteSize += FileUtils::getMemoryPageSize();
            ok = deleteFile(fromInfo->urlOf(UrlInfoType::kUrl), targetPathInfo->urlOf(UrlInfoType::kUrl), skip);
        }
    } else if (fromInfo->isAttributes(OptInfoType::kIsDir)) {
        ok = checkAndCopyDir(fromInfo, toInfo, skip);
        if (ok) {
            workData->zeroOrlinkOrDirWriteSize += workData->dirSize;
            ok = deleteDir(fromInfo->urlOf(UrlInfoType::kUrl), targetPathInfo->urlOf(UrlInfoType::kUrl), skip);
        }
    } else {
        const QUrl &url = toInfo->urlOf(UrlInfoType::kUrl);

        // check file file size bigger than 4 GB
        if (!checkFileSize(fromInfo->size(), fromInfo->urlOf(UrlInfoType::kUrl), url, skip))
            return ok;

        FileUtils::cacheCopyingFileUrl(url);
        initSignalCopyWorker();
        if (fromInfo->size() > bigFileSize || !supportDfmioCopy || workData->exBlockSyncEveryWrite) {
            ok = copyOtherFileWorker->doCopyFilePractically(fromInfo, toInfo, skip);
        } else {
            ok = copyOtherFileWorker->doDfmioFileCopy(fromInfo, toInfo, skip);
        }
        if (ok)
            ok = deleteFile(fromInfo->urlOf(UrlInfoType::kUrl), targetPathInfo->urlOf(UrlInfoType::kUrl), skip);
        FileUtils::removeCopyingFileUrl(url);
    }

    if (ok && toInfo->exists() && targetInfo == targetPathInfo) {
        completeSourceFiles.append(fromInfo->urlOf(UrlInfoType::kUrl));
        completeTargetFiles.append(toInfo->urlOf(UrlInfoType::kUrl));
    }

    targetPathInfo->refresh();

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
bool FileOperateBaseWorker::doCheckFile(const FileInfoPointer &fromInfo, const FileInfoPointer &toInfo, const QString &fileName,
                                        FileInfoPointer &newTargetInfo, bool *skip)
{
    // 检查源文件的文件信息
    if (!fromInfo) {
        qCritical() << " check file from file info is  nullpter !!!!!!!";
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(QUrl(), toInfo == nullptr ? QUrl() : toInfo->urlOf(UrlInfoType::kUrl),
                                                                        AbstractJobHandler::JobErrorType::kProrogramError);
        setSkipValue(skip, action);
        return false;
    }
    // 检查源文件是否存在
    if (!fromInfo->exists()) {
        qCritical() << " check file from file is  not exists !!!!!!!" << fromInfo->fileUrl();
        auto fromUrl = fromInfo->urlOf(UrlInfoType::kUrl);
        fromUrl.setPath(fromUrl.path().replace("\\", "/"));
        AbstractJobHandler::JobErrorType errortype = (fromInfo->pathOf(PathInfoType::kAbsolutePath).startsWith("/root/")
                                                      && !toInfo->pathOf(PathInfoType::kPath).startsWith("/root/"))
                ? AbstractJobHandler::JobErrorType::kPermissionError
                : AbstractJobHandler::JobErrorType::kNonexistenceError;
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromUrl,
                                                                        toInfo == nullptr ? QUrl() : toInfo->urlOf(UrlInfoType::kUrl), errortype);

        setSkipValue(skip, action);
        return false;
    }
    // 检查目标文件的文件信息
    if (!toInfo) {
        qCritical() << " check file to file perant info is  nullpter !!!!!!!";
        auto fromUrl = fromInfo->urlOf(UrlInfoType::kUrl);
        fromUrl.setPath(fromUrl.path().replace("\\", "/"));
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromUrl, QUrl(),
                                                                        AbstractJobHandler::JobErrorType::kProrogramError);
        setSkipValue(skip, action);
        return false;
    }
    // 检查目标文件是否存在
    if (!toInfo->exists()) {
        qCritical() << " check file to file perant file is  not exists !!!!!!!";
        AbstractJobHandler::JobErrorType errortype = (fromInfo->pathOf(PathInfoType::kPath).startsWith("/root/")
                                                      && !toInfo->pathOf(PathInfoType::kPath).startsWith("/root/"))
                ? AbstractJobHandler::JobErrorType::kPermissionError
                : AbstractJobHandler::JobErrorType::kNonexistenceError;
        auto fromUrl = fromInfo->urlOf(UrlInfoType::kUrl);
        fromUrl.setPath(fromUrl.path().replace("\\", "/"));
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromUrl,
                                                                        toInfo->urlOf(UrlInfoType::kUrl), errortype, true);
        setSkipValue(skip, action);
        return false;
    }
    // 特殊文件判断
    switch (fromInfo->fileType()) {
    case FileInfo::FileType::kCharDevice:
    case FileInfo::FileType::kBlockDevice:
    case FileInfo::FileType::kFIFOFile:
    case FileInfo::FileType::kSocketFile: {
        auto fromUrl = fromInfo->urlOf(UrlInfoType::kUrl);
        fromUrl.setPath(fromUrl.path().replace("\\", "/"));
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromUrl,
                                                                        toInfo->urlOf(UrlInfoType::kUrl),
                                                                        AbstractJobHandler::JobErrorType::kSpecialFileError);
        setSkipValue(skip, action);
        if (skip && *skip)
            workData->skipWriteSize += fromInfo->size() <= 0 ? workData->dirSize : fromInfo->size();

        return false;
    }
    default:
        break;
    }

    // 创建新的目标文件并做检查
    QString fileNewName = fileName;
    // bug 205732, 回收站文件找到源文件名称
    bool isTrashFile = FileUtils::isTrashFile(fromInfo->urlOf(UrlInfoType::kUrl));
    if (isTrashFile) {
        auto trashInfoUrl= trashInfo(fromInfo);
        fileNewName = fileOriginName(trashInfoUrl);
    }
    newTargetInfo.reset();
    if (!doCheckNewFile(fromInfo, toInfo, newTargetInfo, fileNewName, skip, true))
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
bool FileOperateBaseWorker::createSystemLink(const FileInfoPointer &fromInfo, const FileInfoPointer &toInfo,
                                             const bool followLink, const bool doCopy, bool *skip)
{
    // 创建链接文件
    FileInfoPointer newFromInfo = fromInfo;
    if (followLink) {
        QStringList pathList;
        QString pathValue = newFromInfo->urlOf(UrlInfoType::kUrl).path();
        pathValue = pathValue.endsWith(QDir::separator()) && pathValue != QDir::separator()
                ? QString(pathValue).left(pathValue.length() -1)
                : pathValue;
        pathList.append(pathValue);
        do {
            QUrl newUrl = newFromInfo->urlOf(UrlInfoType::kUrl);
            pathValue = newFromInfo->pathOf(PathInfoType::kSymLinkTarget);
            pathValue = pathValue.endsWith(QDir::separator()) && pathValue != QDir::separator()
                    ? QString(pathValue).left(pathValue.length() -1)
                    : pathValue;
            newUrl.setPath(pathValue);
            FileInfoPointer symlinkTarget = InfoFactory::create<FileInfo>(newUrl, Global::CreateFileInfoType::kCreateFileInfoSync);

            if (!symlinkTarget || !symlinkTarget->exists() || pathList.contains(pathValue)) {
                break;
            }
            pathList.append(pathValue);

            newFromInfo = symlinkTarget;
        } while (newFromInfo->isAttributes(OptInfoType::kIsSymLink));

        if (newFromInfo->exists() && doCopy) {
            // copy file here
            if (fromInfo->isAttributes(OptInfoType::kIsFile)) {
                return checkAndCopyFile(fromInfo, toInfo, skip);
            } else {
                return checkAndCopyDir(fromInfo, toInfo, skip);
            }
        }
    }

    AbstractJobHandler::SupportAction actionForlink { AbstractJobHandler::SupportAction::kNoAction };

    do {
        actionForlink = AbstractJobHandler::SupportAction::kNoAction;
        if (localFileHandler->createSystemLink(newFromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl))) {
            return true;
        }
        actionForlink = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl),
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
bool FileOperateBaseWorker::doCheckNewFile(const FileInfoPointer &fromInfo, const FileInfoPointer &toInfo,
                                           FileInfoPointer &newTargetInfo, QString &fileNewName, bool *skip, bool isCountSize)
{
    auto newTargetUrl = createNewTargetUrl(toInfo, fileNewName);
    if (createNewTargetInfo(fromInfo, toInfo, newTargetInfo, newTargetUrl, skip, isCountSize))
        return true;
    if (!workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyToSelf) && FileOperationsUtils::isAncestorUrl(fromInfo->urlOf(UrlInfoType::kUrl), newTargetUrl)) {
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl),
                                                                        toInfo->urlOf(UrlInfoType::kUrl),
                                                                        AbstractJobHandler::JobErrorType::kTargetIsSelfError);
        if (AbstractJobHandler::SupportAction::kSkipAction == action) {
            setSkipValue(skip, action);
            if (skip && *skip)
                workData->skipWriteSize += isCountSize && (fromInfo->isAttributes(OptInfoType::kIsSymLink) || fromInfo->size() <= 0) ? workData->dirSize : fromInfo->size();
            return false;
        }

        if (action != AbstractJobHandler::SupportAction::kEnforceAction) {
            setSkipValue(skip, action);
            if (skip && *skip)
                workData->skipWriteSize += isCountSize && (fromInfo->isAttributes(OptInfoType::kIsSymLink) || fromInfo->size() <= 0) ? workData->dirSize : fromInfo->size();
            return false;
        }
    };

    bool newTargetIsFile = newTargetInfo->isAttributes(OptInfoType::kIsFile) || newTargetInfo->isAttributes(OptInfoType::kIsSymLink);
    AbstractJobHandler::JobErrorType errortype = newTargetIsFile ? AbstractJobHandler::JobErrorType::kFileExistsError
                                                                 : AbstractJobHandler::JobErrorType::kDirectoryExistsError;
    AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl),
                                                                    newTargetInfo->urlOf(UrlInfoType::kUrl), errortype, true);
    switch (action) {
    case AbstractJobHandler::SupportAction::kReplaceAction: {
        const QVariant &var = doActionReplace(fromInfo, newTargetInfo, isCountSize);
        if (var.isValid()) {
            return var.toBool();
        }
        break;
    }
    case AbstractJobHandler::SupportAction::kMergeAction: {
        const QVariant &var = doActionMerge(fromInfo, newTargetInfo, isCountSize);
        if (var.isValid()) {
            return var.toBool();
        }
        break;
    }
    case AbstractJobHandler::SupportAction::kSkipAction: {
        workData->skipWriteSize += isCountSize && (fromInfo->isAttributes(OptInfoType::kIsSymLink) || fromInfo->size() <= 0) ? workData->dirSize : fromInfo->size();
        setSkipValue(skip, action);
        return false;
    }
    case AbstractJobHandler::SupportAction::kCoexistAction: {
        fileNewName = FileUtils::nonExistFileName(newTargetInfo, toInfo);
        if (fileNewName.isEmpty()) {
            return false;
        }
        auto newTargetUrl = createNewTargetUrl(toInfo, fileNewName);
        return createNewTargetInfo(fromInfo, toInfo, newTargetInfo, newTargetUrl, skip, isCountSize);
    }
    case AbstractJobHandler::SupportAction::kCancelAction: {
        stopWork.store(true);
        return false;
    }
    default:
        return false;
    }
    return true;
}

bool FileOperateBaseWorker::checkAndCopyFile(const FileInfoPointer fromInfo, const FileInfoPointer toInfo, bool *skip)
{
    // check file file size bigger than 4 GB
    if (!checkFileSize(fromInfo->size(), fromInfo->urlOf(UrlInfoType::kUrl),
                       toInfo->urlOf(UrlInfoType::kUrl), skip))
        return false;

    if (jobType == AbstractJobHandler::JobType::kCutType)
        return doCopyOtherFile(fromInfo, toInfo, skip);

    if (isSourceFileLocal && isTargetFileLocal && !workData->signalThread) {
        while (bigFileCopy && !isStopped()) {
            QThread::msleep(10);
        }
        if (fromInfo->size() > bigFileSize) {
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

bool FileOperateBaseWorker::checkAndCopyDir(const FileInfoPointer &fromInfo, const FileInfoPointer &toInfo, bool *skip)
{
    emitCurrentTaskNotify(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl));
    // 检查文件的一些合法性，源文件是否存在，创建新的目标目录名称，检查新创建目标目录名称是否存在
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    QFileDevice::Permissions permissions = fromInfo->permissions();
    if (!toInfo->exists()) {
        do {
            action = AbstractJobHandler::SupportAction::kNoAction;
            if (localFileHandler->mkdir(toInfo->urlOf(UrlInfoType::kUrl)))
                break;

            action = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl),
                                          AbstractJobHandler::JobErrorType::kMkdirError, true,
                                          localFileHandler->errorString());
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
            dirinfo->target = toInfo->urlOf(UrlInfoType::kUrl);
            dirinfo->permission = permissions;
            dirPermissonList.appendByLock(dirinfo);
            return true;
        }
    }

    // 遍历源文件，执行一个一个的拷贝
    QString error;
    const AbstractDirIteratorPointer &iterator = DirIteratorFactory::create<AbstractDirIterator>(fromInfo->urlOf(UrlInfoType::kUrl), &error);
    if (!iterator) {
        qCritical() << "create dir's iterator failed, case : " << error;
        doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl), AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }

    while (iterator->hasNext()) {
        if (!stateCheck()) {
            return false;
        }

        const QUrl &url = iterator->next();
        const FileInfoPointer &info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
        bool ok = doCopyFile(info, toInfo, skip);
        if (!ok && (!skip || !*skip)) {
            return false;
        }
    }

    if (isTargetFileLocal && isSourceFileLocal) {
        DirPermsissonPointer dirinfo(new DirSetPermissonInfo);
        dirinfo->target = toInfo->urlOf(UrlInfoType::kUrl);
        dirinfo->permission = permissions;
        dirPermissonList.appendByLock(dirinfo);
    } else {
        if (permissions && !FileUtils::isMtpFile(toInfo->urlOf(UrlInfoType::kUrl)))
            localFileHandler->setPermissions(toInfo->urlOf(UrlInfoType::kUrl), permissions);
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

QUrl FileOperateBaseWorker::trashInfo(const FileInfoPointer &fromInfo)
{
    auto parentPath = fromInfo->urlOf(UrlInfoType::kParentUrl).path();
    if (!parentPath.endsWith("files"))
        return QUrl();
    auto fileName = fromInfo->nameOf(NameInfoType::kFileName);
    auto trashInfoUrl = QUrl::fromLocalFile(parentPath.replace("files", "info/") + fileName + ".trashinfo");
    return trashInfoUrl;
}

QString FileOperateBaseWorker::fileOriginName(const QUrl &trashInfoUrl)
{
    if (!trashInfoUrl.isValid())
        return QString();
    DFile file(trashInfoUrl);
    if (!file.open(dfmio::DFile::OpenFlag::kReadOnly)) {
        qWarning() << "open trash file info err : " << file.lastError().errorMsg() << " ,trashInfoUrl = " << trashInfoUrl;
        return QString();
    }
    auto data = file.readAll().simplified().split(' ');
    // trash info file readAll() = "[Trash Info] Path=%E6%96%B0%E5%BB%BAWord%E6%96%87%E6%A1%A3.doc DeletionDate=2023-05-05T11:19:06";
    // has three char " ", so data has 4 item, the 3th is the "Path=%E6%96%B0%E5%BB%BAWord%E6%96%87%E6%A1%A3.doc"
    if (data.size() <= 3) {
        qWarning() << "reade trash file info err,trashInfoUrl = " << trashInfoUrl;
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
    qDebug() << "delete trash file info. trashInfoUrl = " << trashInfoUrl;
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

QUrl FileOperateBaseWorker::createNewTargetUrl(const FileInfoPointer &toInfo, const QString &fileName)
{
    QString fileNewName = formatFileName(fileName);
    // 创建文件的名称
    QUrl newTargetUrl = toInfo->urlOf(UrlInfoType::kUrl);
    const QString &newTargetPath = newTargetUrl.path();
    const QString &newPath = DFMIO::DFMUtils::buildFilePath(newTargetPath.toStdString().c_str(), fileNewName.toStdString().c_str(), nullptr);
    newTargetUrl.setPath(newPath);
    return newTargetUrl;
}

bool FileOperateBaseWorker::doCopyLocalFile(const FileInfoPointer fromInfo, const FileInfoPointer toInfo)
{
    if (!stateCheck())
        return false;

    QtConcurrent::run(threadPool.data(), threadCopyWorker[threadCopyFileCount % threadCount].data(),
                      static_cast<void (DoCopyFileWorker::*)(const FileInfoPointer, const FileInfoPointer)>(&DoCopyFileWorker::doFileCopy),
                      fromInfo, toInfo);

    threadCopyFileCount++;
    return true;
}

bool FileOperateBaseWorker::doCopyLocalBigFile(const FileInfoPointer fromInfo, const FileInfoPointer toInfo, bool *skip)
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
        munmap(fromPoint, static_cast<size_t>(fromInfo->size()));
        close(fromFd);
        close(toFd);
        return false;
    }
    // memcpy file in other thread
    memcpyLocalBigFile(fromInfo, toInfo, fromPoint, toPoint);
    // wait copy
    waitThreadPoolOver();
    // clear
    doCopyLocalBigFileClear(static_cast<size_t>(fromInfo->size()), fromFd, toFd, fromPoint, toPoint);
    // set permissions
    setTargetPermissions(fromInfo, toInfo);
    return true;
}

bool FileOperateBaseWorker::doCopyLocalBigFileResize(const FileInfoPointer fromInfo, const FileInfoPointer toInfo, int toFd, bool *skip)
{
    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    do {
        __off_t length = fromInfo->size();
        action = AbstractJobHandler::SupportAction::kNoAction;
        if (-1 == ftruncate(toFd, length)) {
            auto lastError = strerror(errno);
            qWarning() << "file resize error, url from: " << fromInfo->urlOf(UrlInfoType::kUrl)
                       << " url to: " << toInfo->urlOf(UrlInfoType::kUrl) << " open flag: " << O_RDONLY
                       << " error code: " << errno << " error msg: " << lastError;

            action = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl),
                                          AbstractJobHandler::JobErrorType::kResizeError, true, lastError);
        }
    } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());

    checkRetry();

    if (!actionOperating(action, fromInfo->size() <= 0 ? FileUtils::getMemoryPageSize() : fromInfo->size(), skip))
        return false;

    return true;
}

char *FileOperateBaseWorker::doCopyLocalBigFileMap(const FileInfoPointer fromInfo, const FileInfoPointer toInfo, int fd, const int per, bool *skip)
{
    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    void *point = nullptr;
    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        point = mmap(nullptr, static_cast<size_t>(fromInfo->size()),
                     per, MAP_SHARED, fd, 0);
        if (!point || point == MAP_FAILED) {
            auto lastError = strerror(errno);
            qWarning() << "file mmap error, url from: " << fromInfo->urlOf(UrlInfoType::kUrl)
                       << " url to: " << fromInfo->urlOf(UrlInfoType::kUrl)
                       << " error code: " << errno << " error msg: " << lastError;

            action = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl),
                                          AbstractJobHandler::JobErrorType::kOpenError, fd == PROT_WRITE, lastError);
        }
    } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());

    checkRetry();

    if (!actionOperating(action, fromInfo->size() <= 0 ? FileUtils::getMemoryPageSize() : fromInfo->size(), skip))
        return nullptr;

    return static_cast<char *>(point);
}

void FileOperateBaseWorker::memcpyLocalBigFile(const FileInfoPointer fromInfo, const FileInfoPointer toInfo, char *fromPoint, char *toPoint)
{
    auto offset = fromInfo->size() / threadCount;
    char *fromPointStart = fromPoint;
    char *toPointStart = toPoint;
    for (int i = 0; i < threadCount; i++) {
        offset = (i == (threadCount - 1) ? fromInfo->size() - (threadCount - 1) * offset : offset);

        char *tempfFromPointStart = fromPointStart;
        char *tempfToPointStart = toPointStart;
        size_t tempOffet = static_cast<size_t>(offset);
        QtConcurrent::run(threadPool.data(), threadCopyWorker[i].data(),
                          static_cast<void (DoCopyFileWorker::*)(const FileInfoPointer fromInfo,
                                                                 const FileInfoPointer toInfo,
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

int FileOperateBaseWorker::doOpenFile(const FileInfoPointer fromInfo, const FileInfoPointer toInfo, const bool isTo, const int openFlag, bool *skip)
{
    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    emitCurrentTaskNotify(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl));
    int fd = -1;
    do {
        QUrl url = isTo ? toInfo->urlOf(UrlInfoType::kUrl) : fromInfo->urlOf(UrlInfoType::kUrl);
        std::string path = url.path().toStdString();
        fd = open(path.c_str(), openFlag, 0666);
        action = AbstractJobHandler::SupportAction::kNoAction;
        if (fd < 0) {
            auto lastError = strerror(errno);
            qWarning() << "file open error, url from: " << fromInfo->urlOf(UrlInfoType::kUrl)
                       << " url to: " << fromInfo->urlOf(UrlInfoType::kUrl) << " open flag: " << openFlag
                       << " error code: " << errno << " error msg: " << lastError;

            action = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl),
                                          AbstractJobHandler::JobErrorType::kOpenError, isTo, lastError);
        }
    } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());

    checkRetry();

    if (!actionOperating(action, fromInfo->size() <= 0 ? FileUtils::getMemoryPageSize() : fromInfo->size(), skip)) {
        if (fd >= 0)
            close(fd);
        return -1;
    }

    return fd;
}

bool FileOperateBaseWorker::doCopyOtherFile(const FileInfoPointer fromInfo, const FileInfoPointer toInfo, bool *skip)
{
    initSignalCopyWorker();
    const QString &targetUrl = toInfo->urlOf(UrlInfoType::kUrl).toString();

    FileUtils::cacheCopyingFileUrl(targetUrl);
    bool ok{ false };
    if (fromInfo->size() > bigFileSize || !supportDfmioCopy || workData->exBlockSyncEveryWrite) {
        ok = copyOtherFileWorker->doCopyFilePractically(fromInfo, toInfo, skip);
    } else {
        ok = copyOtherFileWorker->doDfmioFileCopy(fromInfo, toInfo, skip);
    }
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

bool FileOperateBaseWorker::createNewTargetInfo(const FileInfoPointer &fromInfo, const FileInfoPointer &toInfo, FileInfoPointer &newTargetInfo, const QUrl &fileNewUrl, bool *skip, bool isCountSize)
{
    newTargetInfo.reset();

    QString error;
    newTargetInfo = InfoFactory::create<FileInfo>(fileNewUrl, Global::CreateFileInfoType::kCreateFileInfoSync, &error);

    if (!newTargetInfo || !error.isEmpty()) {
        qWarning() << "newTargetInfo is null = " << !newTargetInfo << ", error message = " << error;
        AbstractJobHandler::SupportAction action =
                doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl),
                                     toInfo->urlOf(UrlInfoType::kUrl),
                                     AbstractJobHandler::JobErrorType::kProrogramError);
        setSkipValue(skip, action);
        if (skip && *skip)
            workData->skipWriteSize += (isCountSize && (fromInfo->isAttributes(OptInfoType::kIsSymLink) || fromInfo->size() <= 0)) ? workData->dirSize : fromInfo->size();

        return false;
    }

    if (!newTargetInfo->exists())
        return true;

    return false;
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

QVariant FileOperateBaseWorker::checkLinkAndSameUrl(const FileInfoPointer &fromInfo, const FileInfoPointer &newTargetInfo, const bool isCountSize)
{
    if (newTargetInfo->isAttributes(OptInfoType::kIsSymLink)) {
        LocalFileHandler handler;
        if (!handler.deleteFile(newTargetInfo->urlOf(UrlInfoType::kUrl)))
            return false;
    }

    const QUrl &newTargetUrl = newTargetInfo->urlOf(UrlInfoType::kUrl);
    if (newTargetUrl == fromInfo->urlOf(UrlInfoType::kUrl)) {
        workData->skipWriteSize += (isCountSize && (fromInfo->isAttributes(OptInfoType::kIsSymLink) || fromInfo->size() <= 0)) ? workData->dirSize : fromInfo->size();
        return true;
    }

    return QVariant();
}

QVariant FileOperateBaseWorker::doActionReplace(const FileInfoPointer &fromInfo, const FileInfoPointer &newTargetInfo, const bool isCountSize)
{
    const QVariant &var = checkLinkAndSameUrl(fromInfo, newTargetInfo, isCountSize);
    if (var.isValid())
        return var;

    const bool fromIsFile = fromInfo->isAttributes(OptInfoType::kIsFile) || fromInfo->isAttributes(OptInfoType::kIsSymLink);
    const bool newTargetIsFile = newTargetInfo->isAttributes(OptInfoType::kIsFile) || newTargetInfo->isAttributes(OptInfoType::kIsSymLink);

    if (fromIsFile == newTargetIsFile) {
        return QVariant();
    } else {
        return false;
    }
}

QVariant FileOperateBaseWorker::doActionMerge(const FileInfoPointer &fromInfo, const FileInfoPointer &newTargetInfo, const bool isCountSize)
{
    const bool fromIsFile = fromInfo->isAttributes(OptInfoType::kIsFile) || fromInfo->isAttributes(OptInfoType::kIsSymLink);
    const bool newTargetIsFile = newTargetInfo->isAttributes(OptInfoType::kIsFile) || newTargetInfo->isAttributes(OptInfoType::kIsSymLink);

    if (!fromIsFile && !newTargetIsFile) {
        // target is dir, do merged
        return QVariant();
    } else if (fromIsFile && newTargetIsFile) {
        return checkLinkAndSameUrl(fromInfo, newTargetInfo, isCountSize);
    } else {
        return false;
    }
}

bool FileOperateBaseWorker::doCopyFile(const FileInfoPointer &fromInfo, const FileInfoPointer &toInfo, bool *skip)
{
    FileInfoPointer newTargetInfo(nullptr);
    bool result = false;
    if (!doCheckFile(fromInfo, toInfo,
                     fromInfo->nameOf(NameInfoType::kFileCopyName), newTargetInfo, skip))
        return result;

    if (fromInfo->isAttributes(OptInfoType::kIsSymLink)) {
        result = createSystemLink(fromInfo, newTargetInfo, workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyFollowSymlink), true, skip);
        if (result)
            workData->zeroOrlinkOrDirWriteSize += (newTargetInfo->size() > 0 ? newTargetInfo->size() : FileUtils::getMemoryPageSize());
    } else if (fromInfo->isAttributes(OptInfoType::kIsDir)) {
        result = checkAndCopyDir(fromInfo, newTargetInfo, skip);
        if (result || skip)
            workData->zeroOrlinkOrDirWriteSize += workData->dirSize;
    } else {
        result = checkAndCopyFile(fromInfo, newTargetInfo, skip);
    }

    if (targetInfo == toInfo) {
        completeSourceFiles.append(fromInfo->urlOf(UrlInfoType::kUrl));
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
    auto rootPath = DFMUtils::mountPathFromUrl(targetOrgUrl);
    auto device = DFMUtils::deviceNameFromUrl(targetOrgUrl);
    if (device.startsWith("/dev/")) {
        isTargetFileLocal = FileOperationsUtils::isFileOnDisk(targetOrgUrl);
        isTargetFileExBlock = false;
        qDebug("Target block device: \"%s\", Root Path: \"%s\"", device.toStdString().data(), qPrintable(rootPath));
        const bool isFileSystemTypeExt = DFMUtils::fsTypeFromUrl(targetOrgUrl).startsWith("ext");
        if (!isFileSystemTypeExt) {
            blocakTargetRootPath = rootPath;
            QProcess process;
            process.start("lsblk", { "-niro", "MAJ:MIN,HOTPLUG,LOG-SEC", device }, QIODevice::ReadOnly);

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
                            workData->exBlockSyncEveryWrite = FileOperationsUtils::blockSync();
                            countWriteType = workData->exBlockSyncEveryWrite ?
                                        CountWriteSizeType::kCustomizeType :
                                        CountWriteSizeType::kWriteBlockType;
                            workData->isBlockDevice = true;
                            targetDeviceStartSectorsWritten = workData->exBlockSyncEveryWrite ?
                                        0 : getSectorsWritten();
                        }

                        qDebug("Block device path: \"%s\", Sys dev path: \"%s\", Is removable: %d, Log-Sec: %d",
                               qPrintable(device), qPrintable(targetSysDevPath), bool(targetIsRemovable), targetLogSecionSize);
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
}

void FileOperateBaseWorker::syncFilesToDevice()
{
    if (CountWriteSizeType::kWriteBlockType != countWriteType)
        return;

    qInfo() << "start sync all file to extend block device!!!!! target : " << targetUrl;
    for (const auto &url : completeTargetFiles) {
        std::string stdStr = url.path().toUtf8().toStdString();
        int tofd = open(stdStr.data(), O_RDONLY);
        if (-1 != tofd) {
            syncfs(tofd);
            close(tofd);
        }
    }

    qInfo() << "end sync all file to extend block device!!!!! target : " << targetUrl;

    qDebug() << __FUNCTION__ << "syncFilesToDevice begin";
    qint64 writeSize = getWriteDataSize();
    while (!isStopped() && sourceFilesTotalSize > 0 && writeSize < sourceFilesTotalSize) {
        QThread::msleep(100);
        writeSize = getWriteDataSize();
    }
    qDebug() << __FUNCTION__ << "syncFilesToDevice end";
}
