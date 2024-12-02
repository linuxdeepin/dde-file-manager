// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileoperationseventhandler.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/clipboard.h>

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
    case AbstractJobHandler::JobType::kDeleteType:
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
        fmWarning() << "Invalid Job Type";
    }
}

void FileOperationsEventHandler::removeUrlsInClipboard(AbstractJobHandler::JobType jobType, const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok)
{
    if (!ok)
        return;

    switch (jobType) {
    case AbstractJobHandler::JobType::kDeleteType:
    case AbstractJobHandler::JobType::kMoveToTrashType:
        ClipBoard::instance()->removeUrls(srcUrls);
        break;
    case AbstractJobHandler::JobType::kCleanTrashType:
        ClipBoard::instance()->removeUrls(destUrls);
        break;
    case AbstractJobHandler::JobType::kCutType:
        ClipBoard::instance()->removeUrls(srcUrls);
        break;
    default:
        break;
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
        fmCritical() << "Invalid job: " << jobType;
        return;
    }

    QSharedPointer<bool> ok { new bool { true } };
    QSharedPointer<QString> errMsg { new QString };
    connect(ptr.get(), &AbstractJobHandler::errorNotify, this, &FileOperationsEventHandler::handleErrorNotify);
    connect(ptr.get(), &AbstractJobHandler::finishedNotify, this, &FileOperationsEventHandler::handleFinishedNotify);
}

void FileOperationsEventHandler::handleErrorNotify(const JobInfoPointer &jobInfo)
{
    QSharedPointer<bool> ok { new bool { true } };
    QSharedPointer<QString> errMsg { new QString };
    auto errType { jobInfo->value(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey).value<AbstractJobHandler::JobErrorType>() };
    if (errType != AbstractJobHandler::JobErrorType::kNoError) {
        *ok = false;
        *errMsg = jobInfo->value(AbstractJobHandler::NotifyInfoKey::kErrorMsgKey).toString();
    }
}

void FileOperationsEventHandler::handleFinishedNotify(const JobInfoPointer &jobInfo)
{
    if (!jobInfo->contains(AbstractJobHandler::NotifyInfoKey::kJobtypeKey))
        return;
    QSharedPointer<bool> ok { new bool { true } };
    QSharedPointer<QString> errMsg { new QString };
    auto srcUrls { jobInfo->value(AbstractJobHandler::NotifyInfoKey::kCompleteFilesKey).value<QList<QUrl>>() };
    auto destUrls { jobInfo->value(AbstractJobHandler::NotifyInfoKey::kCompleteTargetFilesKey).value<QList<QUrl>>() };
    auto customInfos = jobInfo->value(AbstractJobHandler::NotifyInfoKey::kCompleteCustomInfosKey).toList();
    auto jobType = jobInfo->value(AbstractJobHandler::NotifyInfoKey::kJobtypeKey).value<DFMBASE_NAMESPACE::AbstractJobHandler::JobType>();
    publishJobResultEvent(jobType, srcUrls, destUrls, customInfos, *ok, *errMsg);
    removeUrlsInClipboard(jobType, srcUrls, destUrls, *ok);
}
