// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dodeletefilesworker.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/finallyutil.h>

#include <QUrl>

#include <unistd.h>
#include <fts.h>
// for DFMIO::DFMUtils::isInvalidCodecByPath used in originPath:: encoding handling
#include <dfm-io/dfmio_utils.h>

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
    bool success = deleteAllFiles();

    // 完成
    fmInfo() << "Delete operation completed successfully";
    endWork();

    return success;
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
    useFts = FileOperationsUtils::useFtsDelete();
    if (useFts) {
        // FTS only handles local files; if any source is non-local, fall back to the
        // per-device paths so remote files are actually deleted (not silently dropped).
        bool allLocal = true;
        for (const auto &url : sourceUrls) {
            if (!url.isLocalFile()) {
                allLocal = false;
                break;
            }
        }
        if (allLocal)
            return deleteFilesByFts();
    }
    if (isSourceFileLocal) {
        return deleteFilesOnCanNotRemoveDevice();
    }
    return deleteFilesOnOtherDevice();
}

bool DoDeleteFilesWorker::deleteFilesByFts()
{
    if (sourceUrls.isEmpty())
        return false;

    QList<QByteArray> pathData;
    pathData.reserve(sourceUrls.size());
    for (const auto &url : sourceUrls) {
        if (!url.isLocalFile()) {
            fmWarning() << "deleteFilesByFts: skip non-local path:" << url;
            continue;
        }
        pathData.append(url.toLocalFile().toUtf8());
    }
    if (pathData.isEmpty())
        return false;

    QVector<char *> pathPtrs(pathData.size() + 1, nullptr);
    for (int i = 0; i < pathData.size(); ++i)
        pathPtrs[i] = pathData[i].data();

    FTS *fts = fts_open(pathPtrs.data(), FTS_PHYSICAL | FTS_NOSTAT | FTS_NOCHDIR, nullptr);
    if (!fts) {
        fmWarning() << "deleteFilesByFts: fts_open failed:" << strerror(errno);
        return false;
    }
    // Flush any buffered fileDeleted signals on every exit path.
    dfmbase::FinallyUtil atFinish([&] { flushFileDeletedBatch(); });

    // FTS only traverses local files (non-local sources are filtered above), so the
    // local inotify watcher already delivers delete notifications - no manual notify
    // is needed here (mirrors deleteFilesOnCanNotRemoveDevice's local behavior).
    QSet<QUrl> sourceUrlsSet(sourceUrls.begin(), sourceUrls.end());
    bool success = true;
    int errorCount = 0;

    FTSENT *ent;
    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    errno = 0;
    for (ent = fts_read(fts); ent != nullptr; errno = 0, ent = fts_read(fts)) {
        if (!stateCheck()) {
            success = false;
            break;
        }

        bool isDir = false;
        bool shouldDelete = false;

        switch (ent->fts_info) {
        case FTS_F:
        case FTS_SL:
        case FTS_SLNONE:
        case FTS_NSOK:
            shouldDelete = true;
            isDir = false;
            break;
        case FTS_DP:
            shouldDelete = true;
            isDir = true;
            break;
        case FTS_DNR:
        case FTS_NS:
        case FTS_ERR:
        case FTS_DC:
            fmWarning() << "deleteFilesByFts: traversal error:" << ent->fts_path
                       << strerror(ent->fts_errno);
            errorCount++;
            continue;
        default:
            continue;
        }

        if (!shouldDelete)
            continue;

        auto url = QUrl::fromLocalFile(ent->fts_path);
        // Match the encoding used by statisticsFilesSize() so that non-UTF8 / invalid
        // codec paths produce the same originPath::-prefixed URL, keeping sourceUrlsSet
        // lookups (completeSourceFiles / completeTargetFiles) consistent.
        if (DFMIO::DFMUtils::isInvalidCodecByPath(ent->fts_path))
            url.setUserInfo(QString::fromLatin1("originPath::") + QString::fromLatin1(ent->fts_path));
        emitCurrentTaskNotify(url, QUrl());
        // Only emit/count an entry when it was actually removed; a retry that is
        // interrupted by stop, or a non-retried failure, must not signal deletion.
        bool deleted = false;
        do {
            action = AbstractJobHandler::SupportAction::kNoAction;
            int ret = isDir ? ::rmdir(ent->fts_accpath) : ::unlink(ent->fts_accpath);
            if (ret == 0) {
                deleted = true;
                break;
            }
            // Already gone - nothing left to remove; treat as deleted.
            if (errno == ENOENT || errno == ENOTDIR) {
                deleted = true;
                break;
            }
            fmWarning() << "deleteFilesByFts: delete failed:" << ent->fts_path
                       << strerror(errno);
            errorCount++;
            action = doHandleErrorAndWait(url, AbstractJobHandler::JobErrorType::kDeleteFileError,
                                          strerror(errno));
        } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

        // Stopped mid-operation: do not emit/count the in-flight entry; abort traversal.
        if (isStopped()) {
            success = false;
            break;
        }
        // Not actually deleted (failed/skipped): no signal, no count.
        if (!deleted)
            continue;

        batchEmitFileDeleted(url);

        if (sourceUrlsSet.contains(url)) {
            completeSourceFiles.append(url);
            completeTargetFiles.append(url);
        }

        deleteFilesCount++;
    }

    if (errno != 0) {
        fmWarning() << "deleteFilesByFts: fts_read interrupted:" << strerror(errno);
        success = false;
    }

    fts_close(fts);

    fmInfo() << "deleteFilesByFts: done -" << errorCount << "errors," << deleteFilesCount << "deleted";

    return success && errorCount == 0;
}

void DoDeleteFilesWorker::flushFileDeletedBatch()
{
    if (!fileDeletedBuffer.isEmpty()) {
        emit fileDeleted(fileDeletedBuffer);
        fileDeletedBuffer.clear();
    }
}

void DoDeleteFilesWorker::batchEmitFileDeleted(const QUrl &url)
{
    if (fileDeletedBuffer.isEmpty())
        fileDeletedTimer.start();

    fileDeletedBuffer.append(url);

    if (fileDeletedBuffer.size() >= 1000 || fileDeletedTimer.hasExpired(500))
        flushFileDeletedBatch();
}
/*!
 * \brief DoDeleteFilesWorker::deleteFilesOnCanNotRemoveDevice Delete files on non removable devices
 * \return delete file success
 */
bool DoDeleteFilesWorker::deleteFilesOnCanNotRemoveDevice()
{
    fmDebug() << "Deleting files on non-removable device - file count:" << allFilesList.count();
    // Flush any buffered fileDeleted signals on every exit path.
    dfmbase::FinallyUtil atFinish([&] { flushFileDeletedBatch(); });

    if (allFilesList.count() == 1 && isConvert) {
        auto info = InfoFactory::create<FileInfo>(allFilesList.first(), Global::CreateFileInfoType::kCreateFileInfoSync);
        if (info) {
            deleteFirstFileSize = info->size();
            fmDebug() << "Single file deletion, size:" << deleteFirstFileSize;
        }
    }

    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    for (QList<QUrl>::iterator it = --allFilesList.end(); it != --allFilesList.begin(); --it) {
        if (!stateCheck()) {
            return false;
        }
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

        if (action != AbstractJobHandler::SupportAction::kNoAction) {
            return false;
        }

        batchEmitFileDeleted(url);
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
    // Flush any buffered fileDeleted signals on every exit path.
    dfmbase::FinallyUtil atFinish([&] { flushFileDeletedBatch(); });

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
        batchEmitFileDeleted(url);
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

    const QUrl dirUrl = dir->urlOf(UrlInfoType::kUrl);
    fmDebug() << "Deleting directory recursively:" << dirUrl;

    if (dir->countChildFile() < 0) {
        fmDebug() << "Directory has no children, treating as file:" << dirUrl;
        return deleteFileOnOtherDevice(dirUrl);
    }

    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    AbstractDirIteratorPointer iterator(nullptr);
    do {
        action = AbstractJobHandler::SupportAction::kNoAction;
        QString errorMsg;
        iterator = DirIteratorFactory::create<AbstractDirIterator>(dirUrl, &errorMsg);
        if (!iterator) {
            fmWarning() << "Create directory iterator failed - dir:" << dirUrl << "error:" << errorMsg;
            action = doHandleErrorAndWait(dirUrl, AbstractJobHandler::JobErrorType::kDeleteFileError, errorMsg);
        }
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    if (action == AbstractJobHandler::SupportAction::kSkipAction) {
        fmInfo() << "Skipped deleting directory:" << dirUrl;
        return true;
    }
    if (action != AbstractJobHandler::SupportAction::kNoAction)
        return false;

    QList<QUrl> childUrls;
    const qint64 expectedChildren = dir->countChildFile();
    if (expectedChildren > 0)
        childUrls.reserve(static_cast<int>(expectedChildren));

    fmDebug() << "Taking child snapshot before delete:" << dirUrl << "expected child count:" << expectedChildren;
    while (iterator->hasNext()) {
        if (!stateCheck())
            return false;

        const QUrl url = iterator->next();
        if (!url.isValid()) {
            const QString errorMsg = QStringLiteral("Directory iterator returned an invalid child while deleting directory");
            fmWarning() << "Invalid child returned by directory iterator - dir:" << dirUrl << "collected children:" << childUrls.count();
            action = doHandleErrorAndWait(dirUrl, AbstractJobHandler::JobErrorType::kDeleteFileError, errorMsg);
            if (action == AbstractJobHandler::SupportAction::kRetryAction)
                return deleteDirOnOtherDevice(dir);
            if (action == AbstractJobHandler::SupportAction::kSkipAction)
                return true;
            return false;
        }

        childUrls.append(url);
    }

    fmDebug() << "Directory child snapshot completed:" << dirUrl << "snapshot count:" << childUrls.count();

    bool ok { true };
    int childCount = 0;
    for (const QUrl &url : childUrls) {
        if (!stateCheck())
            return false;

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

    fmDebug() << "Deleted" << childCount << "children from directory:" << dirUrl;

    // delete self dir
    return deleteFileOnOtherDevice(dirUrl);
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
