// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dodeletefilesworker.h"
#include <dfm-base/base/schemefactory.h>

#include <QUrl>
#include <QDebug>

DPFILEOPERATIONS_USE_NAMESPACE
DoDeleteFilesWorker::DoDeleteFilesWorker(QObject *parent)
    : AbstractWorker(parent)
{
    jobType = AbstractJobHandler::JobType::kDeleteType;
}

DoDeleteFilesWorker::~DoDeleteFilesWorker()
{
    stop();
}

bool DoDeleteFilesWorker::doWork()
{
    if (!AbstractWorker::doWork())
        return false;

    deleteAllFiles();

    // 完成
    endWork();

    return true;
}

void DoDeleteFilesWorker::stop()
{
    // ToDo::停止删除的业务逻辑
    AbstractWorker::stop();
}

void DoDeleteFilesWorker::onUpdateProgress()
{
    emitProgressChangedNotify(deleteFilesCount);
}

/*!
 * \brief DoDeleteFilesWorker::deleteAllFiles delete All files
 * \return delete all files success
 */
bool DoDeleteFilesWorker::deleteAllFiles()
{
    // sources file list is checked
    // delete files on can't remove device
    if (isSourceFileLocal) {
        return deleteFilesOnCanNotRemoveDevice();
    }
    return deleteFilesOnOtherDevice();
}
/*!
 * \brief DoDeleteFilesWorker::deleteFilesOnCanNotRemoveDevice Delete files on non removable devices
 * \return delete file success
 */
bool DoDeleteFilesWorker::deleteFilesOnCanNotRemoveDevice()
{
    if (allFilesList.count() == 1 && isConvert) {
        auto info = InfoFactory::create<FileInfo>(allFilesList.first(), Global::CreateFileInfoType::kCreateFileInfoSync);
        if (info)
            deleteFirstFileSize = info->size();
    }

    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    for (QList<QUrl>::iterator it = --allFilesList.end(); it != --allFilesList.begin(); --it) {
        if (!stateCheck())
            return false;
        const QUrl &url = *it;
        auto info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
        emitCurrentTaskNotify(url, QUrl());
        do {
            action = AbstractJobHandler::SupportAction::kNoAction;
            if (!localFileHandler->deleteFile(url)) {
                action = doHandleErrorAndWait(url, AbstractJobHandler::JobErrorType::kDeleteFileError,
                                              localFileHandler->errorString());
            }
        } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

        if (sourceUrls.contains(url)) {
            if (action == AbstractJobHandler::SupportAction::kNoAction) {
                completeSourceFiles.append(url);
                completeTargetFiles.append(url);
            }
        }

        deleteFilesCount++;

        if (action == AbstractJobHandler::SupportAction::kSkipAction)
            continue;

        if (action != AbstractJobHandler::SupportAction::kNoAction)
            return false;

        dpfSignalDispatcher->publish("dfmplugin_fileoperations", "signal_File_Delete", url);
    }
    return true;
}
/*!
 * \brief DoDeleteFilesWorker::deleteFilesOnOtherDevice Delete files on removable devices and other
 * \return delete file success
 */
bool DoDeleteFilesWorker::deleteFilesOnOtherDevice()
{
    bool ok = true;
    if (sourceUrls.count() == 1 && isConvert) {
        auto info = InfoFactory::create<FileInfo>(sourceUrls.first(), Global::CreateFileInfoType::kCreateFileInfoSync);
        if (info)
            deleteFirstFileSize = info->size();
    }
    for (auto &url : sourceUrls) {
        const auto &info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
        if (!info) {
            // pause and emit error msg
            if (doHandleErrorAndWait(url, AbstractJobHandler::JobErrorType::kProrogramError) == AbstractJobHandler::SupportAction::kSkipAction)
                continue;
            return false;
        }

        if (info->isAttributes(OptInfoType::kIsSymLink) || info->isAttributes(OptInfoType::kIsFile)) {
            ok = deleteFileOnOtherDevice(url);
        } else {
            ok = deleteDirOnOtherDevice(info);
        }

        if (!ok)
            return false;
        completeTargetFiles.append(url);
        completeSourceFiles.append(url);
    }
    return true;
}
/*!
 * \brief DoDeleteFilesWorker::deleteFileOnOtherDevice Delete file on removable devices and other
 * \param url delete url
 * \return delete success
 */
bool DoDeleteFilesWorker::deleteFileOnOtherDevice(const QUrl &url)
{
    if (!stateCheck())
        return false;

    emitCurrentTaskNotify(url, QUrl());

    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        if (!localFileHandler->deleteFile(url)) {
            action = doHandleErrorAndWait(url, AbstractJobHandler::JobErrorType::kDeleteFileError,
                                          localFileHandler->errorString());
        }
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    deleteFilesCount++;

    if (action == AbstractJobHandler::SupportAction::kSkipAction)
        return true;

    return action == AbstractJobHandler::SupportAction::kNoAction;
}
/*!
 * \brief DoDeleteFilesWorker::deleteDirOnOtherDevice Delete dir on removable devices and other
 * \param dir delete dir
 * \return delete success
 */
bool DoDeleteFilesWorker::deleteDirOnOtherDevice(const FileInfoPointer &dir)
{
    if (!stateCheck())
        return false;

    if (dir->countChildFile() < 0)
        return deleteFileOnOtherDevice(dir->urlOf(UrlInfoType::kUrl));

    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    AbstractDirIteratorPointer iterator(nullptr);
    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        QString errorMsg;
        iterator = DirIteratorFactory::create<AbstractDirIterator>(dir->urlOf(UrlInfoType::kUrl), &errorMsg);
        if (!iterator) {
            action = doHandleErrorAndWait(dir->urlOf(UrlInfoType::kUrl), AbstractJobHandler::JobErrorType::kDeleteFileError, errorMsg);
        }
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    if (action == AbstractJobHandler::SupportAction::kSkipAction)
        return true;
    if (action != AbstractJobHandler::SupportAction::kNoAction)
        return false;

    bool ok { true };
    while (iterator->hasNext()) {
        const QUrl &url = iterator->next();

        const auto &info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);;
        if (!info) {
            // pause and emit error msg
            if (doHandleErrorAndWait(url, AbstractJobHandler::JobErrorType::kProrogramError) == AbstractJobHandler::SupportAction::kSkipAction)
                continue;
            return false;
        }

        if (info->isAttributes(OptInfoType::kIsSymLink) || info->isAttributes(OptInfoType::kIsFile)) {
            ok = deleteFileOnOtherDevice(url);
        } else {
            ok = deleteDirOnOtherDevice(info);
        }

        if (!ok)
            return false;
    }

    // delete self dir
    return deleteFileOnOtherDevice(dir->urlOf(UrlInfoType::kUrl));
}
/*!
 * \brief DoCopyFilesWorker::doHandleErrorAndWait Blocking handles errors and returns
 * actions supported by the operation
 * \param from source information
 * \param to target information
 * \param error error type
 * \param needRetry is neef retry action
 * \param errorMsg error message
 * \return support action
 */
AbstractJobHandler::SupportAction
DoDeleteFilesWorker::doHandleErrorAndWait(const QUrl &from,
                                          const AbstractJobHandler::JobErrorType &error,
                                          const QString &errorMsg)
{
    setStat(AbstractJobHandler::JobState::kPauseState);
    emitErrorNotify(from, QUrl(), error, false, 0, errorMsg);

    waitCondition.wait(&mutex);

    return currentAction;
}
