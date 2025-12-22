// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "docutfilesworker.h"
#include "fileoperations/fileoperationutils/fileoperationsutils.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-io/dfmio_utils.h>

#include <QUrl>
#include <QProcess>
#include <QMutex>
#include <QStorageInfo>
#include <QQueue>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <syscall.h>

DPFILEOPERATIONS_USE_NAMESPACE
DoCutFilesWorker::DoCutFilesWorker(QObject *parent)
    : FileOperateBaseWorker(parent)
{
    jobType = AbstractJobHandler::JobType::kCutType;
}

DoCutFilesWorker::~DoCutFilesWorker()
{
    stop();
}

bool DoCutFilesWorker::doWork()
{
    // The endcopy interface function has been called here
    if (!AbstractWorker::doWork())
        return false;

    // check progress notify type
    determineCountProcessType();

    // 执行剪切
    fmInfo() << "Start cutting files - count:" << sourceUrls.count();
    if (!cutFiles()) {
        endWork();
        return false;
    }

    // 完成
    fmInfo() << "Cut operation completed successfully";
    endWork();

    return true;
}

void DoCutFilesWorker::stop()
{
    AbstractWorker::stop();
}

bool DoCutFilesWorker::initArgs()
{

    AbstractWorker::initArgs();

    if (sourceUrls.count() <= 0) {
        fmCritical() << "Cut operation failed: source URLs list is empty";
        // pause and emit error msg
        doHandleErrorAndWait(QUrl(), QUrl(), AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }
    if (!targetUrl.isValid()) {
        fmCritical() << "Cut operation failed: target URL is invalid - target:" << targetUrl;
        // pause and emit error msg
        doHandleErrorAndWait(sourceUrls.first(), targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }
    targetInfo.reset(new DFileInfo(targetUrl));
    targetInfo->initQuerier();
    if (!targetInfo->exists()) {
        fmWarning() << "Cut operation failed: target directory does not exist - target:" << targetUrl;
        // pause and emit error msg
        doHandleErrorAndWait(sourceUrls.first(), targetUrl, AbstractJobHandler::JobErrorType::kNonexistenceError, true);
        return false;
    }

    targetOrgUrl = targetUrl;
    if (targetInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool())
        targetOrgUrl = QUrl::fromLocalFile(targetInfo->attribute(DFileInfo::AttributeID::kStandardSymlinkTarget).toString());

    return true;
}

bool DoCutFilesWorker::cutFiles()
{
    for (const auto &url : sourceUrls) {
        if (!stateCheck()) {
            return false;
        }

        DFileInfoPointer fileInfo(new DFileInfo(url));
        fileInfo->initQuerier();
        // check self
        if (checkSelf(fileInfo))
            continue;

        // check hierarchy
        if (fileInfo->attribute(DFileInfo::AttributeID::kStandardIsDir).toBool()) {
            const bool higher = FileUtils::isHigherHierarchy(url, targetUrl) || url == targetUrl;
            if (higher) {
                fmWarning() << "Cannot move directory to itself or parent - from:" << url << "to:" << targetUrl;
                emit requestShowTipsDialog(DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType::kCopyMoveToSelf, {});
                return false;
            }
        }

        // check link
        if (fileInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool()) {
            const bool ok = checkSymLink(fileInfo);
            if (ok)
                continue;
            else
                return false;
        }
        bool skip = false;
        if (!doCutFile(fileInfo, targetInfo, &skip) && !skip) {
            return false;
        }
    }
    return true;
}

bool DoCutFilesWorker::doCutFile(const DFileInfoPointer &fromInfo, const DFileInfoPointer &targetPathInfo, bool *skip)
{
    QUrl trashInfoUrl;
    QString fileName = fromInfo->attribute(DFileInfo::AttributeID::kStandardFileName).toString();
    const bool isTrashFile = FileUtils::isTrashFile(fromInfo->uri());
    if (isTrashFile) {
        trashInfoUrl = trashInfo(fromInfo);
        fileName = fileOriginName(trashInfoUrl);
    }
    DFileInfoPointer toInfo = nullptr;
    bool success = false;

    // 检查是否再同一个挂载点下，不在就执行copyAndDeleteFile
    const bool isSameDevice = FileUtils::isSameMountPoint(fromInfo->uri(), targetOrgUrl);
    if (isSameDevice) {
        // Same device: try to rename directly. This is the fast path for moving files.
        bool renameOk = false;
        toInfo = trySameDeviceRename(fromInfo, targetPathInfo, fileName, &renameOk, skip);
        success = renameOk;
        if (!success) {
            fmWarning() << "Same-device rename failed - from:" << fromInfo->uri();
            // If rename fails on the same device, it's a genuine error. We should not fall back to copy-delete.
            // Return false unless the operation was skipped by user interaction.
            return skip && *skip;
        }
        // For same-device rename, we update progress based on file/dir size.
        const auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
        workData->currentWriteSize += fromSize;
        if (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsFile).toBool()) {
            workData->blockRenameWriteSize += fromSize;
            if (fromSize <= 0)
                workData->zeroOrlinkOrDirWriteSize += FileUtils::getMemoryPageSize();
        } else {   // Directory
            SizeInfoPointer sizeInfo(new FileUtils::FilesSizeInfo);
            FileOperationsUtils::statisticFilesSize(fromInfo->uri(), sizeInfo);
            workData->blockRenameWriteSize += sizeInfo->totalSize;
            if (sizeInfo->totalSize <= 0)
                workData->zeroOrlinkOrDirWriteSize += workData->dirSize;
        }
    } else {
        // Cross-device: fall back to copy-then-delete.
        fmInfo() << "Cross-device move detected, using copy-delete fallback - from:" << fromInfo->uri() << "to:" << targetPathInfo->uri();
        toInfo = doCheckFile(fromInfo, targetPathInfo, fileName, skip);
        if (toInfo.isNull()) {
            fmDebug() << "File check failed for cross-device move - from:" << fromInfo->uri();
            return skip && *skip;
        }
        success = copyAndDeleteFile(fromInfo, targetPathInfo, toInfo, skip);
        if (success) {
            const auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
            workData->currentWriteSize += fromSize;
        } else {
            fmDebug() << "Copy-delete failed for cross-device move - from:" << fromInfo->uri() << "to:" << toInfo->uri();
        }
    }

    if (!success) {
        if (stopWork.load())
            stopWork.store(false);
        return false;
    }

    if (stopWork.load()) {
        stopWork.store(false);
        return false;
    }

    if (skip && *skip)
        return false;

    if (toInfo.isNull()) {
        fmWarning() << "Operation failed: cannot create target file info";
        return false;
    }

    QUrl orignalUrl = fromInfo->uri();
    if (isTrashFile) {
        removeTrashInfo(trashInfoUrl);
        orignalUrl.setScheme("trash");
        orignalUrl.setPath("/" + orignalUrl.path().replace("/", "\\"));
        auto tmpFileName = fromInfo->uri().fileName();
        auto orignalName = QUrl::toPercentEncoding(tmpFileName);
        orignalUrl.setPath(orignalUrl.path().replace(tmpFileName, orignalName));
    }
    emit fileRenamed(orignalUrl, toInfo->uri());
    return true;
}

void DoCutFilesWorker::onUpdateProgress()
{
    const qint64 writSize = getWriteDataSize();
    emitProgressChangedNotify(writSize);
    emitSpeedUpdatedNotify(writSize);
}

void DoCutFilesWorker::endWork()
{
    // delete all cut source files
    if (localFileHandler) {
        for (const auto &info : cutAndDeleteFiles) {
            const auto &uri = info->uri();
            bool ret = localFileHandler->deleteFile(uri);
            if (!ret) {
                fmWarning() << "Failed to delete source file after cut - file:" << uri << "error:" << localFileHandler->errorString();
                continue;
            }
            FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType::kFileDeleted, uri);
        }
    }

    return FileOperateBaseWorker::endWork();
}

void DoCutFilesWorker::emitCompleteFilesUpdatedNotify(const qint64 &writCount)
{
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kCompleteFilesKey, QVariant::fromValue(writCount));

    emit stateChangedNotify(info);
}

bool DoCutFilesWorker::doMergDir(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo, bool *skip)
{
    // 遍历源文件，执行一个一个的拷贝
    QString error;
    const AbstractDirIteratorPointer &iterator = DirIteratorFactory::create<AbstractDirIterator>(fromInfo->uri(), &error);
    if (!iterator) {
        fmCritical() << "Create directory iterator failed - dir:" << fromInfo->uri() << "error:" << error;
        doHandleErrorAndWait(fromInfo->uri(), toInfo->uri(), AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }

    iterator->setProperty("QueryAttributes", "standard::name");
    while (iterator->hasNext()) {
        if (!stateCheck()) {
            return false;
        }

        const QUrl &url = iterator->next();
        DFileInfoPointer info(new DFileInfo(url));
        info->initQuerier();
        bool skip = false;
        bool ok = doCutFile(info, toInfo, &skip);
        if (!ok && !skip) {
            return false;
        }

        if (!ok)
            continue;
    }

    cutAndDeleteFiles.append(fromInfo);

    return true;
}

bool DoCutFilesWorker::checkSymLink(const DFileInfoPointer &fileInfo)
{
    const QUrl &sourceUrl = fileInfo->uri();
    bool skip = false;
    DFileInfoPointer newTargetInfo = doCheckFile(fileInfo, targetInfo,
                                                 fileInfo->attribute(DFileInfo::AttributeID::kStandardFileName).toString(), &skip);
    if (newTargetInfo.isNull())
        return skip;

    bool ok = createSystemLink(fileInfo, newTargetInfo, true, false, &skip);
    if (!ok && !skip)
        return false;

    if (ok && !skip)
        cutAndDeleteFiles.append(fileInfo);

    completeSourceFiles.append(sourceUrl);
    completeTargetFiles.append(newTargetInfo->uri());

    return true;
}

bool DoCutFilesWorker::checkSelf(const DFileInfoPointer &fileInfo)
{
    const QString &fileName = fileInfo->attribute(DFileInfo::AttributeID::kStandardFileName).toString();
    QString newFileUrl = targetInfo->uri().toString();
    if (!newFileUrl.endsWith("/"))
        newFileUrl.append("/");
    newFileUrl.append(fileName);
    DFMIO::DFileInfo newFileInfo(QUrl(newFileUrl, QUrl::TolerantMode));

    if (newFileInfo.uri() == fileInfo->uri()
        || (FileUtils::isSameFile(fileInfo->uri(), newFileInfo.uri(), Global::CreateFileInfoType::kCreateFileInfoSync)
            && !fileInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool())) {
        return true;
    }
    return false;
}

bool DoCutFilesWorker::renameFileByHandler(const DFileInfoPointer &sourceInfo, const DFileInfoPointer &targetInfo, bool *skip)
{
    if (localFileHandler) {
        const QUrl &sourceUrl = sourceInfo->uri();
        const QUrl &targetUrl = targetInfo->uri();
        AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

        do {
            action = AbstractJobHandler::SupportAction::kNoAction;
            if (!localFileHandler->renameFile(sourceUrl, targetUrl, false)) {
                auto err = AbstractJobHandler::JobErrorType::kPermissionError;
                if (localFileHandler->errorCode() != DFMIOErrorCode::DFM_IO_ERROR_PERMISSION_DENIED) {
                    err = AbstractJobHandler::JobErrorType::kUnknowError;
                }
                fmWarning() << "Rename file failed - from:" << sourceUrl << "to:" << targetUrl
                            << "error:" << localFileHandler->errorString()
                            << "code:" << localFileHandler->errorCode();
                action = doHandleErrorAndWait(sourceUrl, targetUrl, err, false, localFileHandler->errorString());
            }
        } while (action == AbstractJobHandler::SupportAction::kRetryAction && !isStopped());

        checkRetry();

        if (action != AbstractJobHandler::SupportAction::kNoAction) {
            setSkipValue(skip, action);
            return false;
        }
    }
    return true;
}

DFileInfoPointer DoCutFilesWorker::trySameDeviceRename(const DFileInfoPointer &sourceInfo,
                                                       const DFileInfoPointer &targetPathInfo,
                                                       const QString fileName, bool *ok, bool *skip)
{
    // This function assumes the source and target are on the same device.
    // It handles name collision checks and performs the rename operation.
    const QUrl &sourceUrl = sourceInfo->uri();
    fmDebug() << "Attempting same-device rename - from:" << sourceUrl << "to:" << targetPathInfo->uri();
    auto newTargetInfo = doCheckFile(sourceInfo, targetPathInfo, fileName, skip);
    if (newTargetInfo.isNull()) {
        fmDebug() << "File check failed during same-device rename - from:" << sourceUrl;
        return nullptr;
    }

    emitCurrentTaskNotify(sourceUrl, newTargetInfo->uri());
    bool result = false;
    if (isCutMerge) {
        newTargetInfo->initQuerier();
        isCutMerge = false;
        result = doMergDir(sourceInfo, newTargetInfo, skip);
    } else {
        if (newTargetInfo->exists()) {
            result = deleteFile(newTargetInfo->uri(), newTargetInfo->uri(), skip);
            if (result) {
                *skip = false;   // deleteFile拷贝成功会设置skip为true，正确的删除后设置skip为false
                result = renameFileByHandler(sourceInfo, newTargetInfo, skip);
            } else {
                fmDebug() << "Failed to delete existing target file during rename - file:" << newTargetInfo->uri();
            }
        } else {
            result = renameFileByHandler(sourceInfo, newTargetInfo, skip);
        }
    }

    if (result) {
        if (targetPathInfo == this->targetInfo) {
            completeSourceFiles.append(sourceUrl);
            completeTargetFiles.append(newTargetInfo->uri());
        }
    }
    if (ok)
        *ok = result;
    return newTargetInfo;
}
