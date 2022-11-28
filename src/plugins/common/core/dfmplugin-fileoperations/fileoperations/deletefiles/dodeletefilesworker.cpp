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
#include "dodeletefilesworker.h"
#include "dfm-base/base/schemefactory.h"

#include <QUrl>
#include <QDebug>

DPFILEOPERATIONS_USE_NAMESPACE
DoDeleteFilesWorker::DoDeleteFilesWorker(QObject *parent)
    : AbstractWorker(parent)
{
    jobType = AbstractJobHandler::JobType::kDeleteTpye;
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
    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    for (QList<QUrl>::iterator it = --allFilesList.end(); it != --allFilesList.begin(); --it) {
        if (!stateCheck())
            return false;
        const QUrl &url = *it;
        emitCurrentTaskNotify(url, QUrl());
        do {
            if (!localFileHandler->deleteFile(url)) {
                action = doHandleErrorAndWait(url, AbstractJobHandler::JobErrorType::kDeleteFileError, localFileHandler->errorString());
            }
        } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

        if (sourceUrls.contains(url)) {
            if (action == AbstractJobHandler::SupportAction::kNoAction)
                completeSourceFiles.append(url);
            deleteFilesCount++;
        }

        if (action == AbstractJobHandler::SupportAction::kSkipAction)
            continue;

        if (action != AbstractJobHandler::SupportAction::kNoAction)
            return false;
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
    for (auto &url : sourceUrls) {
        const auto &info = InfoFactory::create<AbstractFileInfo>(url);
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

        deleteFilesCount++;

        if (!ok)
            return false;

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
        if (!localFileHandler->deleteFile(url)) {
            action = doHandleErrorAndWait(url, AbstractJobHandler::JobErrorType::kDeleteFileError, localFileHandler->errorString());
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
bool DoDeleteFilesWorker::deleteDirOnOtherDevice(const AbstractFileInfoPointer &dir)
{
    if (!stateCheck())
        return false;

    if (dir->countChildFile() < 0)
        return deleteFileOnOtherDevice(dir->urlOf(UrlInfoType::kUrl));

    AbstractJobHandler::SupportAction action { AbstractJobHandler::SupportAction::kNoAction };
    AbstractDirIteratorPointer iterator(nullptr);
    do {
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

        const auto &info = iterator->fileInfo();
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
AbstractJobHandler::SupportAction DoDeleteFilesWorker::doHandleErrorAndWait(const QUrl &from, const AbstractJobHandler::JobErrorType &error, const QString &errorMsg)
{
    setStat(AbstractJobHandler::JobState::kPauseState);
    emitErrorNotify(from, QUrl(), error, 0, errorMsg);

    waitCondition.wait(&mutex);

    return currentAction;
}
