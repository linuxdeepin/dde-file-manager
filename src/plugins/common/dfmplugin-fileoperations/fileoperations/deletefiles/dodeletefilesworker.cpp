// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dodeletefilesworker.h"
#include <dfm-base/base/schemefactory.h>

#include <QUrl>

DPFILEOPERATIONS_USE_NAMESPACE
DoDeleteFilesWorker::DoDeleteFilesWorker(QObject *parent)
    : AbstractWorker(parent)
{
    jobType = AbstractJobHandler::JobType::kDeleteType;
    fmDebug() << "Delete files worker created";
}

DoDeleteFilesWorker::~DoDeleteFilesWorker()
{
    stop();
    fmDebug() << "Delete files worker destroyed";
}

bool DoDeleteFilesWorker::doWork()
{
    if (!AbstractWorker::doWork())
        return false;

    fmInfo() << "Start deleting files - count:" << sourceUrls.count();
    deleteAllFiles();

    // 完成
    fmInfo() << "Delete operation completed successfully";
    endWork();

    return true;
}

void DoDeleteFilesWorker::stop()
{
    fmInfo() << "Stopping delete operation";
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
    fmDebug() << "Delete all files - source file local:" << isSourceFileLocal;
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
    fmDebug() << "Deleting files on non-removable device - file count:" << allFilesList.count();

    if (allFilesList.count() == 1 && isConvert) {
        auto info = InfoFactory::create<FileInfo>(allFilesList.first(), Global::CreateFileInfoType::kCreateFileInfoSync);
        if (info) {
            deleteFirstFileSize = info->size();
            fmDebug() << "Single file deletion, size:" << deleteFirstFileSize;
        }
    }

    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    for (QList<QUrl>::iterator it = --allFilesList.end(); it != --allFilesList.begin(); --it) {
        if (!stateCheck())
            return false;
        const QUrl &url = *it;
        emitCurrentTaskNotify(url, QUrl());
        do {
            action = AbstractJobHandler::SupportAction::kNoAction;
            if (!localFileHandler->deleteFile(url)) {
                fmWarning() << "Delete file failed - file:" << url << "error:" << localFileHandler->errorString();
                action = doHandleErrorAndWait(url, AbstractJobHandler::JobErrorType::kDeleteFileError,
                                              localFileHandler->errorString());
            } else {
                fmDebug() << "Successfully deleted file:" << url;
            }
        } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

        if (sourceUrls.contains(url)) {
            if (action == AbstractJobHandler::SupportAction::kNoAction) {
                completeSourceFiles.append(url);
                completeTargetFiles.append(url);
            }
        }

        deleteFilesCount++;

        if (action == AbstractJobHandler::SupportAction::kSkipAction) {
            fmInfo() << "Skipped deleting file:" << url;
            continue;
        }

        if (action != AbstractJobHandler::SupportAction::kNoAction)
            return false;

        emit fileDeleted(url);
    }

    fmInfo() << "Completed deletion on non-removable device - deleted count:" << deleteFilesCount;
    return true;
}
/*!
 * \brief DoDeleteFilesWorker::deleteFilesOnOtherDevice Delete files on removable devices and other
 * \return delete file success
 */
bool DoDeleteFilesWorker::deleteFilesOnOtherDevice()
{
    fmDebug() << "Deleting files on other device - source count:" << sourceUrls.count();

    bool ok = true;
    if (sourceUrls.count() == 1 && isConvert) {
        auto info = InfoFactory::create<FileInfo>(sourceUrls.first(), Global::CreateFileInfoType::kCreateFileInfoSync);
        if (info) {
            deleteFirstFileSize = info->size();
            fmDebug() << "Single file deletion on other device - size:" << deleteFirstFileSize;
        }
    }

    for (auto &url : sourceUrls) {
        const auto &info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
        if (!info) {
            fmCritical() << "Failed to create file info for deletion - URL:" << url;
            // pause and emit error msg
            if (doHandleErrorAndWait(url, AbstractJobHandler::JobErrorType::kProrogramError) == AbstractJobHandler::SupportAction::kSkipAction) {
                fmInfo() << "Skipped file due to info creation failure:" << url;
                continue;
            }
            return false;
        }

        if (info->isAttributes(OptInfoType::kIsSymLink) || info->isAttributes(OptInfoType::kIsFile)) {
            fmDebug() << "Deleting file/symlink:" << url;
            ok = deleteFileOnOtherDevice(url);
        } else {
            fmDebug() << "Deleting directory:" << url;
            ok = deleteDirOnOtherDevice(info);
        }

        if (!ok) {
            fmWarning() << "Failed to delete item:" << url;
            return false;
        }

        completeTargetFiles.append(url);
        completeSourceFiles.append(url);
        emit fileDeleted(url);
        fmDebug() << "Successfully deleted item:" << url;
    }

    fmInfo() << "Completed deletion on other device - processed count:" << sourceUrls.count();
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
            fmWarning() << "Delete file failed on other device - file:" << url << "error:" << localFileHandler->errorString();
            action = doHandleErrorAndWait(url, AbstractJobHandler::JobErrorType::kDeleteFileError,
                                          localFileHandler->errorString());
        } else {
            FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType::kFileDeleted, url);
            fmDebug() << "Successfully deleted file on other device:" << url;
        }
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    deleteFilesCount++;

    if (action == AbstractJobHandler::SupportAction::kSkipAction) {
        fmInfo() << "Skipped deleting file on other device:" << url;
        return true;
    }

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

    fmDebug() << "Deleting directory recursively:" << dir->urlOf(UrlInfoType::kUrl);

    if (dir->countChildFile() < 0) {
        fmDebug() << "Directory has no children, treating as file:" << dir->urlOf(UrlInfoType::kUrl);
        return deleteFileOnOtherDevice(dir->urlOf(UrlInfoType::kUrl));
    }

    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    AbstractDirIteratorPointer iterator(nullptr);
    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        QString errorMsg;
        iterator = DirIteratorFactory::create<AbstractDirIterator>(dir->urlOf(UrlInfoType::kUrl), &errorMsg);
        if (!iterator) {
            fmWarning() << "Create directory iterator failed - dir:" << dir->urlOf(UrlInfoType::kUrl) << "error:" << errorMsg;
            action = doHandleErrorAndWait(dir->urlOf(UrlInfoType::kUrl), AbstractJobHandler::JobErrorType::kDeleteFileError, errorMsg);
        }
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    if (action == AbstractJobHandler::SupportAction::kSkipAction) {
        fmInfo() << "Skipped deleting directory:" << dir->urlOf(UrlInfoType::kUrl);
        return true;
    }
    if (action != AbstractJobHandler::SupportAction::kNoAction)
        return false;

    bool ok { true };
    int childCount = 0;
    while (iterator->hasNext()) {
        const QUrl &url = iterator->next();
        childCount++;

        const auto &info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
        if (!info) {
            fmCritical() << "Failed to create file info for directory child - URL:" << url;
            // pause and emit error msg
            if (doHandleErrorAndWait(url, AbstractJobHandler::JobErrorType::kProrogramError) == AbstractJobHandler::SupportAction::kSkipAction) {
                fmInfo() << "Skipped child due to info creation failure:" << url;
                continue;
            }
            return false;
        }

        if (info->isAttributes(OptInfoType::kIsSymLink) || info->isAttributes(OptInfoType::kIsFile)) {
            fmDebug() << "Deleting child file/symlink:" << url;
            ok = deleteFileOnOtherDevice(url);
        } else {
            fmDebug() << "Deleting child directory:" << url;
            ok = deleteDirOnOtherDevice(info);
        }

        if (!ok) {
            fmWarning() << "Failed to delete child item:" << url;
            return false;
        }
    }

    fmDebug() << "Deleted" << childCount << "children from directory:" << dir->urlOf(UrlInfoType::kUrl);

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
    fmWarning() << "Delete error - file:" << from << "error:" << static_cast<int>(error) << "message:" << errorMsg;

    setStat(AbstractJobHandler::JobState::kPauseState);
    emitErrorNotify(from, QUrl(), error, false, 0, errorMsg);

    {
        QMutexLocker locker(&mutex);
        waitCondition.wait(&mutex);
    }

    fmDebug() << "Error handling completed - action:" << static_cast<int>(currentAction);
    return currentAction;
}
