// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "docleantrashfilesworker.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/file/local/localfilehandler.h>

#include <dfm-io/denumerator.h>

#include <QUrl>
#include <QDebug>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE
DoCleanTrashFilesWorker::DoCleanTrashFilesWorker(QObject *parent)
    : FileOperateBaseWorker(parent)
{
    jobType = AbstractJobHandler::JobType::kCleanTrashType;
}

DoCleanTrashFilesWorker::~DoCleanTrashFilesWorker()
{
    stop();
}

bool DoCleanTrashFilesWorker::doWork()
{
    if (!AbstractWorker::doWork())
        return false;

    cleanAllTrashFiles();

    endWork();

    return true;
}

void DoCleanTrashFilesWorker::onUpdateProgress()
{
    emitProgressChangedNotify(cleanTrashFilesCount);
}

bool DoCleanTrashFilesWorker::statisticsFilesSize()
{
    if (sourceUrls.count() == 0) {
        fmWarning() << "sources files list is empty!";
        return false;
    }

    if (sourceUrls.size() == 1) {
        const QUrl &urlSource = sourceUrls[0];
        if (UniversalUtils::urlEquals(urlSource, FileUtils::trashRootUrl())) {
            DFMIO::DEnumerator enumerator(urlSource);
            while (enumerator.hasNext()) {
                auto url = FileUtils::bindUrlTransform(enumerator.next());
                if (!allFilesList.contains(url))
                    allFilesList.append(url);
            }
        }
    }

    return true;
}

bool DoCleanTrashFilesWorker::initArgs()
{
    return AbstractWorker::initArgs();
}

/*!
 * \brief DoCleanTrashFilesWorker::deleteAllFiles delete All files
 * \return delete all files success
 */
bool DoCleanTrashFilesWorker::cleanAllTrashFiles()
{
    QList<QUrl>::iterator it = sourceUrls.begin();
    QList<QUrl>::iterator itend = sourceUrls.end();
    if (!allFilesList.isEmpty()) {
        fmInfo() << "sourceUrls has children, use allFilesList replace sourceUrls"
                 << " sourceUrls: " << sourceUrls;
        if (allFilesList.size() > 20)
            fmInfo() << "allFilesList size > 20, ignore allFilesList print";
        else
            fmInfo() << "allFilesList: " << allFilesList;

        it = allFilesList.begin();
        itend = allFilesList.end();
    }
    while (it != itend) {
        if (!stateCheck())
            return false;
        const QUrl &url = *it;
        emitCurrentTaskNotify(url, QUrl());

        if (url.scheme() != Global::Scheme::kTrash) {
            // pause and emit error msg
            AbstractJobHandler::SupportAction action = doHandleErrorAndWait(url, AbstractJobHandler::JobErrorType::kIsNotTrashFileError);
            if (AbstractJobHandler::SupportAction::kSkipAction != action) {
                return false;
            } else {
                ++it;
                continue;
            }
        }

        const auto &fileInfo = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
        if (!fileInfo) {
            // pause and emit error msg
            AbstractJobHandler::SupportAction action = doHandleErrorAndWait(url, AbstractJobHandler::JobErrorType::kProrogramError);
            if (AbstractJobHandler::SupportAction::kSkipAction != action) {
                return false;
            } else {
                ++it;
                continue;
            }
        }

        if (!clearTrashFile(fileInfo))
            return false;

        cleanTrashFilesCount++;
        completeTargetFiles.append(fileInfo->urlOf(UrlInfoType::kRedirectedFileUrl));
        ++it;
    }
    return true;
}
/*!
 * \brief DoCleanTrashFilesWorker::clearTrashFile
 * \param fromUrl URL of the source file
 * \param toUrl Destination URL
 * \param trashInfo File information in Recycle Bin
 * \return Is the execution successful
 */
bool DoCleanTrashFilesWorker::clearTrashFile(const FileInfoPointer &trashInfo)
{
    if (!trashInfo)
        return false;

    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        const QUrl &fileUrl = trashInfo->urlOf(UrlInfoType::kUrl);
        bool resultFile = deleteFile(fileUrl);

        if (!resultFile) {
            action = doHandleErrorAndWait(fileUrl, AbstractJobHandler::JobErrorType::kDeleteTrashFileError,
                                          false, localFileHandler->errorString());
        } else {
            emit fileDeleted(fileUrl);
        }

    } while (isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    return action == AbstractJobHandler::SupportAction::kNoAction || AbstractJobHandler::SupportAction::kSkipAction == action;
}
/*!
 * \brief DoCleanTrashFilesWorker::doHandleErrorAndWait Blocking handles errors and returns
 * actions supported by the operation
 * \param from source information
 * \param to target information
 * \param error error type
 * \param needRetry is neef retry action
 * \param errorMsg error message
 * \return support action
 */
AbstractJobHandler::SupportAction
DoCleanTrashFilesWorker::doHandleErrorAndWait(const QUrl &from,
                                              const AbstractJobHandler::JobErrorType &error,
                                              const bool isTo,
                                              const QString &errorMsg)
{
    setStat(AbstractJobHandler::JobState::kPauseState);
    emitErrorNotify(from, QUrl(), error, isTo, 0, errorMsg);
    {
        QMutexLocker locker(&mutex);
        waitCondition.wait(&mutex);
    }

    return currentAction;
}

bool DoCleanTrashFilesWorker::deleteFile(const QUrl &url)
{
    return localFileHandler->deleteFile(url);
}
