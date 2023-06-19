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
    time.start();

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
    targetInfo = InfoFactory::create<FileInfo>(targetUrl, Global::CreateFileInfoType::kCreateFileInfoSync);
    if (!targetInfo) {
        // pause and emit error msg
        doHandleErrorAndWait(sourceUrls.first(), targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }

    if (!targetInfo->exists()) {
        // pause and emit error msg
        doHandleErrorAndWait(sourceUrls.first(), targetUrl, AbstractJobHandler::JobErrorType::kNonexistenceError, true);
        return false;
    }

    targetOrgUrl = targetUrl;
    if (targetInfo->isAttributes(OptInfoType::kIsSymLink))
        targetOrgUrl = QUrl::fromLocalFile(targetInfo->pathOf(PathInfoType::kSymLinkTarget));

    return true;
}

bool DoCutFilesWorker::cutFiles()
{
    for (const auto &url : sourceUrls) {
        if (!stateCheck()) {
            return false;
        }

        const auto &fileInfo = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
        if (!fileInfo) {
            // pause and emit error msg
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(url, targetUrl, AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                continue;
            }
        }

        // check self
        if (checkSelf(fileInfo))
            continue;

        // check hierarchy
        if (fileInfo->isAttributes(OptInfoType::kIsDir)) {
            const bool higher = FileUtils::isHigherHierarchy(url, targetUrl) || url == targetUrl;
            if (higher) {
                emit requestShowTipsDialog(DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType::kCopyMoveToSelf, {});
                return false;
            }
        }

        // check link
        if (fileInfo->isAttributes(OptInfoType::kIsSymLink)) {
            const bool ok = checkSymLink(fileInfo);
            if (ok)
                continue;
            else
                return false;
        }

        if (!doCutFile(fileInfo, targetInfo)) {
            return false;
        }
        fileInfo->refresh();
    }
    return true;
}

bool DoCutFilesWorker::doCutFile(const FileInfoPointer &fromInfo, const FileInfoPointer &targetPathInfo)
{
    // try rename
    bool ok = false;
    FileInfoPointer toInfo = nullptr;
    // 获取trashinfourl
    QUrl trashInfoUrl;
    QString fileName = fromInfo->nameOf(NameInfoType::kFileCopyName);
    bool isTrashFile = FileUtils::isTrashFile(fromInfo->urlOf(UrlInfoType::kUrl));
    if (isTrashFile) {
        trashInfoUrl= trashInfo(fromInfo);
        fileName = fileOriginName(trashInfoUrl);
    }
    if (doRenameFile(fromInfo, targetPathInfo, toInfo, fileName, &ok) || ok) {
        workData->currentWriteSize += fromInfo->size();
        if (fromInfo->isAttributes(OptInfoType::kIsFile)) {
            workData->blockRenameWriteSize += fromInfo->size();
            workData->currentWriteSize += (fromInfo->size() > 0 ? fromInfo->size() : FileUtils::getMemoryPageSize());
            if (fromInfo->size() <= 0)
                workData->zeroOrlinkOrDirWriteSize += FileUtils::getMemoryPageSize();
        } else {
            // count size
            SizeInfoPointer sizeInfo(new FileUtils::FilesSizeInfo);
            FileOperationsUtils::statisticFilesSize(fromInfo->urlOf(UrlInfoType::kUrl), sizeInfo);
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

    qDebug() << "do rename failed, use copy and delete way, from url: " << fromInfo->urlOf(UrlInfoType::kUrl) << " to url: " << targetPathInfo->urlOf(UrlInfoType::kUrl);

    bool result = false;
    if (!copyAndDeleteFile(fromInfo, targetPathInfo, toInfo, &result))
        return result;

    workData->currentWriteSize += fromInfo->size();
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

void DoCutFilesWorker::emitCompleteFilesUpdatedNotify(const qint64 &writCount)
{
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kCompleteFilesKey, QVariant::fromValue(writCount));

    emit stateChangedNotify(info);
}

bool DoCutFilesWorker::checkSymLink(const FileInfoPointer &fileInfo)
{
    const QUrl &sourceUrl = fileInfo->urlOf(UrlInfoType::kUrl);
    FileInfoPointer newTargetInfo(nullptr);
    bool result = false;
    bool ok = doCheckFile(fileInfo, targetInfo, fileInfo->nameOf(NameInfoType::kFileCopyName),
                          newTargetInfo, &result);
    if (!ok && !result)
        return false;
    ok = createSystemLink(fileInfo, newTargetInfo, true, false, &result);
    if (!ok && !result)
        return false;
    ok = deleteFile(sourceUrl, QUrl(), &result);
    if (!ok && !result)
        return false;

    completeSourceFiles.append(sourceUrl);
    completeTargetFiles.append(newTargetInfo->urlOf(UrlInfoType::kUrl));

    return true;
}

bool DoCutFilesWorker::checkSelf(const FileInfoPointer &fileInfo)
{
    const QString &fileName = fileInfo->nameOf(NameInfoType::kFileName);
    QString newFileUrl = targetInfo->urlOf(UrlInfoType::kUrl).toString();
    if (!newFileUrl.endsWith("/"))
        newFileUrl.append("/");
    newFileUrl.append(fileName);
    DFMIO::DFileInfo newFileInfo(QUrl(newFileUrl, QUrl::TolerantMode));

    if (newFileInfo.uri() == fileInfo->urlOf(UrlInfoType::kUrl)
        || (FileUtils::isSameFile(fileInfo->urlOf(UrlInfoType::kUrl), newFileInfo.uri(), Global::CreateFileInfoType::kCreateFileInfoSync) && !fileInfo->isAttributes(OptInfoType::kIsSymLink))) {
        return true;
    }
    return false;
}

bool DoCutFilesWorker::renameFileByHandler(const FileInfoPointer &sourceInfo, const FileInfoPointer &targetInfo)
{
    if (localFileHandler) {
        const QUrl &sourceUrl = sourceInfo->urlOf(UrlInfoType::kUrl);
        const QUrl &targetUrl = targetInfo->urlOf(UrlInfoType::kUrl);
        return localFileHandler->renameFile(sourceUrl, targetUrl);
    }
    return false;
}

bool DoCutFilesWorker::doRenameFile(const FileInfoPointer &sourceInfo, const FileInfoPointer &targetPathInfo, FileInfoPointer &toInfo, const QString fileName, bool *ok)
{
    const QUrl &sourceUrl = sourceInfo->urlOf(UrlInfoType::kUrl);

    toInfo.reset();
    if (DFMIO::DFMUtils::deviceNameFromUrl(sourceInfo->urlOf(UrlInfoType::kUrl)) == DFMIO::DFMUtils::deviceNameFromUrl(targetOrgUrl)) {
        if (!doCheckFile(sourceInfo, targetPathInfo, fileName, toInfo, ok))
            return ok ? *ok : false;

        emitCurrentTaskNotify(sourceInfo->urlOf(UrlInfoType::kUrl), toInfo->urlOf(UrlInfoType::kUrl));
        bool result = renameFileByHandler(sourceInfo, toInfo);
        if (result) {
            if (targetPathInfo == this->targetInfo) {
                completeSourceFiles.append(sourceUrl);
                completeTargetFiles.append(toInfo->urlOf(UrlInfoType::kUrl));
            }
        }
        if (ok)
            *ok = result;
        return result;
    }

    if (!toInfo && !doCheckFile(sourceInfo, targetPathInfo, fileName, toInfo, ok))
        return false;

    return false;
}
