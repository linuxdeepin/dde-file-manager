/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
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
#include "fileoperationseventhandler.h"

#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/event/event.h>

#include <QUrl>

DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

FileOperationsEventHandler::FileOperationsEventHandler(QObject *parent)
    : QObject(parent)
{
}

void FileOperationsEventHandler::publishJobResultEvent(AbstractJobHandler::JobType jobType,
                                                       const QList<QUrl> &srcUrls,
                                                       const QList<QUrl> &destUrls, const QVariantList &customInfos,
                                                       bool ok,
                                                       const QString &errMsg)
{
    switch (jobType) {
    case AbstractJobHandler::JobType::kCopyType:
        dpfSignalDispatcher->publish(GlobalEventType::kCopyResult, srcUrls, destUrls, ok, errMsg);
        break;
    case AbstractJobHandler::JobType::kCutType:
        dpfSignalDispatcher->publish(GlobalEventType::kCutFileResult, srcUrls, destUrls, ok, errMsg);
        break;
    case AbstractJobHandler::JobType::kDeleteTpye:
        dpfSignalDispatcher->publish(GlobalEventType::kDeleteFilesResult, srcUrls, ok, errMsg);
        break;
    case AbstractJobHandler::JobType::kMoveToTrashType:
        dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrashResult, srcUrls, ok, errMsg);
        break;
    case AbstractJobHandler::JobType::kRestoreType:
        dpfSignalDispatcher->publish(GlobalEventType::kRestoreFromTrashResult, srcUrls, destUrls, customInfos, ok, errMsg);
        break;
    case AbstractJobHandler::JobType::kCleanTrashType:
        dpfSignalDispatcher->publish(GlobalEventType::kCleanTrashResult, destUrls, ok, errMsg);
        break;
    default:
        qWarning() << "Invalid Job Type";
    }
}

FileOperationsEventHandler *FileOperationsEventHandler::instance()
{
    static FileOperationsEventHandler instance;
    return &instance;
}

void FileOperationsEventHandler::handleJobResult(DFMBASE_NAMESPACE::AbstractJobHandler::JobType jobType, JobHandlePointer ptr)
{
    if (!ptr || jobType == AbstractJobHandler::JobType::kUnknow) {
        qCritical() << "Invalid job: " << jobType;
        return;
    }

    QSharedPointer<bool> ok { new bool { true } };
    QSharedPointer<QString> errMsg { new QString };
    connect(ptr.data(), &AbstractJobHandler::errorNotify, this, [ok, errMsg](const JobInfoPointer &jobInfo) {
        auto errType { jobInfo->value(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey).value<AbstractJobHandler::JobErrorType>() };
        if (errType != AbstractJobHandler::JobErrorType::kNoError) {
            *ok = false;
            *errMsg = jobInfo->value(AbstractJobHandler::NotifyInfoKey::kErrorMsgKey).toString();
        }
    });

    connect(ptr.data(), &AbstractJobHandler::finishedNotify, this, [this, jobType, ok, errMsg](const JobInfoPointer &jobInfo) {
        auto srcUrls { jobInfo->value(AbstractJobHandler::NotifyInfoKey::kCompleteFilesKey).value<QList<QUrl>>() };
        auto destUrls { jobInfo->value(AbstractJobHandler::NotifyInfoKey::kCompleteTargetFilesKey).value<QList<QUrl>>() };
        auto customInfos = jobInfo->value(AbstractJobHandler::NotifyInfoKey::kCompleteCustomInfosKey).toList();
        publishJobResultEvent(jobType, srcUrls, destUrls, customInfos, *ok, *errMsg);
    });
}
