// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "docopyfromtrashfilesworker.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-io/dfmio_utils.h>

#include <QUrl>
#include <QDebug>
#include <QMutex>
#include <QSettings>
#include <QStorageInfo>

DPFILEOPERATIONS_USE_NAMESPACE
DoCopyFromTrashFilesWorker::DoCopyFromTrashFilesWorker(QObject *parent)
    : FileOperateBaseWorker(parent)
{
    jobType = AbstractJobHandler::JobType::kCopyType;
}

DoCopyFromTrashFilesWorker::~DoCopyFromTrashFilesWorker()
{
    stop();
}

bool DoCopyFromTrashFilesWorker::doWork()
{
    // The endcopy interface function has been called here
    if (!AbstractWorker::doWork())
        return false;

    doOperate();

    // 完成
    endWork();

    return true;
}

bool DoCopyFromTrashFilesWorker::statisticsFilesSize()
{
    sourceFilesCount = sourceUrls.size();
    if (sourceUrls.count() == 0) {
        qWarning() << "sources files list is empty!";
        return false;
    }

    return true;
}

bool DoCopyFromTrashFilesWorker::initArgs()
{
    completeTargetFiles.clear();
    return AbstractWorker::initArgs();
}
/*!
 * \brief DoCopyFromTrashFilesWorker::doRestoreTrashFiles Performing a recycle bin restore
 * \return Is the recycle bin restore successful
 */
bool DoCopyFromTrashFilesWorker::doOperate()
{
    bool result = false;

    for (const auto &url : sourceUrls) {
        if (!stateCheck())
            return false;

        const FileInfoPointer &fileInfo = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
        if (!fileInfo) {
            // pause and emit error msg
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(url, QUrl(), AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                completeFilesCount++;
                continue;
            }
        }

        const QUrl &targetFileUrl = DFMIO::DFMUtils::buildFilePath(this->targetUrl.toString().toStdString().c_str(),
                                                                   fileInfo->displayOf(DisPlayInfoType::kFileDisplayName).toStdString().c_str(), nullptr);

        const FileInfoPointer &targetFileInfo = InfoFactory::create<FileInfo>(targetFileUrl, Global::CreateFileInfoType::kCreateFileInfoSync);
        if (!targetFileInfo) {
            // pause and emit error msg
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(url, targetFileUrl, AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                completeFilesCount++;
                continue;
            }
        }

        FileInfoPointer targetInfo = nullptr;
        if (!createParentDir(fileInfo, targetFileInfo, targetInfo, &result)) {
            if (result) {
                completeFilesCount++;
                continue;
            } else {
                return false;
            }
        }

        // read trash info
        emitCurrentTaskNotify(url, targetFileUrl);

        FileInfoPointer newTargetInfo(nullptr);
        bool ok = false;
        if (!doCheckFile(fileInfo, targetInfo, fileInfo->nameOf(NameInfoType::kFileCopyName), newTargetInfo, &ok))
            continue;

        bool copySucc = this->copyFileFromTrash(url, newTargetInfo->urlOf(UrlInfoType::kUrl), DFMIO::DFile::CopyFlag::kOverwrite);
        if (copySucc) {
            completeFilesCount++;
            if (!completeSourceFiles.contains(url)) {
                completeSourceFiles.append(url);
            }
            if (!completeTargetFiles.contains(targetFileInfo->urlOf(UrlInfoType::kUrl)))
                completeTargetFiles.append(targetFileInfo->urlOf(UrlInfoType::kUrl));
            continue;
        }
        return false;
    }

    return true;
}

bool DoCopyFromTrashFilesWorker::createParentDir(const FileInfoPointer &trashInfo, const FileInfoPointer &restoreInfo,
                                                 FileInfoPointer &targetFileInfo, bool *result)
{
    const QUrl &fromUrl = trashInfo->urlOf(UrlInfoType::kUrl);
    const QUrl &toUrl = restoreInfo->urlOf(UrlInfoType::kUrl);
    const QUrl &parentUrl = UrlRoute::urlParent(toUrl);
    if (!parentUrl.isValid())
        return false;
    targetFileInfo.reset();
    targetFileInfo = InfoFactory::create<FileInfo>(parentUrl, Global::CreateFileInfoType::kCreateFileInfoSync);
    if (!targetFileInfo)
        return false;

    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    if (!targetFileInfo->exists()) {
        do {
            action = AbstractJobHandler::SupportAction::kNoAction;
            DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
            if (!fileHandler.mkdir(parentUrl))
                // pause and emit error msg
                action = doHandleErrorAndWait(fromUrl, toUrl, AbstractJobHandler::JobErrorType::kCreateParentDirError,
                                              true, fileHandler.errorString());
        } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

        if (action != AbstractJobHandler::SupportAction::kNoAction) {
            if (result)
                *result = action == AbstractJobHandler::SupportAction::kSkipAction;
            return false;
        }
    }

    return true;
}
