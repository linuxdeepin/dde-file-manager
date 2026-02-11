// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileoperatebaseworker.h"
#include "dfm-base/dfm_log_defines.h"
#include "filenameutils.h"
#include "fileoperations/fileoperationutils/fileoperationsutils.h"
#include "workerdata.h"

#include <dfm-base/interfaces/abstractdiriterator.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/protocolutils.h>

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
#include <sys/sysmacros.h>
#include <linux/fs.h>

#include "sync_interface_qt6.h"

DPFILEOPERATIONS_USE_NAMESPACE
USING_IO_NAMESPACE

/*!
 * \brief 为文件操作准备替换目标
 *
 * 仅普通文件需要使用临时文件策略进行安全替换。
 * 符号链接和目录直接使用原始目标，无需临时文件。
 *
 * \param finalInfo 最终目标文件信息
 * \param fromInfo 源文件信息（用于判断文件类型）
 * \param cleanupManager 清理管理器（用于追踪临时文件）
 * \return 替换目标结构
 */
static FileOperateBaseWorker::ReplacementTarget prepareReplacementTarget(
        const DFileInfoPointer &finalInfo,
        const DFileInfoPointer &fromInfo,
        FileCleanupManager &cleanupManager)
{
    if (!finalInfo || !fromInfo) {
        fmWarning() << "Invalid file info pointers in prepareReplacementTarget";
        return FileOperateBaseWorker::ReplacementTarget();
    }

    FileOperateBaseWorker::ReplacementTarget rt;
    rt.actualInfo = finalInfo;

    // 仅普通文件需要替换处理
    const bool isSymlink = fromInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool();
    const bool isDir = fromInfo->attribute(DFileInfo::AttributeID::kStandardIsDir).toBool();

    if (isSymlink || isDir) {
        // 符号链接和目录直接使用原始目标，不创建替换上下文
        return rt;
    }

    // 普通文件：尝试创建替换上下文
    rt.ctx = FileReplacer::createReplacementContext(finalInfo->uri().path());

    if (rt.ctx.isReplacement()) {
        // 需要临时文件
        rt.actualInfo.reset(new DFileInfo(QUrl::fromLocalFile(rt.ctx.temporaryFilePath)));
        rt.actualInfo->initQuerier();
        cleanupManager.trackIncompleteFile(rt.actualInfo->uri());
        fmDebug() << "Will use temporary file for replacement:" << rt.ctx.temporaryFilePath;
    }

    return rt;
}

/*!
 * \brief 应用文件替换（如果需要）
 *
 * 如果存在替换上下文，执行原子替换操作：
 * 1. 删除原目标文件
 * 2. 原子重命名临时文件到目标位置
 * 3. 确认临时文件操作完成
 *
 * 如果没有替换操作，直接返回成功。
 *
 * \param rt 替换目标（包含上下文和临时文件信息）
 * \param cleanupManager 清理管理器
 * \return 替换是否成功（或无需替换）
 */
static bool applyReplacementIfNeeded(
        FileOperateBaseWorker::ReplacementTarget &rt,
        FileCleanupManager &cleanupManager)
{
    if (!rt.isUsingTemporary()) {
        return true;   // 没有替换操作，直接成功
    }

    fmDebug() << "Applying replacement:" << rt.ctx.originalTargetPath;
    if (!FileReplacer::applyReplacement(rt.ctx)) {
        fmWarning() << "Failed to apply replacement";
        return false;
    }

    cleanupManager.confirmCompleted(rt.actualInfo->uri());
    return true;
}

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
    if (!workData) {
        return currentAction;
    }
    if (workData->errorOfAction.contains(error)) {
        currentAction = workData->errorOfAction.value(error);
        return currentAction;
    }

    if (FileUtils::isSameFile(urlFrom, urlTo, Global::CreateFileInfoType::kCreateFileInfoSync)) {
        currentAction = AbstractJobHandler::SupportAction::kCoexistAction;
        return currentAction;
    }

    fmWarning() << "File operation error - from:" << urlFrom << "to:" << urlTo
                << "error:" << static_cast<int>(error) << "message:" << errorMsg;

    // 发送错误处理 阻塞自己
    emitErrorNotify(urlFrom, urlTo, error, isTo, quintptr(this), errorMsg, errorMsgAll);
    pause();
    {
        QMutexLocker locker(&mutex);
        waitCondition.wait(&mutex);
    }
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
            ? writSize * 1000 / (elTime)
            : 0;
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(jobType));
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobStateKey, QVariant::fromValue(currentState));
    info->insert(AbstractJobHandler::NotifyInfoKey::kSpeedKey, QVariant::fromValue(speed));
    info->insert(AbstractJobHandler::NotifyInfoKey::kRemindTimeKey, QVariant::fromValue(speed == 0 ? -1 : (sourceFilesTotalSize - writSize) / speed));

    emit stateChangedNotify(info);
    emit speedUpdatedNotify(info);
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

    fmWarning() << "File size exceeds FAT32 limit - file:" << fromUrl << "size:" << size << "limit: 4GB";
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
        qint64 freeBytes = DeviceUtils::deviceBytesFree(toUrl);
        fmInfo() << "Disk space check - available:" << freeBytes << "required:" << sourceFilesTotalSize;

        action = AbstractJobHandler::SupportAction::kNoAction;
        if (sourceFilesTotalSize >= freeBytes) {
            fmWarning() << "Insufficient disk space - required:" << sourceFilesTotalSize << "available:" << freeBytes;
            action = doHandleErrorAndWait(fromUrl, toUrl, AbstractJobHandler::JobErrorType::kNotEnoughSpaceError);
        }
    } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());

    checkRetry();

    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        setSkipValue(skip, action);
        return false;
    }

    return true;
}

bool FileOperateBaseWorker::copyFileFromTrash(const QUrl &urlSource, const QUrl &urlTarget, DFile::CopyFlag flag)
{
    auto fileinfo = InfoFactory::create<FileInfo>(urlSource, Global::CreateFileInfoType::kCreateFileInfoSync);
    if (fileinfo->isAttributes(OptInfoType::kIsDir)) {
        if (!DFMIO::DFile(urlTarget).exists()) {
            DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
            if (!fileHandler.mkdir(urlTarget)) {
                fmWarning() << "Failed to create directory when restoring from trash - target:" << urlTarget << "error:" << fileHandler.errorString();
                return false;
            }
        }

        QString error;
        const AbstractDirIteratorPointer &iterator = DirIteratorFactory::create<AbstractDirIterator>(urlSource, &error);
        if (!iterator) {
            fmWarning() << "Failed to create directory iterator when restoring from trash - source:" << urlSource << "error:" << error;
            return false;
        }
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
                if (!succ) {
                    fmDebug() << "Failed to restore directory from trash - from:" << url << "to:" << newTargetInfo->uri();
                    return false;
                }
            } else {
                DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
                bool trashSucc = fileHandler.copyFile(url, newTargetInfo->uri(), flag);
                if (!trashSucc) {
                    fmWarning() << "Failed to restore file from trash - from:" << url << "to:" << newTargetInfo->uri() << "error:" << fileHandler.errorString();
                    return false;
                }
            }
        }
        return true;
    } else {
        DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
        bool trashSucc = fileHandler.copyFile(urlSource, urlTarget, flag);
        if (!trashSucc) {
            fmWarning() << "Failed to restore file from trash - from:" << urlSource << "to:" << urlTarget << "error:" << fileHandler.errorString();
        }
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
    if (!checkDiskSpaceAvailable(fromInfo->uri(), targetOrgUrl, skip)) {
        fmDebug() << "Disk space check failed for cut operation - from:" << fromInfo->uri();
        return false;
    }

    bool ok = false;
    if (!toInfo)
        return false;

    // 追踪目标文件（跨分区剪切时）
    cleanupManager.trackIncompleteFile(toInfo->uri());

    if (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool()) {
        ok = createSystemLink(fromInfo, toInfo, workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyFollowSymlink), true, skip);
        if (ok) {
            workData->zeroOrlinkOrDirWriteSize += FileUtils::getMemoryPageSize();
            if (!skip || !*skip)
                cutAndDeleteFiles.append(fromInfo);
        }
    } else if (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsDir).toBool()) {
        ok = checkAndCopyDir(fromInfo, toInfo, skip);
        if (ok)
            workData->zeroOrlinkOrDirWriteSize += workData->dirSize;
    } else {
        const QUrl &url = toInfo->uri();
        auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();

        // 准备替换目标
        ReplacementTarget rt = prepareReplacementTarget(toInfo, fromInfo, cleanupManager);
        DFileInfoPointer &actualToInfo = rt.actualInfo;

        // Set expected size for target file to ensure correct grouping during cut operation
        setExpectedSizeForTarget(toInfo->uri(), fromSize);

        // check file file size bigger than 4 GB
        if (!checkFileSize(fromSize, fromInfo->uri(), url, skip)) {
            fmDebug() << "File size check failed for cut operation - from:" << fromInfo->uri();
            return ok;
        }

        FileUtils::cacheCopyingFileUrl(url);
        initSignalCopyWorker();
        DoCopyFileWorker::NextDo nextDo { DoCopyFileWorker::NextDo::kDoCopyNext };
        if (fromSize > bigFileSize || !supportDfmioCopy || workData->exBlockSyncEveryWrite) {
            do {
                nextDo = copyOtherFileWorker->doCopyFilePractically(fromInfo, actualToInfo, skip);
            } while (nextDo == DoCopyFileWorker::NextDo::kDoCopyReDoCurrentFile && !isStopped());
            ok = nextDo != DoCopyFileWorker::NextDo::kDoCopyErrorAddCancel;
        } else {
            ok = copyOtherFileWorker->doDfmioFileCopy(fromInfo, actualToInfo, skip);
        }

        // 应用替换（如果有）
        if (ok) {
            if (!applyReplacementIfNeeded(rt, cleanupManager)) {
                ok = false;
            }
        }

        if (ok)
            cutAndDeleteFiles.append(fromInfo);
        FileUtils::removeCopyingFileUrl(url);
    }

    toInfo->initQuerier();
    if (ok && toInfo->exists() && targetInfo == targetPathInfo) {
        // 确认文件已完成（从清理列表移除）
        cleanupManager.confirmCompleted(toInfo->uri());
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
                    ? workData->dirSize
                    : fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();

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

        DFileInfo::AttributeID attributeId = newFromInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool()
                ? DFileInfo::AttributeID::kStandardSymlinkTarget
                : DFileInfo::AttributeID::kStandardFilePath;

        auto target = QUrl::fromLocalFile(newFromInfo->attribute(attributeId).toString());

        if (localFileHandler->createSystemLink(target, toInfo->uri())) {
            return true;
        }
        fmWarning() << "Create symlink failed - target:" << target << "link:" << toInfo->uri() << "error:" << localFileHandler->errorString();
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
    if (!newTargetUrl.isValid())
        return {};

    DFileInfoPointer newTargetInfo { new DFileInfo(newTargetUrl) };
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
                        ? workData->dirSize
                        : fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
            return nullptr;
        }

        if (action != AbstractJobHandler::SupportAction::kEnforceAction) {
            setSkipValue(skip, action);
            if (skip && *skip)
                workData->skipWriteSize += isCountSize
                                && (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool()
                                    || fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong() <= 0)
                        ? workData->dirSize
                        : fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
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
        newTargetInfo->initQuerier();
        if (jobType == AbstractJobHandler::JobType::kCutType && newTargetInfo)
            isCutMerge = true;
        break;
    }
    case AbstractJobHandler::SupportAction::kSkipAction: {
        workData->skipWriteSize += (isCountSize && (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool() || fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong() <= 0))
                ? workData->dirSize
                : fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
        setSkipValue(skip, action);
        return nullptr;
    }
    case AbstractJobHandler::SupportAction::kCoexistAction: {
        const auto ne = InfoFactory::create<FileInfo>(newTargetInfo->uri(), Global::CreateFileInfoType::kCreateFileInfoSync);
        const auto to = InfoFactory::create<FileInfo>(toInfo->uri(), Global::CreateFileInfoType::kCreateFileInfoSync);
        fileNewName = FileNamingUtils::generateNonConflictingName(ne, to);
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

    // 追踪目标文件（文件创建时立即追踪）
    cleanupManager.trackIncompleteFile(toInfo->uri());

    // Set expected size for target file to ensure correct grouping during copy
    setExpectedSizeForTarget(toInfo->uri(), fromSize);

    // check file file size bigger than 4 GB
    if (!checkFileSize(fromSize, fromInfo->uri(),
                       toInfo->uri(), skip)) {
        fmDebug() << "File size check failed, operation skipped - from:" << fromInfo->uri();
        return false;
    }

    if (jobType == AbstractJobHandler::JobType::kCutType)
        return doCopyOtherFile(fromInfo, toInfo, skip);

    // 使用统一的判断接口（替换原来的内联判断）
    if (shouldUseMultiThreadCopy(fromInfo)) {
        while (bigFileCopy && !isStopped()) {
            QThread::msleep(10);
        }
        if (fromSize > bigFileSize && FileUtils::isSameMountPoint(fromInfo->uri(), targetUrl)) {
            bigFileCopy = true;
            auto result = doCopyLocalByRange(fromInfo, toInfo, skip);
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

    // Ensure the caller passes a directory (development-time check)
    Q_ASSERT(fromInfo->attribute(DFileInfo::AttributeID::kStandardIsDir).toBool());

    if (!fromInfo->attribute(DFileInfo::AttributeID::kAccessCanRead).toBool()) {
        fmWarning() << "Cannot copy source directory - permission denied:" << fromInfo->uri();

        auto action = doHandleErrorAndWait(
                fromInfo->uri(),
                toInfo->uri(),
                AbstractJobHandler::JobErrorType::kPermissionError,
                false,
                tr("Cannot copy source directory: permission denied"));

        setSkipValue(skip, action);
        return false;
    }

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
            auto fileUrl = toInfo->uri();
            if (localFileHandler->errorCode() == DFMIOErrorCode::DFM_IO_ERROR_FAILED
                && fileUrl.path().toLocal8Bit().size() > 255
                && ProtocolUtils::isMTPFile(fileUrl))
                errstr = tr("The file name or the path is too long!");

            fmWarning() << "Create directory failed - dir:" << toInfo->uri() << "error:" << errstr;
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
        fmCritical() << "Create directory iterator failed - dir:" << fromInfo->uri() << "error:" << error;
        doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(), AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }

    bool self = true;
    iterator->setProperty("QueryAttributes", "standard::name");
    while (iterator->hasNext()) {
        if (!stateCheck()) {
            return false;
        }

        const QUrl &url = iterator->next();
        DFileInfoPointer info(new DFileInfo(url));
        info->initQuerier();
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
        if (permissions && !ProtocolUtils::isMTPFile(toInfo->uri()))
            localFileHandler->setPermissions(toInfo->uri(), permissions);
    }

    return true;
}

/*!
 * \brief FileOperateBaseWorker::shouldUseMultiThreadCopy 判断是否应该使用多线程本地复制
 *
 * 统一的判断接口，确保多线程复制的条件在所有调用点保持一致。
 *
 * \param fromInfo 源文件信息
 * \return true 表示使用多线程复制，false 表示使用同步复制
 */
bool FileOperateBaseWorker::shouldUseMultiThreadCopy(const DFileInfoPointer &fromInfo) const
{
    // 必须是复制操作（剪切操作在 checkAndCopyFile 中走同步分支）
    if (jobType != AbstractJobHandler::JobType::kCopyType) {
        return false;
    }

    // 必须是本地到本地的复制
    if (!isSourceFileLocal || !isTargetFileLocal) {
        return false;
    }

    // 不能是单线程模式
    if (workData->singleThread) {
        return false;
    }

    // 必须是普通文件（目录和符号链接不使用多线程复制）
    const bool isRegularFile = fromInfo->attribute(DFileInfo::AttributeID::kStandardIsFile).toBool();
    if (!isRegularFile) {
        return false;
    }

    return true;
}

/*!
 * \brief FileOperateBaseWorker::applyAllPendingReplacements 批量应用所有待处理的替换
 *
 * 在多线程复制完成后的等待点调用，将所有临时文件原子地替换到目标位置。
 * 此函数在主线程执行，无需加锁。
 *
 * \return 所有替换都成功返回 true，至少一个失败返回 false
 */
bool FileOperateBaseWorker::applyAllPendingReplacements()
{
    if (pendingReplacements.isEmpty()) {
        return true;
    }

    fmInfo() << "Applying" << pendingReplacements.size() << "pending replacements";

    bool allSuccess = true;
    for (auto &rt : pendingReplacements) {
        if (!rt.isUsingTemporary()) {
            continue;
        }

        if (!FileReplacer::applyReplacement(rt.ctx)) {
            fmCritical() << "Failed to apply replacement for"
                         << rt.ctx.originalTargetPath;
            allSuccess = false;
        } else {
            cleanupManager.confirmCompleted(rt.actualInfo->uri());
        }
    }

    pendingReplacements.clear();
    return allSuccess;
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

    // 等待完成后，批量执行所有延迟的替换操作
    if (!applyAllPendingReplacements()) {
        fmWarning() << "Some pending replacements failed";
    }
}

// waitThreadPoolOver method removed - no longer needed without multi-threading

void FileOperateBaseWorker::initCopyWay()
{
    if (isSourceFileLocal && isTargetFileLocal) {
        countWriteType = CountWriteSizeType::kCustomizeType;
        workData->singleThread = (sourceFilesCount > 1 || sourceFilesTotalSize > FileOperationsUtils::bigFileSize()) && FileUtils::getCpuProcessCount() > 4
                ? false
                : true;
        if (!workData->singleThread)
            threadCount = FileUtils::getCpuProcessCount() < 4 ? 2 : 4;
    }

    if (ProtocolUtils::isSMBFile(targetUrl)
        || ProtocolUtils::isFTPFile(targetUrl)
        || workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCountProgressCustomize)) {
        countWriteType = CountWriteSizeType::kCustomizeType;
    } else if (shouldUseBlockWriteType()) {
        // Use block device write counting for specific scenarios
        countWriteType = CountWriteSizeType::kWriteBlockType;
        fmDebug() << "Using kWriteBlockType for progress counting";
    }

    if (!workData->singleThread) {
        initThreadCopy();
    }

    copyTid = (countWriteType == CountWriteSizeType::kTidType) ? syscall(SYS_gettid) : -1;
}

/*!
 * \brief FileOperateBaseWorker::shouldUseBlockWriteType Determine if should use block write type for progress counting
 * \return true if should use kWriteBlockType, false otherwise
 */
bool FileOperateBaseWorker::shouldUseBlockWriteType() const
{
    // Only consider block write type when copying to removable devices
    if (!targetIsRemovable || !workData->isBlockDevice || !workData->exBlockSyncEveryWrite) {
        return false;
    }

    // Get target filesystem type
    const QString &targetFsType = dfmio::DFMUtils::fsTypeFromUrl(targetOrgUrl);

    // Condition 1: Target filesystem is fuse
    if (targetFsType.toLower().contains("fuse")) {
        fmDebug() << "Using block write type: target filesystem is fuse (" << targetFsType << ")";
        return true;
    }

    // Condition 2: Both source and target files are on devices (not local)
    if (!isSourceFileLocal && !isTargetFileLocal) {
        fmDebug() << "Using block write type: both source and target are on devices";
        return true;
    }

    return false;
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

void FileOperateBaseWorker::setExpectedSizeForTarget(const QUrl &targetUrl, qint64 size)
{
    FileInfoPointer targetFileInfo = InfoFactory::create<FileInfo>(targetUrl, Global::CreateFileInfoType::kCreateFileInfoSyncAndCache);
    if (targetFileInfo) {
        targetFileInfo->setExtendedAttributes(ExtInfoType::kExpectedSize, size);
        fmDebug() << "Set expected size for target file:" << targetUrl << "size:" << size;
    }
}

void FileOperateBaseWorker::initThreadCopy()
{
    for (int i = 0; i < threadCount; i++) {
        QSharedPointer<DoCopyFileWorker> copy(new DoCopyFileWorker(workData));
        // todo init new
        connect(copy.data(), &DoCopyFileWorker::errorNotify, this, &FileOperateBaseWorker::emitErrorNotify, Qt::DirectConnection);
        connect(copy.data(), &DoCopyFileWorker::currentTask, this, &FileOperateBaseWorker::emitCurrentTaskNotify, Qt::DirectConnection);
        connect(copy.data(), &DoCopyFileWorker::retryErrSuccess, this, &FileOperateBaseWorker::retryErrSuccess, Qt::DirectConnection);
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
    if (!toInfo)
        return {};

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

    threadPool->start([this, fromInfo, toInfo]() {
        threadCopyWorker[threadCopyFileCount % threadCount]->doFileCopy(fromInfo, toInfo);
    });

    threadCopyFileCount++;
    return true;
}

bool FileOperateBaseWorker::doCopyLocalByRange(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip)
{
    waitThreadPoolOver();
    initSignalCopyWorker();
    const QString &targetUrl = toInfo->uri().toString();

    FileUtils::cacheCopyingFileUrl(targetUrl);
    DoCopyFileWorker::NextDo nextDo = copyOtherFileWorker->doCopyFileByRange(fromInfo, toInfo, skip);
    FileUtils::removeCopyingFileUrl(targetUrl);

    if (nextDo == DoCopyFileWorker::NextDo::kDoCopyNext) {
        return true;
    } else if (nextDo == DoCopyFileWorker::NextDo::kDoCopyFallback) {
        // For same-device local copies, copy_file_range should work
        // If it returns fallback, this indicates a system issue that should be reported
        fmWarning() << "copy_file_range failed for same-device local copy, system error"
                    << "from:" << fromInfo->uri() << "to:" << toInfo->uri();

        // Show error dialog like in doCopyFileByRange
        // We don't have direct access to errno here, but we know copy_file_range failed
        auto lastError = "copy_file_range system call failed";
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(
                fromInfo->uri(), toInfo->uri(),
                AbstractJobHandler::JobErrorType::kWriteError,
                false,
                lastError);

        if (skip) {
            *skip = (action == AbstractJobHandler::SupportAction::kSkipAction);
        }

        return action == AbstractJobHandler::SupportAction::kNoAction;
    } else {
        // kDoCopyErrorAddCancel or other error cases
        return false;
    }
}

bool FileOperateBaseWorker::doCopyOtherFile(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip)
{
    initSignalCopyWorker();
    const QUrl &targetFileUrl = toInfo->uri();

    FileUtils::cacheCopyingFileUrl(targetFileUrl);

    bool ok = false;
    const auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();

    // Strategy 1: Try copy_file_range first, but only for same device copies
    // copy_file_range only works within the same filesystem (e.g., U盘 to U盘)
    bool isSameDevice = FileUtils::isSameMountPoint(fromInfo->uri(), this->targetUrl);
    if (isSameDevice) {
        DoCopyFileWorker::NextDo nextDo = copyOtherFileWorker->doCopyFileByRange(fromInfo, toInfo, skip);
        if (nextDo == DoCopyFileWorker::NextDo::kDoCopyNext) {
            ok = true;
        } else if (nextDo == DoCopyFileWorker::NextDo::kDoCopyErrorAddCancel) {
            FileUtils::removeCopyingFileUrl(targetFileUrl);
            return false;
        } else if (nextDo == DoCopyFileWorker::NextDo::kDoCopyFallback) {
            fmDebug() << "copy_file_range fallback needed for same device, trying other methods";

            // Clean up any partially created target file before fallback
            // copy_file_range may have created an empty file that needs cleanup
            QString targetPath = toInfo->uri().path();
            if (QFile::exists(targetPath)) {
                if (QFile::remove(targetPath)) {
                    fmDebug() << "Successfully cleaned up partially created target file:" << targetPath;
                } else {
                    fmWarning() << "Failed to cleanup partially created target file:" << targetPath;
                }
            }
            // Continue to fallback methods
        }
    } else {
        fmDebug() << "Cross-device copy detected, skipping copy_file_range";
    }
    // If copy_file_range failed but not cancelled, fallback to other methods

    // Strategy 2: Use doCopyFilePractically for large files, sync mode, or unsupported dfmio
    if (!ok && (fromSize > bigFileSize || !supportDfmioCopy || workData->exBlockSyncEveryWrite)) {
        DoCopyFileWorker::NextDo nextDo;
        do {
            nextDo = copyOtherFileWorker->doCopyFilePractically(fromInfo, toInfo, skip);
        } while (nextDo == DoCopyFileWorker::NextDo::kDoCopyReDoCurrentFile && !isStopped());
        ok = nextDo != DoCopyFileWorker::NextDo::kDoCopyErrorAddCancel;
    }

    // Strategy 3: Fallback to dfmio copy for small files
    if (!ok && !workData->exBlockSyncEveryWrite) {
        ok = copyOtherFileWorker->doDfmioFileCopy(fromInfo, toInfo, skip);
    }

    FileUtils::removeCopyingFileUrl(targetFileUrl);

    return ok;
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
        workData->skipWriteSize += (isCountSize && (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool() || fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong() <= 0))
                ? workData->dirSize
                : fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
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

    // 使用统一的判断接口确定是否使用多线程复制
    const bool useMultiThreadCopy = shouldUseMultiThreadCopy(fromInfo);

    // 准备替换目标（仅普通文件使用临时文件）
    ReplacementTarget rt = prepareReplacementTarget(newTargetInfo, fromInfo, cleanupManager);
    DFileInfoPointer &actualTargetInfo = rt.actualInfo;

    // 执行复制/链接/目录操作，使用actualTargetInfo
    if (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool()) {
        result = createSystemLink(fromInfo, newTargetInfo,
                                  workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kCopyFollowSymlink),
                                  true, skip);
        if (result) {
            workData->zeroOrlinkOrDirWriteSize +=
                    (newTargetInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong() > 0
                             ? newTargetInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong()
                             : FileUtils::getMemoryPageSize());
        }
    } else if (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsDir).toBool()) {
        result = checkAndCopyDir(fromInfo, newTargetInfo, skip);
        if (result || skip) {
            workData->zeroOrlinkOrDirWriteSize += workData->dirSize <= 0 ? FileUtils::getMemoryPageSize() : workData->dirSize;
        }
    } else {
        result = checkAndCopyFile(fromInfo, actualTargetInfo, skip);
    }

    // 根据场景选择立即替换或延迟替换
    if (result) {
        if (useMultiThreadCopy && rt.isUsingTemporary()) {
            // 多线程场景：延迟替换，收集上下文
            pendingReplacements.append(rt);
            fmDebug() << "Deferred replacement for multi-thread copy:"
                      << rt.ctx.originalTargetPath;
        } else {
            // 单线程场景：立即替换
            if (!applyReplacementIfNeeded(rt, cleanupManager)) {
                result = false;
            }
        }
    }

    if (result) {
        emit fileAdded(newTargetInfo->uri());
        // 仅在非延迟替换场景确认完成（延迟替换在批量替换时确认）
        if (!useMultiThreadCopy || !rt.isUsingTemporary()) {
            cleanupManager.confirmCompleted(newTargetInfo->uri());
        }
    }

    if (targetInfo == toInfo) {
        completeSourceFiles.append(fromInfo->uri());
        precompleteTargetFileInfo.append(newTargetInfo);
    }

    return result;
}

void FileOperateBaseWorker::setAllDirPermisson()
{
    for (auto info : dirPermissonList.listByLock()) {
        if (info->permission && supportSetPermission)
            localFileHandler->setPermissions(info->target, info->permission);
    }
}

qint64 FileOperateBaseWorker::getWriteDataSize()
{
    qint64 writeSize = 0;
    if (!workData)
        return writeSize;

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
            writeSize = (currentSectorsWritten - targetDeviceStartSectorsWritten) * targetLogicSectorSize;
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
    // Check target file validity and determine write progress counting method:
    // - Use block device sector counting for removable devices
    // - Use thread-based byte counting for local disks
    auto rootPath = DFMUtils::mountPathFromUrl(targetOrgUrl);
    auto device = DFMUtils::deviceNameFromUrl(targetOrgUrl);

    if (!device.startsWith("/dev/"))
        return;

    isTargetFileLocal = FileOperationsUtils::isFileOnDisk(targetOrgUrl);
    workData->isTargetFileLocal = isTargetFileLocal;

    fmDebug("Target block device: \"%s\", Root Path: \"%s\"", device.toStdString().data(), qPrintable(rootPath));

    if (isTargetFileLocal)
        return;

    // Use DeviceProxyManager to check if target is on external removable device
    // This handles encrypted devices (LUKS/dm-crypt) correctly by checking backing device
    targetIsRemovable = DevProxyMng->isFileOfExternalBlockMounts(rootPath);

    fmDebug("targetIsRemovable = %d (via isFileOfExternalBlockMounts)", bool(targetIsRemovable));

    if (!targetIsRemovable)
        return;

    // Get device info to determine physical device for sector size query
    auto devInfo = DevProxyMng->queryDeviceInfoByPath(rootPath, false);
    QString physicalDevice = devInfo.value(GlobalServerDefines::DeviceProperty::kDevice).toString();

    // For encrypted devices, get the underlying physical device path
    QString cryptoBacking = devInfo.value(GlobalServerDefines::DeviceProperty::kCryptoBackingDevice).toString();
    if (!cryptoBacking.isEmpty() && cryptoBacking != "/") {
        auto backingInfo = DevProxyMng->queryBlockInfo(cryptoBacking);
        if (!backingInfo.isEmpty()) {
            physicalDevice = backingInfo.value(GlobalServerDefines::DeviceProperty::kDevice).toString();
            fmDebug("Encrypted device detected, using backing device: \"%s\"", qPrintable(physicalDevice));
        }
    }

    // Get sector size and sys dev path from physical device using system calls
    if (physicalDevice.isEmpty() || !physicalDevice.startsWith("/dev/"))
        return;

    // Get MAJ:MIN via stat() for targetSysDevPath
    struct stat st;
    if (stat(physicalDevice.toLocal8Bit().constData(), &st) != 0 || !S_ISBLK(st.st_mode)) {
        fmWarning("Failed to stat device: \"%s\"", qPrintable(physicalDevice));
        return;
    }
    targetSysDevPath = QString("/sys/dev/block/%1:%2").arg(major(st.st_rdev)).arg(minor(st.st_rdev));

    // Get logical sector size from sysfs (no root permission required)
    // For partitions, read from parent disk's queue directory
    QFile sectorFile(targetSysDevPath + "/../queue/logical_block_size");
    if (!sectorFile.exists()) {
        // Maybe it's a whole disk device, try direct path
        sectorFile.setFileName(targetSysDevPath + "/queue/logical_block_size");
    }
    if (sectorFile.open(QIODevice::ReadOnly)) {
        bool ok = false;
        int sectorSize = sectorFile.readAll().trimmed().toInt(&ok);
        targetLogicSectorSize = ok ? static_cast<qint16>(sectorSize) : 512;
        sectorFile.close();
    } else {
        targetLogicSectorSize = 512;
        fmWarning("Failed to read sector size from sysfs, using default 512");
    }

    workData->exBlockSyncEveryWrite = FileOperationsUtils::blockSync();
    targetDeviceStartSectorsWritten = getSectorsWritten();
    workData->isBlockDevice = true;

    fmDebug("Physical device: \"%s\", Sys dev path: \"%s\", Log-Sec: %d",
            qPrintable(physicalDevice), qPrintable(targetSysDevPath), targetLogicSectorSize);
}

/*!
 * \brief FileOperateBaseWorker::needsSync Check if sync is needed before stopping
 * \return true if sync is needed, false otherwise
 */
bool FileOperateBaseWorker::needsSync() const
{
    // Need sync if:
    // 1. Target is external device (not local)
    // 2. Target URL is valid (we can use it for sync)
    // Note: exBlockSyncEveryWrite condition is now handled in syncFilesToDevice()

    if (!copyOtherFileWorker) {
        return false;
    }

    if (jobType != AbstractJobHandler::JobType::kCopyType
        && jobType != AbstractJobHandler::JobType::kCutType) {
        return false;
    }

    if (workData && !workData->isBlockDevice) {
        return false;
    }

    return !isTargetFileLocal && targetUrl.isValid();
}

/*!
 * \brief FileOperateBaseWorker::performSync Perform synchronization before stopping
 */
void FileOperateBaseWorker::performSync()
{
    fmInfo() << "Performing sync for external device - target:" << targetUrl;
    // Directly sync the target filesystem using targetUrl
    std::string stdStr = targetUrl.path().toUtf8().toStdString();
    int tofd = open(stdStr.data(), O_RDONLY);
    if (-1 != tofd) {
        syncfs(tofd);   // Sync the entire filesystem
        close(tofd);
        fmInfo() << "Sync completed successfully";
    } else {
        fmWarning() << "Failed to open target path for sync:" << targetUrl.path();
    }
}

/*!
 * \brief FileOperateBaseWorker::performAsyncSync Perform non-blocking sync using D-Bus interface
 */
void FileOperateBaseWorker::performAsyncSync()
{
    fmInfo() << "Performing non-blocking sync for external device - target:" << targetUrl;

    // Create typed D-Bus interface to daemon sync service
    OrgDeepinFilemanagerDaemonSyncInterface syncInterface(
            "org.deepin.Filemanager.Daemon",
            "/org/deepin/Filemanager/Daemon/Sync",
            QDBusConnection::sessionBus(),
            this);

    syncInterface.setTimeout(2000);

    if (!syncInterface.isValid()) {
        fmWarning() << "Failed to create sync D-Bus interface, fallback to blocking sync";
        return;
    }

    // Call async sync method using generated interface
    syncInterface.SyncFS(targetUrl.path());

    // Don't wait for completion - this is non-blocking
    // The daemon will handle the sync asynchronously
    fmInfo() << "Non-blocking sync request sent to daemon";
}
