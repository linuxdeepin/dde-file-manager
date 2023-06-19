// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
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
#include <QDebug>
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
        qWarning() << "sources files list is empty!";
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
            if (deleteInfo.length() != 2)
                // pause and emit error msg
                action = doHandleErrorAndWait(url, UrlRoute::urlParent(url), AbstractJobHandler::JobErrorType::kFailedParseUrlOfTrash);

        } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

        if (action == AbstractJobHandler::SupportAction::kSkipAction)
            continue;

        if (action != AbstractJobHandler::SupportAction::kNoAction)
            return false;
        QSharedPointer<TrashHelper::DeleteTimeInfo> info(new TrashHelper::DeleteTimeInfo);
        info->startTime = deleteInfo.first().toInt();
        info->endTime = deleteInfo.at(1).toInt();
        url.setUserInfo("");
        targetUrls.insert(url, info);
    }

    if (targetUrls.size() < 0)
        return false;

    QString errorMsg;

    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    TrashHelper trashHelper;
    trashHelper.setDeleteInfos(targetUrls);
    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        if (!trashHelper.getTrashUrls(&sourceUrls, &errorMsg))
            return false;
        if (sourceUrls.length() <= 0)
            action = doHandleErrorAndWait(targetUrls.keys().length() > 0 ? targetUrls.keys().first() : FileUtils::trashRootUrl(), QUrl(),
                                          AbstractJobHandler::JobErrorType::kFailedObtainTrashOriginalFile, false, errorMsg);
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
    //获取当前的
    bool result = false;
    // 总大小使用源文件个数
    QList<QUrl> urlsSource = sourceUrls;
    if (!allFilesList.empty())
        urlsSource = allFilesList;

    QList<QUrl> failUrls;
    for (const auto &url : urlsSource) {
        if (!stateCheck())
            return false;
        auto fileUrl = FileUtils::bindUrlTransform(url);
        if (handleSourceFiles.contains(fileUrl))
            continue;

        FileInfoPointer restoreInfo { nullptr };
        if (!checkRestoreInfo(url, restoreInfo)) {
            completeFilesCount++;
            handleSourceFiles.append(fileUrl);
            continue;
        }

        const auto &fileInfo = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
        FileInfoPointer targetInfo = nullptr;
        if (!createParentDir(fileInfo, restoreInfo, targetInfo, &result)) {
            if (result) {
                completeFilesCount++;
                handleSourceFiles.append(fileUrl);
                continue;
            } else {
                return false;
            }
        }

        // read trash info
        QUrl trashInfoUrl { fileInfo->urlOf(UrlInfoType::kRedirectedFileUrl).toString().replace("/files/", "/info/") + ".trashinfo" };
        const QString &trashInfoCache { DFMIO::DFile(trashInfoUrl).readAll() };
        emitCurrentTaskNotify(url, restoreInfo->urlOf(UrlInfoType::kUrl));
        FileInfoPointer newTargetInfo(nullptr);
        bool ok = false;
        if (!doCheckFile(fileInfo, targetInfo, fileInfo->nameOf(NameInfoType::kFileCopyName), newTargetInfo, &ok)) {
            handleSourceFiles.append(fileUrl);
            continue;
        }

        DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
        bool trashSucc = fileHandler.moveFile(url, newTargetInfo->urlOf(UrlInfoType::kUrl), DFMIO::DFile::CopyFlag::kOverwrite);
        if (trashSucc) {
            completeFilesCount++;
            if (!completeSourceFiles.contains(fileUrl)) {
                completeSourceFiles.append(fileUrl);
                completeCustomInfos.append(trashInfoCache);
            }
            if (!completeTargetFiles.contains(restoreInfo->urlOf(UrlInfoType::kUrl)))
                completeTargetFiles.append(restoreInfo->urlOf(UrlInfoType::kUrl));
        } else {
            auto errorCode = fileHandler.errorCode();
            switch (errorCode) {
            case DFMIOErrorCode::DFM_IO_ERROR_WOULD_MERGE: {
                trashSucc = this->mergeDir(url, newTargetInfo->urlOf(UrlInfoType::kUrl), DFMIO::DFile::CopyFlag::kOverwrite);
                break;
            };
            default:
                break;
            }
            if (!trashSucc)
                failUrls.append(url);
        }
        handleSourceFiles.append(fileUrl);
    }

    if (failUrls.count() > 0) {
        emit requestShowTipsDialog(DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType::kRestoreFailed, failUrls);
        return false;
    }

    return true;
}

bool DoRestoreTrashFilesWorker::createParentDir(const FileInfoPointer &trashInfo, const FileInfoPointer &restoreInfo,
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
                action = doHandleErrorAndWait(fromUrl, toUrl, AbstractJobHandler::JobErrorType::kCreateParentDirError, true, fileHandler.errorString());
        } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

        if (action != AbstractJobHandler::SupportAction::kNoAction) {
            if (result)
                *result = action == AbstractJobHandler::SupportAction::kSkipAction;
            return false;
        }
    }

    return true;
}

bool DoRestoreTrashFilesWorker::checkRestoreInfo(const QUrl &url, FileInfoPointer &restoreInfo)
{
    bool result;
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        result = true;
        const auto &fileInfo = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
        if (!fileInfo) {
            // pause and emit error msg
            action = doHandleErrorAndWait(url, QUrl(), AbstractJobHandler::JobErrorType::kProrogramError);
            result = false;
            continue;
        }

        QUrl restoreFileUrl;
        if (!this->targetUrl.isValid()) {
            // 获取回收站文件的原路径
            restoreFileUrl = fileInfo->urlOf(UrlInfoType::kOriginalUrl);
            if (!restoreFileUrl.isValid()) {
                action = doHandleErrorAndWait(url, restoreFileUrl, AbstractJobHandler::JobErrorType::kGetRestorePathError);
                result = false;
                continue;
            }
        } else {
            restoreFileUrl = DFMIO::DFMUtils::buildFilePath(this->targetUrl.toString().toStdString().c_str(),
                                                            fileInfo->nameOf(NameInfoType::kFileCopyName).toStdString().c_str(), nullptr);
        }

        restoreInfo = InfoFactory::create<FileInfo>(restoreFileUrl, Global::CreateFileInfoType::kCreateFileInfoSync);
        if (!restoreInfo) {
            // pause and emit error msg
            action = doHandleErrorAndWait(url, restoreFileUrl, AbstractJobHandler::JobErrorType::kProrogramError);
            result = false;
            continue;
        }
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
