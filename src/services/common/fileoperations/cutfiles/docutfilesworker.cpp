/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "services/common/fileoperations/copyfiles/storageinfo.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/decorator/decoratorfileinfo.h"

#include "dfm-io/core/diofactory.h"

#include <QUrl>
#include <QProcess>
#include <QMutex>
#include <QStorageInfo>
#include <QWaitCondition>
#include <QQueue>
#include <QDebug>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

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
        doHandleErrorAndWait(QUrl(), QUrl(), QUrl(), AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }
    if (!targetUrl.isValid()) {
        // pause and emit error msg
        doHandleErrorAndWait(sourceUrls.first(), targetUrl, QUrl(), AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }
    targetInfo = InfoFactory::create<AbstractFileInfo>(targetUrl);
    if (!targetInfo) {
        // pause and emit error msg
        doHandleErrorAndWait(sourceUrls.first(), targetUrl, QUrl(), AbstractJobHandler::JobErrorType::kProrogramError);
        return false;
    }

    if (!targetInfo->exists()) {
        // pause and emit error msg
        doHandleErrorAndWait(sourceUrls.first(), targetUrl, QUrl(), AbstractJobHandler::JobErrorType::kNonexistenceError);
        return false;
    }

    targetStorageInfo.reset(new StorageInfo(targetUrl.path()));

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
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(url, targetUrl, QUrl(), AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                continue;
            }
        }
        fileInfo->refresh();

        const QUrl &urlFrom = fileInfo->url();
        const QString &fileName = fileInfo->fileName();

        QString newFileUrl = targetInfo->url().toString();
        if (!newFileUrl.endsWith("/"))
            newFileUrl.append("/");
        newFileUrl.append(fileName);
        DecoratorFileInfo newFileInfo(QUrl(newFileUrl, QUrl::TolerantMode));

        if (newFileInfo.url() == fileInfo->url()
            || (FileUtils::isSameFile(urlFrom, newFileInfo.url()) && !fileInfo->isSymLink())) {
            ++completedFilesCount;
            continue;
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
    bool ok = false;
    if (doRenameFile(fromInfo, toInfo, &ok) || ok) {
        return true;
    }

    bool result = false;
    // check space
    if (!checkDiskSpaceAvailable(fromInfo->url(), toInfo->url(), targetStorageInfo, &result))
        return result;

    if (!copyAndDeleteFile(fromInfo, toInfo, &result))
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

AbstractJobHandler::SupportActions DoCutFilesWorker::supportActions(const AbstractJobHandler::JobErrorType &error)
{
    AbstractJobHandler::SupportActions support = AbstractJobHandler::SupportAction::kCancelAction;
    switch (error) {
    case AbstractJobHandler::JobErrorType::kPermissionError:
    case AbstractJobHandler::JobErrorType::kOpenError:
    case AbstractJobHandler::JobErrorType::kReadError:
    case AbstractJobHandler::JobErrorType::kWriteError:
    case AbstractJobHandler::JobErrorType::kSymlinkError:
    case AbstractJobHandler::JobErrorType::kMkdirError:
    case AbstractJobHandler::JobErrorType::kResizeError:
    case AbstractJobHandler::JobErrorType::kRemoveError:
    case AbstractJobHandler::JobErrorType::kRenameError:
    case AbstractJobHandler::JobErrorType::kIntegrityCheckingError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kRetryAction;
    case AbstractJobHandler::JobErrorType::kSpecialFileError:
        return AbstractJobHandler::SupportAction::kSkipAction;
    case AbstractJobHandler::JobErrorType::kFileSizeTooBigError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kEnforceAction;
    case AbstractJobHandler::JobErrorType::kNotEnoughSpaceError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kRetryAction | AbstractJobHandler::SupportAction::kEnforceAction;
    case AbstractJobHandler::JobErrorType::kFileExistsError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kReplaceAction | AbstractJobHandler::SupportAction::kCoexistAction;
    case AbstractJobHandler::JobErrorType::kDirectoryExistsError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kMergeAction | AbstractJobHandler::SupportAction::kCoexistAction;
    case AbstractJobHandler::JobErrorType::kTargetReadOnlyError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kEnforceAction;
    case AbstractJobHandler::JobErrorType::kTargetIsSelfError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kEnforceAction;
    case AbstractJobHandler::JobErrorType::kSymlinkToGvfsError:
        return support | AbstractJobHandler::SupportAction::kSkipAction;
    default:
        break;
    }

    return support;
}

void DoCutFilesWorker::doOperateWork(AbstractJobHandler::SupportActions actions)
{
    AbstractWorker::doOperateWork(actions);
    resume();
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

bool DoCutFilesWorker::doRenameFile(const AbstractFileInfoPointer &sourceInfo, const AbstractFileInfoPointer &targetInfo, bool *ok)
{
    QSharedPointer<QStorageInfo> sourceStorageInfo = nullptr;
    sourceStorageInfo.reset(new QStorageInfo(sourceInfo->url().path()));

    const QUrl &sourceUrl = sourceInfo->url();
    const QUrl &targetUrl = targetInfo->url();

    if (sourceStorageInfo->device() == targetStorageInfo->device()) {
        AbstractFileInfoPointer newTargetInfo(nullptr);
        if (!doCheckFile(sourceInfo, targetInfo, newTargetInfo, ok))
            return *ok;

        if (sourceInfo->isSymLink()) {
            if (newTargetInfo->exists()) {
                if (!isConvert && targetInfo == this->targetInfo) {
                    completeFiles.append(sourceUrl);
                    completeTargetFiles.append(newTargetInfo->url());
                }
                bool succ = deleteFile(sourceUrl, ok);
                if (!succ) {
                    return *ok;
                }
            }

            // create link

            AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
            do {
                if (!handler->createSystemLink(sourceUrl, newTargetInfo->url()))
                    // pause and emit error msg
                    action = doHandleErrorAndWait(sourceUrl, targetUrl, QUrl(), AbstractJobHandler::JobErrorType::kSymlinkError);

            } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

            if (action != AbstractJobHandler::SupportAction::kNoAction) {
                return action == AbstractJobHandler::SupportAction::kSkipAction;
            }

            if (!isConvert && targetInfo == this->targetInfo) {
                completeFiles.append(sourceUrl);
                completeTargetFiles.append(newTargetInfo->url());
            }
            // remove old link file
            if (!deleteFile(sourceUrl, ok))
                return *ok;

            return true;

        } else {
            *ok = renameFileByHandler(sourceInfo, newTargetInfo);
            return *ok;
        }
    }

    return false;
}
