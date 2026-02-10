// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dorestoretrashfilesworker.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-io/dfmio_utils.h>
#include <dfm-io/denumerator.h>
#include <dfm-io/trashhelper.h>

#include <QUrl>

#include <QMutex>
#include <QSettings>
#include <QStorageInfo>

DPFILEOPERATIONS_USE_NAMESPACE
DoRestoreTrashFilesWorker::DoRestoreTrashFilesWorker(QObject *parent)
    : FileOperateBaseWorker(parent)
{
    jobType = AbstractJobHandler::JobType::kRestoreType;
}

DoRestoreTrashFilesWorker::~DoRestoreTrashFilesWorker()
{
    stop();
}

bool DoRestoreTrashFilesWorker::doWork()
{
    // The endcopy interface function has been called here
    if (!AbstractWorker::doWork())
        return false;

    if (translateUrls())
        doRestoreTrashFiles();
    // 完成
    endWork();

    return true;
}

bool DoRestoreTrashFilesWorker::statisticsFilesSize()
{
    sourceFilesCount = sourceUrls.size();
    if (sourceUrls.count() == 0) {
        fmWarning() << "Restore from trash operation failed: source files list is empty";
        return false;
    }

    if (sourceUrls.size() == 1) {
        const QUrl &urlSource = sourceUrls[0];
        if (UniversalUtils::urlEquals(urlSource, FileUtils::trashRootUrl())) {
            DFMIO::DEnumerator enumerator(urlSource);
            while (enumerator.hasNext())
                allFilesList.append(enumerator.next());
            sourceFilesCount = allFilesList.size();
        }
    }

    return true;
}

bool DoRestoreTrashFilesWorker::initArgs()
{
    completeTargetFiles.clear();
    isConvert = workData->jobFlags.testFlag(DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kRevocation);
    return AbstractWorker::initArgs();
}

void DoRestoreTrashFilesWorker::onUpdateProgress()
{
    emitProgressChangedNotify(completeFilesCount);
}

bool DoRestoreTrashFilesWorker::translateUrls()
{
    if (sourceUrls.length() <= 0 || sourceUrls.first().scheme() != dfmbase::Global::Scheme::kFile)
        return true;

    QMap<QUrl, QSharedPointer<TrashHelper::DeleteTimeInfo>> targetUrls;
    for (auto url : sourceUrls) {
        QStringList deleteInfo;
        AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
        do {
            action = AbstractJobHandler::SupportAction::kNoAction;
            auto userInfo = url.userInfo();
            deleteInfo = userInfo.split("-");
            // 错误处理
            if (deleteInfo.length() != 2) {
                fmWarning() << "Failed to parse trash file URL - url:" << url << "userInfo:" << userInfo;
                // pause and emit error msg
                action = doHandleErrorAndWait(url, parentUrl(url), AbstractJobHandler::JobErrorType::kFailedParseUrlOfTrash);
            }

        } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

        if (action == AbstractJobHandler::SupportAction::kSkipAction)
            continue;

        if (action != AbstractJobHandler::SupportAction::kNoAction)
            return false;
        if (deleteInfo.size() >= 2) {
            QSharedPointer<TrashHelper::DeleteTimeInfo> info(new TrashHelper::DeleteTimeInfo);
            info->startTime = deleteInfo.first().toInt();
            info->endTime = deleteInfo.at(1).toInt();
            url.setUserInfo("");
            targetUrls.insert(QUrl::fromPercentEncoding(url.toString().toUtf8()), info);
        }
    }

    if (targetUrls.size() < 0)
        return false;

    QString errorMsg;

    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    TrashHelper trashHelper;
    trashHelper.setDeleteInfos(targetUrls);
    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        if (!trashHelper.getTrashUrls(&sourceUrls, &errorMsg)) {
            fmWarning() << "Failed to get trash URLs - error:" << errorMsg;
            return false;
        }
        if (sourceUrls.length() <= 0) {
            fmWarning() << "No trash URLs found - error:" << errorMsg;
            action = doHandleErrorAndWait(targetUrls.keys().length() > 0 ? targetUrls.keys().first() : FileUtils::trashRootUrl(), QUrl(),
                                          AbstractJobHandler::JobErrorType::kFailedObtainTrashOriginalFile, false, errorMsg);
        }
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    if (action != AbstractJobHandler::SupportAction::kNoAction)
        return false;

    return true;
}

/*!
 * \brief DoRestoreTrashFilesWorker::doRestoreTrashFiles Performing a recycle bin restore
 * \return Is the recycle bin restore successful
 */
bool DoRestoreTrashFilesWorker::doRestoreTrashFiles()
{
    // 获取当前的
    bool result = false;
    // 总大小使用源文件个数
    QList<QUrl> urlsSource = sourceUrls;
    if (!allFilesList.empty())
        urlsSource = allFilesList;

    QMap<QUrl, QString> failUrls;
    for (const auto &url : urlsSource) {
        if (!stateCheck())
            return false;
        auto fileUrl = FileUtils::bindUrlTransform(url);
        if (handleSourceFiles.contains(fileUrl))
            continue;

        DFileInfoPointer restoreInfo = checkRestoreInfo(url);
        if (restoreInfo.isNull()) {
            fmDebug() << "Check restore info failed - url:" << url;
            completeFilesCount++;
            handleSourceFiles.append(fileUrl);
            continue;
        }

        DFileInfoPointer targetInfo = createParentDir(url, restoreInfo, &result);
        if (targetInfo.isNull()) {
            if (result) {
                completeFilesCount++;
                handleSourceFiles.append(fileUrl);
                continue;
            } else {
                fmDebug() << "Create parent directory failed for restore - url:" << url;
                return false;
            }
        }
        DFileInfoPointer fileInfo { new DFileInfo(url) };

        // read trash info
        QUrl trashInfoUrl { fileInfo->attribute(DFileInfo::AttributeID::kStandardTargetUri).toString().replace("/files/", "/info/") + ".trashinfo" };
        QUrl trashUrl = QUrl(fileInfo->attribute(DFileInfo::AttributeID::kStandardTargetUri).toString());
        fileInfo.reset(new DFileInfo(trashUrl));
        const QString &trashInfoCache { DFMIO::DFile(trashInfoUrl).readAll() };
        emitCurrentTaskNotify(url, restoreInfo->uri());
        bool ok = false;
        DFileInfoPointer newTargetInfo = doCheckFile(fileInfo,
                                                     targetInfo,
                                                     fileInfo->attribute(DFileInfo::AttributeID::kStandardFileName).toString(), &ok);
        if (newTargetInfo.isNull()) {
            fmDebug() << "File check failed for restore - from:" << url;
            handleSourceFiles.append(fileUrl);
            continue;
        }

        DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
        bool trashSucc = fileHandler.moveFile(url, newTargetInfo->uri(), DFMIO::DFile::CopyFlag::kOverwrite);
        if (trashSucc) {
            completeFilesCount++;
            if (!completeSourceFiles.contains(fileUrl)) {
                completeSourceFiles.append(fileUrl);
                completeCustomInfos.append(trashInfoCache);
            }
            if (!completeTargetFiles.contains(newTargetInfo->uri()))
                completeTargetFiles.append(newTargetInfo->uri());
            emit fileRenamed(fileUrl, newTargetInfo->uri());
        } else {
            auto errorCode = fileHandler.errorCode();
            switch (errorCode) {
            case DFMIOErrorCode::DFM_IO_ERROR_WOULD_MERGE: {
                fmDebug() << "Directory merge needed for restore - from:" << url << "to:" << newTargetInfo->uri();
                trashSucc = this->mergeDir(url, newTargetInfo->uri(), DFMIO::DFile::CopyFlag::kOverwrite);
                break;
            };
            default:
                fmWarning() << "Failed to restore file from trash - from:" << url << "to:" << newTargetInfo->uri()
                            << "error:" << fileHandler.errorString() << "code:" << errorCode;
                break;
            }
            if (!trashSucc)
                failUrls.insert(url, fileHandler.errorString());
        }
        handleSourceFiles.append(fileUrl);
    }

    if (failUrls.count() > 0) {
        fmWarning() << "Restore operation completed with failures - failed count:" << failUrls.count();
        emit requestShowFailedDialog(failUrls);
        return false;
    }

    return true;
}

DFileInfoPointer DoRestoreTrashFilesWorker::createParentDir(const QUrl &fromUrl,
                                                            const DFileInfoPointer &restoreInfo,
                                                            bool *result)
{
    const QUrl &toUrl = restoreInfo->uri();
    const QUrl &parentUrl = AbstractWorker::parentUrl(toUrl);
    if (!parentUrl.isValid()) {
        fmWarning() << "Invalid parent URL when restoring from trash - target:" << toUrl;
        return nullptr;
    }
    DFileInfoPointer targetFileInfo { new DFileInfo(parentUrl) };
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    targetFileInfo->initQuerier();
    if (!targetFileInfo->exists()) {
        do {
            action = AbstractJobHandler::SupportAction::kNoAction;
            DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
            if (!fileHandler.mkdir(parentUrl)) {
                fmWarning() << "Failed to create parent directory when restoring - dir:" << parentUrl << "error:" << fileHandler.errorString();
                // pause and emit error msg
                action = doHandleErrorAndWait(fromUrl, toUrl, AbstractJobHandler::JobErrorType::kCreateParentDirError, true, fileHandler.errorString());
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

DFileInfoPointer DoRestoreTrashFilesWorker::checkRestoreInfo(const QUrl &url)
{
    DFileInfoPointer result { nullptr };
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        DFileInfoPointer fileInfo(new DFileInfo(url));

        QUrl restoreFileUrl;
        if (!this->targetUrl.isValid()) {
            // 获取回收站文件的原路径
            restoreFileUrl = QUrl::fromLocalFile(fileInfo->attribute(DFileInfo::AttributeID::kTrashOrigPath).toString());
            if (!restoreFileUrl.isValid()) {
                fmWarning() << "Failed to get restore path from trash - url:" << url << "origPath:" << fileInfo->attribute(DFileInfo::AttributeID::kTrashOrigPath).toString();
                action = doHandleErrorAndWait(url, restoreFileUrl, AbstractJobHandler::JobErrorType::kGetRestorePathError);
                result.clear();
                continue;
            }
        } else {
            restoreFileUrl = DFMIO::DFMUtils::buildFilePath(this->targetUrl.toString().toStdString().c_str(),
                                                            fileInfo->attribute(DFileInfo::AttributeID::kStandardFileName).toString().toStdString().c_str(), nullptr);
        }

        result.reset(new DFileInfo(restoreFileUrl));
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    return result;
}

bool DoRestoreTrashFilesWorker::mergeDir(const QUrl &urlSource, const QUrl &urlTarget, DFile::CopyFlag flag)
{
    const bool succ = this->copyFileFromTrash(urlSource, urlTarget, flag);
    if (succ) {
        DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
        return fileHandler.deleteFile(urlSource);
    }
    return false;
}
