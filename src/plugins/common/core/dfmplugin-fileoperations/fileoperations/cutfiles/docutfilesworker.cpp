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
#include <QDebug>

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
    if (!cutFiles()) {
        endWork();
        return false;
    }

    // sync
    syncFilesToDevice();

    // 完成
    endWork();

    return true;
}

void DoCutFilesWorker::stop()
{
    AbstractWorker::stop();
}

bool DoCutFilesWorker::initArgs()
{
    if (!speedtimer) {
        speedtimer = new QElapsedTimer();
        speedtimer->start();
    }

    AbstractWorker::initArgs();

    if (sourceUrls.count() <= 0) {
        // pause and emit error msg
        doHandleErrorAndWait(QUrl(), QUrl(), AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }
    if (!targetUrl.isValid()) {
        // pause and emit error msg
        doHandleErrorAndWait(sourceUrls.first(), targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }
    targetInfo.reset(new DFileInfo(targetUrl));
    targetInfo->initQuerier();
    if (!targetInfo->exists()) {
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

        // check self
        if (checkSelf(fileInfo))
            continue;

        // check hierarchy
        if (fileInfo->attribute(DFileInfo::AttributeID::kStandardIsDir).toBool()) {
            const bool higher = FileUtils::isHigherHierarchy(url, targetUrl) || url == targetUrl;
            if (higher) {
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

        if (!doCutFile(fileInfo, targetInfo)) {
            return false;
        }
    }
    return true;
}

bool DoCutFilesWorker::doCutFile(const DFileInfoPointer &fromInfo, const DFileInfoPointer &targetPathInfo)
{
    // try rename
    bool ok = false;
    // 获取trashinfourl
    QUrl trashInfoUrl;
    QString fileName = fromInfo->attribute(DFileInfo::AttributeID::kStandardFileName).toString();
    bool isTrashFile = FileUtils::isTrashFile(fromInfo->uri());
    if (isTrashFile) {
        trashInfoUrl= trashInfo(fromInfo);
        fileName = fileOriginName(trashInfoUrl);
    }
    DFileInfoPointer toInfo = doRenameFile(fromInfo, targetPathInfo, fileName, &ok);
    auto fromSize = fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong();
    if (ok) {
        workData->currentWriteSize += fromSize;
        if (fromInfo->attribute(DFileInfo::AttributeID::kStandardIsFile).toBool()) {
            workData->blockRenameWriteSize += fromSize;
            workData->currentWriteSize += (fromSize > 0
                                           ? fromSize : FileUtils::getMemoryPageSize());
            if (fromSize <= 0)
                workData->zeroOrlinkOrDirWriteSize += FileUtils::getMemoryPageSize();
        } else {
            // count size
            SizeInfoPointer sizeInfo(new FileUtils::FilesSizeInfo);
            FileOperationsUtils::statisticFilesSize(fromInfo->uri(), sizeInfo);
            workData->blockRenameWriteSize += sizeInfo->totalSize;
            if (sizeInfo->totalSize <= 0)
                workData->zeroOrlinkOrDirWriteSize += workData->dirSize;
        }
        if (isTrashFile)
            removeTrashInfo(trashInfoUrl);
        return true;
    }

    if (stopWork.load()) {
        stopWork.store(false);
        return false;
    }

    if (toInfo.isNull()) {
        fmWarning() << " do rename failed ! create null target Info";
        return false;
    }

    fmDebug() << "do rename failed, use copy and delete way, from url: " << fromInfo->uri() << " to url: "
              << targetPathInfo->uri();
    bool result = false;
    if (!copyAndDeleteFile(fromInfo, targetPathInfo, toInfo, &result))
        return result;

    workData->currentWriteSize += fromSize;
    if (isTrashFile)
        removeTrashInfo(trashInfoUrl);
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
    bool skip{false};
    for (const auto &info : cutAndDeleteFiles) {
        if (!deleteFile(info->uri(), targetOrgUrl, &skip)) {
            fmWarning() << "delete file error, so do not delete other files!!!!";
            break;
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
    ok = deleteFile(sourceUrl, QUrl(), &skip);
    if (!ok && !skip)
        return false;

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

bool DoCutFilesWorker::renameFileByHandler(const DFileInfoPointer &sourceInfo, const DFileInfoPointer &targetInfo)
{
    if (localFileHandler) {
        const QUrl &sourceUrl = sourceInfo->uri();
        const QUrl &targetUrl = targetInfo->uri();
        return localFileHandler->renameFile(sourceUrl, targetUrl);
    }
    return false;
}

DFileInfoPointer DoCutFilesWorker::doRenameFile(const DFileInfoPointer &sourceInfo,
                                                const DFileInfoPointer &targetPathInfo,
                                                const QString fileName, bool *ok)
{
    const QUrl &sourceUrl = sourceInfo->uri();
    if (DFMIO::DFMUtils::deviceNameFromUrl(sourceUrl) == DFMIO::DFMUtils::deviceNameFromUrl(targetOrgUrl)) {
        auto newTargetInfo = doCheckFile(sourceInfo, targetPathInfo, fileName, ok);
        if (newTargetInfo.isNull())
            return nullptr;

        emitCurrentTaskNotify(sourceUrl, newTargetInfo->uri());
        bool result = renameFileByHandler(sourceInfo, newTargetInfo);
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

    auto newTargetInfo = doCheckFile(sourceInfo, targetPathInfo, fileName, ok);
    return newTargetInfo;
}
