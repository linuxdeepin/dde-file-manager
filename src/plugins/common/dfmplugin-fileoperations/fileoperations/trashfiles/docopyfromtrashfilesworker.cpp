// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "docopyfromtrashfilesworker.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-io/dfmio_utils.h>

#include <QUrl>

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
        fmWarning() << "Copy from trash operation failed: source files list is empty";
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

        const DFileInfoPointer fileInfo(new DFileInfo(url));
        if (!fileInfo) {
            fmCritical() << "Failed to create FileInfo object for trash file - url:" << url;
            // pause and emit error msg
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(url, QUrl(), AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                completeFilesCount++;
                continue;
            }
        }

        const QUrl &targetFileUrl = DFMIO::DFMUtils::buildFilePath(this->targetUrl.toString().toStdString().c_str(),
                                                                   fileInfo->attribute(DFileInfo::AttributeID::kStandardDisplayName).
                                                                   toString().toStdString().c_str(), nullptr);

        const DFileInfoPointer targetFileInfo(new DFileInfo(targetFileUrl));
        DFileInfoPointer targetInfo = createParentDir(fileInfo, targetFileInfo, &result);
        if (targetInfo.isNull()) {
            if (result) {
                completeFilesCount++;
                continue;
            } else {
                fmDebug() << "Create parent directory failed for trash file copy - from:" << url;
                return false;
            }
        }

        // read trash info
        emitCurrentTaskNotify(url, targetFileUrl);

        bool ok = false;
        DFileInfoPointer newTargetInfo = doCheckFile(fileInfo, targetInfo, fileInfo->attribute(DFileInfo::AttributeID::kStandardFileName).toString(), &ok);

        if (newTargetInfo.isNull()) {
            fmDebug() << "File check failed for trash file copy - from:" << url;
            continue;
        }

        bool copySucc = this->copyFileFromTrash(url, newTargetInfo->uri(), DFMIO::DFile::CopyFlag::kOverwrite);
        if (copySucc) {
            completeFilesCount++;
            if (!completeSourceFiles.contains(url)) {
                completeSourceFiles.append(url);
            }
            if (!completeTargetFiles.contains(targetFileInfo->uri()))
                completeTargetFiles.append(targetFileInfo->uri());
            continue;
        }
        fmWarning() << "Failed to copy file from trash - from:" << url << "to:" << newTargetInfo->uri();
        return false;
    }

    return true;
}

DFileInfoPointer DoCopyFromTrashFilesWorker::createParentDir(const DFileInfoPointer &trashInfo,
                                                             const DFileInfoPointer &restoreInfo,
                                                             bool *result)
{
    const QUrl &fromUrl = trashInfo->uri();
    const QUrl &toUrl = restoreInfo->uri();
    const QUrl &parentUrl = AbstractWorker::parentUrl(toUrl);
    if (!parentUrl.isValid()) {
        fmWarning() << "Invalid parent URL when copying from trash - target:" << toUrl;
        return nullptr;
    }
    DFileInfoPointer targetFileInfo(new DFileInfo(parentUrl));

    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    targetFileInfo->initQuerier();
    if (!targetFileInfo->exists()) {
        do {
            action = AbstractJobHandler::SupportAction::kNoAction;
            DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
            if (!fileHandler.mkdir(parentUrl)) {
                fmWarning() << "Failed to create parent directory when copying from trash - dir:" << parentUrl << "error:" << fileHandler.errorString();
                // pause and emit error msg
                action = doHandleErrorAndWait(fromUrl, toUrl, AbstractJobHandler::JobErrorType::kCreateParentDirError,
                                              true, fileHandler.errorString());
            }
        } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

        if (action != AbstractJobHandler::SupportAction::kNoAction) {
            if (result)
                *result = action == AbstractJobHandler::SupportAction::kSkipAction;
            return nullptr;
        }
    }

    return targetFileInfo;
}
