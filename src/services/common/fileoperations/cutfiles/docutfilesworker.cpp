/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<liyigang@uniontech.com>
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

#include "docutfilesworker.h"
#include "fileoperations/fileoperationutils/fileoperationsutils.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"

#include "dfm-io/core/diofactory.h"

#include <QUrl>
#include <QProcess>
#include <QMutex>
#include <QStorageInfo>
#include <QWaitCondition>
#include <QQueue>
#include <QDebug>

#include <fcntl.h>

DSC_USE_NAMESPACE

DoCutFilesWorker::DoCutFilesWorker(QObject *parent)
    : FileOperateBaseWorker(parent)
{
    jobType = AbstractJobHandler::JobType::kCutType;
}

DoCutFilesWorker::~DoCutFilesWorker()
{
    stop();
}

bool DoCutFilesWorker::doWork()
{
    // The endcopy interface function has been called here
    if (!AbstractWorker::doWork())
        return false;

    // 执行剪切
    if (!cutFiles()) {
        endWork();
        return false;
    }

    // 完成
    endWork();

    return true;
}

void DoCutFilesWorker::stop()
{
    AbstractWorker::stop();
}

bool DoCutFilesWorker::initArgs()
{
    AbstractWorker::initArgs();

    if (sourceUrls.count() <= 0) {
        // pause and emit error msg
        doHandleErrorAndWait(QUrl(), QUrl(), AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }
    if (!targetUrl.isValid()) {
        // pause and emit error msg
        doHandleErrorAndWait(sourceUrls.first(), targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }
    targetInfo = InfoFactory::create<AbstractFileInfo>(targetUrl);
    if (!targetInfo) {
        // pause and emit error msg
        doHandleErrorAndWait(sourceUrls.first(), targetUrl, AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }

    if (!targetInfo->exists()) {
        // pause and emit error msg
        doHandleErrorAndWait(sourceUrls.first(), targetUrl, AbstractJobHandler::JobErrorType::kNonexistenceError);
        return false;
    }

    targetStorageInfo.reset(new QStorageInfo(targetUrl.path()));

    return true;
}

bool DoCutFilesWorker::cutFiles()
{
    for (const auto &url : sourceUrls) {
        if (!stateCheck()) {
            return false;
        }
        const auto &fileInfo = InfoFactory::create<AbstractFileInfo>(url);
        if (!fileInfo) {
            // pause and emit error msg
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(url, targetUrl, AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                continue;
            }
        }
        if (!doCutFile(fileInfo, targetInfo)) {
            return false;
        }
        ++completedFilesCount;
    }
    return true;
}

bool DoCutFilesWorker::doCutFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo)
{
    // try rename
    if (doRenameFile(fromInfo, toInfo))
        return true;

    bool result = false;
    // check space
    if (!checkDiskSpaceAvailable(fromInfo->url(), toInfo->url(), targetStorageInfo, &result))
        return result;

    if (!doCopyFile(fromInfo, toInfo, &result))
        return result;

    return true;
}

void DoCutFilesWorker::onUpdateProccess()
{
    if (totalMoveFilesCount > 0) {
        qreal realProgress = qreal(completedFilesCount) / totalMoveFilesCount;
        if (realProgress > lastProgress)
            lastProgress = realProgress;
    } else {
        if (completedFilesCount < totalMoveFilesCount && totalMoveFilesCount > 0) {
            qreal fuzzyProgress = qreal(completedFilesCount) / totalMoveFilesCount;
            if (fuzzyProgress < 0.5 && fuzzyProgress > lastProgress)
                lastProgress = fuzzyProgress;
        }
    }
    // 保证至少出现%1
    if (lastProgress < 0.02) {
        lastProgress = 0.01;
    }
    // TODO(lanxs) emit signal to update progress
    //Q_EMIT progressChanged(qMin(lastProgress, 1.0), 0);
}

void DoCutFilesWorker::emitCompleteFilesUpdatedNotify(const qint64 &writCount)
{
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kCompleteFilesKey, QVariant::fromValue(writCount));

    emit stateChangedNotify(info);
}

AbstractJobHandler::SupportAction DoCutFilesWorker::doHandleErrorAndWait(const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error, const QString &errorMsg)
{
    setStat(AbstractJobHandler::JobState::kPauseState);
    emitErrorNotify(from, to, error, errorMsg);

    if (!handlingErrorQMutex)
        handlingErrorQMutex.reset(new QMutex);

    handlingErrorQMutex->lock();
    if (handlingErrorCondition.isNull())
        handlingErrorCondition.reset(new QWaitCondition);
    handlingErrorCondition->wait(handlingErrorQMutex.data());
    handlingErrorQMutex->unlock();

    return currentAction;
}

bool DoCutFilesWorker::renameFileByHandler(const AbstractFileInfoPointer &sourceInfo, const AbstractFileInfoPointer &targetInfo)
{
    if (handler) {
        const QUrl &sourceUrl = sourceInfo->url();
        const QUrl &targetUrl = targetInfo->url();
        return handler->renameFile(sourceUrl, targetUrl);
    }
    return false;
}

bool DoCutFilesWorker::doRenameFile(const AbstractFileInfoPointer &sourceInfo, const AbstractFileInfoPointer &targetInfo)
{
    QSharedPointer<QStorageInfo> sourceStorageInfo = nullptr;
    sourceStorageInfo.reset(new QStorageInfo(sourceInfo->url().path()));

    const QUrl &sourceUrl = sourceInfo->url();
    const QUrl &targetUrl = targetInfo->url();

    if (sourceStorageInfo->device() == targetStorageInfo->device()) {
        bool result = false;
        AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
        AbstractFileInfoPointer newTargetInfo(nullptr);
        if (!doCheckFile(sourceInfo, targetInfo, newTargetInfo, &result))
            return result;

        if (sourceInfo->isSymLink()) {
            // TODO(lanxs)

            if (targetInfo->exists()) {
                bool succ = deleteFile(sourceUrl, targetUrl, targetInfo, &result);
                if (!succ) {
                    return result;
                }
            }

            // create link

            do {
                if (!handler->createSystemLink(sourceUrl, newTargetInfo->url()))
                    // pause and emit error msg
                    action = doHandleErrorAndWait(sourceUrl, targetUrl, AbstractJobHandler::JobErrorType::kSymlinkError);

            } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

            if (action != AbstractJobHandler::SupportAction::kNoAction) {
                return action == AbstractJobHandler::SupportAction::kSkipAction;
            }

            // remove old link file
            if (!deleteFile(sourceUrl, targetUrl, sourceInfo, &result))
                return result;

            return true;

        } else {
            if (!renameFileByHandler(sourceInfo, newTargetInfo))
                // pause and emit error msg
                return false;

            // TODO(lanxs) need update progress
            if (Q_UNLIKELY(!stateCheck())) {
                return false;
            }
            return true;
        }
    }

    return false;
}
