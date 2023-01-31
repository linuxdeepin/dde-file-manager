/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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
#include "dorestoretrashfilesworker.h"
#include "fileoperations/copyfiles/storageinfo.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/utils/decorator/decoratorfile.h"
#include "dfm-base/utils/decorator/decoratorfileenumerator.h"
#include "dfm-base/utils/universalutils.h"

#include <dfm-io/dfmio_global.h>
#include <dfm-io/dfmio_utils.h>
#include <dfm-io/core/diofactory.h>

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
            DecoratorFileEnumerator enumerator(urlSource);
            if (!enumerator.isValid())
                return false;
            while (enumerator.hasNext()) {
                allFilesList.append(enumerator.next());
            }
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

        const auto &fileInfo = InfoFactory::create<AbstractFileInfo>(url);
        if (!fileInfo) {
            // pause and emit error msg
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(url, QUrl(), AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                completeFilesCount++;
                continue;
            }
        }

        QUrl restoreFileUrl;
        if (!this->targetUrl.isValid()) {
            // 获取回收站文件的原路径
            restoreFileUrl = fileInfo->urlOf(UrlInfoType::kOriginalUrl);
            if (!restoreFileUrl.isValid()) {
                completeFilesCount++;
                continue;
            }
        } else {

            restoreFileUrl = DFMIO::DFMUtils::buildFilePath(this->targetUrl.toString().toStdString().c_str(),
                                                            fileInfo->nameOf(NameInfoType::kFileCopyName).toStdString().c_str(), nullptr);
        }

        const AbstractFileInfoPointer &restoreInfo = InfoFactory::create<AbstractFileInfo>(restoreFileUrl, false);
        if (!restoreInfo) {
            // pause and emit error msg
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(url, restoreFileUrl, AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                completeFilesCount++;
                continue;
            }
        }

        AbstractFileInfoPointer targetInfo = nullptr;
        if (!createParentDir(fileInfo, restoreInfo, targetInfo, &result)) {
            if (result) {
                completeFilesCount++;
                continue;
            } else {
                return false;
            }
        }

        // read trash info
        QString trashInfoCache = readTrashInfo(fileInfo->urlOf(UrlInfoType::kRedirectedFileUrl).toString().replace("/files/", "/info/") + ".trashinfo");
        emitCurrentTaskNotify(url, restoreFileUrl);
        AbstractFileInfoPointer newTargetInfo(nullptr);
        bool ok = false;
        if (!doCheckFile(fileInfo, targetInfo, fileInfo->nameOf(NameInfoType::kFileCopyName), newTargetInfo, &ok))
            continue;

        DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
        bool trashSucc = fileHandler.moveFile(url, newTargetInfo->urlOf(UrlInfoType::kUrl), DFMIO::DFile::CopyFlag::kOverwrite);
        if (trashSucc) {
            completeFilesCount++;
            if (!completeSourceFiles.contains(url)) {
                completeSourceFiles.append(url);
                completeCustomInfos.append(trashInfoCache);
            }
            if (!completeTargetFiles.contains(restoreInfo->urlOf(UrlInfoType::kUrl)))
                completeTargetFiles.append(restoreInfo->urlOf(UrlInfoType::kUrl));
            continue;
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
    }

    if (failUrls.count() > 0) {
        emit requestShowTipsDialog(DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType::kRestoreFailed, failUrls);
        return false;
    }

    return true;
}

bool DoRestoreTrashFilesWorker::createParentDir(const AbstractFileInfoPointer &trashInfo, const AbstractFileInfoPointer &restoreInfo,
                                                AbstractFileInfoPointer &targetFileInfo, bool *result)
{
    const QUrl &fromUrl = trashInfo->urlOf(UrlInfoType::kUrl);
    const QUrl &toUrl = restoreInfo->urlOf(UrlInfoType::kUrl);
    const QUrl &parentUrl = UrlRoute::urlParent(toUrl);
    if (!parentUrl.isValid())
        return false;
    targetFileInfo.reset();
    targetFileInfo = InfoFactory::create<AbstractFileInfo>(parentUrl);
    if (!targetFileInfo)
        return false;

    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    if (!DecoratorFile(parentUrl).exists()) {
        do {
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

QString DoRestoreTrashFilesWorker::readTrashInfo(const QUrl &url)
{
    DecoratorFile file(url);
    return file.readAll();
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
