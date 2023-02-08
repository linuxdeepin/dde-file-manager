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
#include "fileoperations/copyfiles/storageinfo.h"
#include "workerdata.h"

#include "dfm-base/interfaces/abstractdiriterator.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/utils/decorator/decoratorfileenumerator.h"
#include "dfm-base/utils/decorator/decoratorfile.h"
#include "dfm-base/utils/decorator/decoratorfileinfo.h"
#include "dfm-base/file/local/localfilehandler.h"

#include <dfm-io/core/diofactory.h>
#include <dfm-io/dfmio_register.h>
#include <dfm-io/dfmio_utils.h>

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

constexpr uint32_t kBigFileSize { 300 * 1024 * 1024 };

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

    if (FileUtils::isSameFile(urlFrom, urlTo, false)) {
        currentAction = AbstractJobHandler::SupportAction::kCoexistAction;
        return currentAction;
    }

    // 发送错误处理 阻塞自己
    emitErrorNotify(urlFrom, urlTo, error, quintptr(this), isTo, errorMsg, errorMsgAll);
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
void FileOperateBaseWorker::setTargetPermissions(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo)
{
    // 修改文件修改时间
    localFileHandler->setFileTime(toInfo->urlOf(UrlInfoType::kUrl),
                                  fromInfo->timeOf(TimeInfoType::kLastRead).value<QDateTime>(),
                                  fromInfo->timeOf(TimeInfoType::kLastModified).value<QDateTime>());
    QFileDevice::Permissions permissions = fromInfo->permissions();
    QString path = fromInfo->urlOf(UrlInfoType::kUrl).path();
    //权限为0000时，源文件已经被删除，无需修改新建的文件的权限为0000
    if (permissions != 0000)
        localFileHandler->setPermissions(toInfo->urlOf(UrlInfoType::kUrl), permissions);
}

/*!
 * \brief FileOperateBaseWorker::readAheadSourceFile Pre read source file content
 * \param fileInfo File information of source file
 */
void FileOperateBaseWorker::readAheadSourceFile(const AbstractFileInfoPointer &fileInfo)
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
                                                    QSharedPointer<StorageInfo> targetStorageInfo,
                                                    bool *skip)
{
    if (!targetStorageInfo)
        return true;

    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

    do {
        targetStorageInfo->refresh();
        qint64 freeBytes = targetStorageInfo->bytesFree();

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
    DecoratorFileEnumerator enumerator(fromUrl);
    if (!enumerator.isValid())
        return false;

    bool succ = false;
    while (enumerator.hasNext()) {
        const QUrl &url = enumerator.next();

        if (DecoratorFileInfo(url).isDir()) {
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
    auto fileinfo = InfoFactory::create<AbstractFileInfo>(urlSource);
    if (fileinfo->isAttributes(OptInfoType::kIsDir)) {
        if (!DecoratorFile(urlTarget).exists()) {
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
            auto fileinfoNext = InfoFactory::create<AbstractFileInfo>(url);

            AbstractFileInfoPointer newTargetInfo(nullptr);
            bool ok = false;
            AbstractFileInfoPointer toInfo = InfoFactory::create<AbstractFileInfo>(urlTarget);
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
bool FileOperateBaseWorker::copyAndDeleteFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &targetPathInfo, const AbstractFileInfoPointer &toInfo, bool *skip)
{
    bool ok = false;
    if (!toInfo)
        return false;

    bool oldExist = DecoratorFile(toInfo->urlOf(UrlInfoType::kUrl)).exists();

    if (fromInfo->isAttributes(OptInfoType::kIsSymLink)) {
        ok = createSystemLink(fromInfo, toInfo, workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyFollowSymlink), true, skip);
        if (ok)
            ok = deleteFile(fromInfo->urlOf(UrlInfoType::kUrl), targetPathInfo->urlOf(UrlInfoType::kUrl), skip);
    } else if (fromInfo->isAttributes(OptInfoType::kIsDir)) {
        ok = checkAndCopyDir(fromInfo, toInfo, skip);
        if (ok)
            ok = deleteDir(fromInfo->urlOf(UrlInfoType::kUrl), targetPathInfo->urlOf(UrlInfoType::kUrl), skip);
    } else {
        const QUrl &url = toInfo->urlOf(UrlInfoType::kUrl);

        FileUtils::cacheCopyingFileUrl(url);
        initSignalCopyWorker();
        ok = copyOtherFileWorker->doCopyFilePractically(fromInfo, toInfo, skip);
        if (ok)
            ok = deleteFile(fromInfo->urlOf(UrlInfoType::kUrl), targetPathInfo->urlOf(UrlInfoType::kUrl), skip);
        FileUtils::removeCopyingFileUrl(url);
    }

    if (!oldExist && DecoratorFile(toInfo->urlOf(UrlInfoType::kUrl)).exists() && targetInfo == targetPathInfo) {
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
bool FileOperateBaseWorker::doCheckFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, const QString &fileName,
                                        AbstractFileInfoPointer &newTargetInfo, bool *skip)
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
    if (!DecoratorFile(fromInfo->urlOf(UrlInfoType::kUrl)).exists()) {
        qCritical() << " check file from file is  not exists !!!!!!!";
        AbstractJobHandler::JobErrorType errortype = (fromInfo->pathOf(PathInfoType::kPath).startsWith("/root/")
                                                      && !toInfo->pathOf(PathInfoType::kPath).startsWith("/root/"))
                ? AbstractJobHandler::JobErrorType::kPermissionError
                : AbstractJobHandler::JobErrorType::kNonexistenceError;
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl),
                                                                        toInfo == nullptr ? QUrl() : toInfo->urlOf(UrlInfoType::kUrl), errortype);

        setSkipValue(skip, action);
        return false;
    }
    // 检查目标文件的文件信息
    if (!toInfo) {
        qCritical() << " check file to file perant info is  nullpter !!!!!!!";
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl), QUrl(),
                                                                        AbstractJobHandler::JobErrorType::kProrogramError);
        setSkipValue(skip, action);
        return false;
    }
    // 检查目标文件是否存在
    if (!DecoratorFile(toInfo->urlOf(UrlInfoType::kUrl)).exists()) {
        qCritical() << " check file to file perant file is  not exists !!!!!!!";
        AbstractJobHandler::JobErrorType errortype = (fromInfo->pathOf(PathInfoType::kPath).startsWith("/root/")
                                                      && !toInfo->pathOf(PathInfoType::kPath).startsWith("/root/"))
                ? AbstractJobHandler::JobErrorType::kPermissionError
                : AbstractJobHandler::JobErrorType::kNonexistenceError;
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl),
                                                                        toInfo->urlOf(UrlInfoType::kUrl), errortype, true);
        setSkipValue(skip, action);
        return false;
    }
    // 特殊文件判断
    switch (fromInfo->fileType()) {
    case AbstractFileInfo::FileType::kCharDevice:
    case AbstractFileInfo::FileType::kBlockDevice:
    case AbstractFileInfo::FileType::kFIFOFile:
    case AbstractFileInfo::FileType::kSocketFile: {
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(fromInfo->urlOf(UrlInfoType::kUrl),
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
bool FileOperateBaseWorker::createSystemLink(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                             const bool followLink, const bool doCopy, bool *skip)
{
    // 创建链接文件
    AbstractFileInfoPointer newFromInfo = fromInfo;
    if (followLink) {
        do {
            QUrl newUrl = newFromInfo->urlOf(UrlInfoType::kUrl);
            newUrl.setPath(newFromInfo->pathOf(PathInfoType::kSymLinkTarget));
            const AbstractFileInfoPointer &symlinkTarget = InfoFactory::create<AbstractFileInfo>(newUrl);

            if (!symlinkTarget || !DecoratorFile(newUrl).exists()) {
                break;
            }

            newFromInfo = symlinkTarget;
        } while (newFromInfo->isAttributes(OptInfoType::kIsSymLink));

        if (DecoratorFile(newFromInfo->urlOf(UrlInfoType::kUrl)).exists() && doCopy) {
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
bool FileOperateBaseWorker::doCheckNewFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                           AbstractFileInfoPointer &newTargetInfo, QString &fileNewName, bool *skip, bool isCountSize)
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

bool FileOperateBaseWorker::checkAndCopyFile(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo, bool *skip)
{
    if (!checkDiskSpaceAvailable(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl), targetStorageInfo, skip)) {
        if (skip && *skip)
            workData->skipWriteSize += fromInfo->size();
        return false;
    }
    checkRetry();
    if (isSourceFileLocal && isTargetFileLocal && !workData->signalThread) {
        while (bigFileCopy && !isStopped()) {
            QThread::msleep(10);
        }
        if (fromInfo->size() > kBigFileSize) {
            bigFileCopy = true;
            auto result = doCopyLocalBigFile(fromInfo, toInfo, skip);
            bigFileCopy = false;
            return result;
        }
        return doCopyLocalFile(fromInfo, toInfo);
    }
    // copy block files
    if (!isTargetFileLocal && isTargetFileExBlock) {
        return doCopyExBlockFile(fromInfo, toInfo);
    }

    // copy other file or cut file
    return doCopyOtherFile(fromInfo, toInfo, skip);
}

bool FileOperateBaseWorker::checkAndCopyDir(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *skip)
{
    emitCurrentTaskNotify(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl));
    // 检查文件的一些合法性，源文件是否存在，创建新的目标目录名称，检查新创建目标目录名称是否存在
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    QFileDevice::Permissions permissions = fromInfo->permissions();
    if (!DecoratorFile(toInfo->urlOf(UrlInfoType::kUrl)).exists()) {
        do {
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

    if (fromInfo->countChildFile() <= 0) {
        //权限为0000时，源文件已经被删除，无需修改新建的文件的权限为0000
        if (permissions != 0000)
            localFileHandler->setPermissions(toInfo->urlOf(UrlInfoType::kUrl), permissions);
        return true;
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
        Q_UNUSED(url);
        const AbstractFileInfoPointer &info = iterator->fileInfo();
        bool ok = doCopyFile(info, toInfo, skip);
        if (!ok && !skip) {
            return false;
        }
    }

    if (isTargetFileLocal && isSourceFileLocal) {
        DirPermsissonPointer dirinfo(new DirSetPermissonInfo);
        dirinfo->target = toInfo->urlOf(UrlInfoType::kUrl);
        dirinfo->permission = permissions;
        dirPermissonList.appendByLock(dirinfo);
    } else if (isTargetFileExBlock) {
        createExBlockFileCopyInfo(fromInfo, toInfo, 0, false, 0, nullptr, true, permissions);
        startBlockFileCopy();
    } else {
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
    bool isExBlockWriteOverFlag = false;
    // wait thread pool copy local file or copy big file over
    while (!isStopped() && threadPool && threadPool->activeThreadCount() > 0) {
        if (isTargetFileExBlock && workData->blockCopyInfoQueue.size() == 0 && threadPool->activeThreadCount() == 1 && !isExBlockWriteOverFlag) {
            // do last block file write
            isExBlockWriteOverFlag = true;
            createExBlockFileCopyInfo(nullptr, nullptr, 0, true, -1);
        }
        QThread::msleep(10);
    }
}

void FileOperateBaseWorker::initCopyWay()
{
    // local file useing least 8 thread
    if (isSourceFileLocal && isTargetFileLocal) {
        countWriteType = CountWriteSizeType::kCustomizeType;
        workData->signalThread = (sourceFilesCount > 1 || sourceFilesTotalSize > kBigFileSize) && FileUtils::getCpuProcessCount() > 4
                ? false
                : true;
        if (!workData->signalThread)
            threadCount = FileUtils::getCpuProcessCount() >= 8 ? FileUtils::getCpuProcessCount() : 8;
    }
    // copy to extra block device use 2 thread
    if (isTargetFileExBlock) {
        threadCount = 2;
        workData->signalThread = false;
    }

    if (DeviceUtils::isSamba(targetUrl) || DeviceUtils::isFtp(targetUrl))
        countWriteType = CountWriteSizeType::kCustomizeType;

    if (!workData->signalThread) {
        initThreadCopy();
    }

    copyTid = (countWriteType == CountWriteSizeType::kTidType) ? syscall(SYS_gettid) : -1;
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

QUrl FileOperateBaseWorker::createNewTargetUrl(const AbstractFileInfoPointer &toInfo, const QString &fileName)
{
    QString fileNewName = formatFileName(fileName);
    // 创建文件的名称
    QUrl newTargetUrl = toInfo->urlOf(UrlInfoType::kUrl);
    const QString &newTargetPath = newTargetUrl.path();
    const QString &newPath = DFMIO::DFMUtils::buildFilePath(newTargetPath.toStdString().c_str(), fileNewName.toStdString().c_str(), nullptr);
    newTargetUrl.setPath(newPath);
    return newTargetUrl;
}

bool FileOperateBaseWorker::doCopyLocalFile(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo)
{
    if (!stateCheck())
        return false;

    QtConcurrent::run(threadPool.data(), threadCopyWorker[threadCopyFileCount % threadCount].data(),
                      static_cast<void (DoCopyFileWorker::*)(const AbstractFileInfoPointer, const AbstractFileInfoPointer)>(&DoCopyFileWorker::doFileCopy),
                      fromInfo, toInfo);

    threadCopyFileCount++;
    return true;
}

bool FileOperateBaseWorker::doCopyLocalBigFile(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo, bool *skip)
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

bool FileOperateBaseWorker::doCopyLocalBigFileResize(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo, int toFd, bool *skip)
{
    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    do {
        __off_t length = fromInfo->size();
        if (-1 == ftruncate(toFd, length)) {
            auto lastError = strerror(errno);
            qWarning() << "file resize error, url from: " << fromInfo->urlOf(UrlInfoType::kUrl)
                       << " url to: " << fromInfo->urlOf(UrlInfoType::kUrl) << " open flag: " << O_RDONLY
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

char *FileOperateBaseWorker::doCopyLocalBigFileMap(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo, int fd, const int per, bool *skip)
{
    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    void *point = nullptr;
    do {
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

void FileOperateBaseWorker::memcpyLocalBigFile(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo, char *fromPoint, char *toPoint)
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
                          static_cast<void (DoCopyFileWorker::*)(const AbstractFileInfoPointer fromInfo,
                                                                 const AbstractFileInfoPointer toInfo,
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

bool FileOperateBaseWorker::doCopyExBlockFile(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo)
{
    if (!stateCheck())
        return false;

    QtConcurrent::run(threadPool.data(), threadCopyWorker[0].data(),
                      static_cast<void (DoCopyFileWorker::*)(const AbstractFileInfoPointer fromInfo,
                                                             const AbstractFileInfoPointer toInfo)>(&DoCopyFileWorker::readExblockFile),
                      fromInfo, toInfo);

    startBlockFileCopy();

    return true;
}

int FileOperateBaseWorker::doOpenFile(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo, const bool isTo, const int openFlag, bool *skip)
{
    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    emitCurrentTaskNotify(fromInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl));
    int fd = -1;
    do {
        QUrl url = isTo ? toInfo->urlOf(UrlInfoType::kUrl) : fromInfo->urlOf(UrlInfoType::kUrl);
        std::string path = url.path().toStdString();
        fd = open(path.c_str(), openFlag, 0666);
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

bool FileOperateBaseWorker::doCopyOtherFile(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo, bool *skip)
{
    initSignalCopyWorker();
    const QString &targetUrl = toInfo->urlOf(UrlInfoType::kUrl).toString();

    FileUtils::cacheCopyingFileUrl(targetUrl);
    bool ok = copyOtherFileWorker->doCopyFilePractically(fromInfo, toInfo, skip);
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

void FileOperateBaseWorker::createExBlockFileCopyInfo(const AbstractFileInfoPointer fromInfo,
                                                      const AbstractFileInfoPointer toInfo,
                                                      const qint64 currentPos,
                                                      const bool closeFlag,
                                                      const qint64 size,
                                                      char *buffer,
                                                      const bool isDir,
                                                      const QFileDevice::Permissions permission)
{
    BlockFileCopyInfoPointer tmpinfo(new WorkerData::BlockFileCopyInfo());
    tmpinfo->closeflag = closeFlag;
    tmpinfo->frominfo = fromInfo;
    tmpinfo->toinfo = toInfo;
    tmpinfo->currentpos = currentPos;
    tmpinfo->buffer = buffer;
    tmpinfo->size = size;
    tmpinfo->isdir = isDir;
    tmpinfo->permission = permission;
    workData->blockCopyInfoQueue.push_backByLock(tmpinfo);
}

void FileOperateBaseWorker::startBlockFileCopy()
{
    if (!exblockThreadStarted && stateCheck()) {
        exblockThreadStarted = true;
        QtConcurrent::run(threadPool.data(), threadCopyWorker[1].data(),
                          static_cast<void (DoCopyFileWorker::*)()>(&DoCopyFileWorker::writeExblockFile));
    }
}

bool FileOperateBaseWorker::createNewTargetInfo(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, AbstractFileInfoPointer &newTargetInfo, const QUrl &fileNewUrl, bool *skip, bool isCountSize)
{
    newTargetInfo.reset();

    QString error;
    newTargetInfo = InfoFactory::create<AbstractFileInfo>(fileNewUrl, false, &error);

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

    if (!DecoratorFile(newTargetInfo->urlOf(UrlInfoType::kUrl)).exists())
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

QVariant FileOperateBaseWorker::checkLinkAndSameUrl(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &newTargetInfo, const bool isCountSize)
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

QVariant FileOperateBaseWorker::doActionReplace(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &newTargetInfo, const bool isCountSize)
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

QVariant FileOperateBaseWorker::doActionMerge(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &newTargetInfo, const bool isCountSize)
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

bool FileOperateBaseWorker::doCopyFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *skip)
{
    AbstractFileInfoPointer newTargetInfo(nullptr);
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

void FileOperateBaseWorker::setAllDirPermisson()
{
    for (auto info : dirPermissonList.list()) {
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

    if (!targetStorageInfo)
        targetStorageInfo.reset(new StorageInfo(targetUrl.path()));

    qDebug("Target block device: \"%s\", Root Path: \"%s\"", targetStorageInfo->device().constData(), qPrintable(targetStorageInfo->rootPath()));

    if (targetStorageInfo->isLocalDevice()) {
        isTargetFileLocal = FileOperationsUtils::isFileOnDisk(targetUrl);
        isTargetFileExBlock = DeviceUtils::isExternalBlock(targetUrl);

        const bool isFileSystemTypeExt = targetStorageInfo->fileSystemType().startsWith("ext");
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
