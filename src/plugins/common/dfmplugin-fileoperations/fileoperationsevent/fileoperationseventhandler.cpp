// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
    fmInfo() << "FileOperationsEventHandler initialized";
}

void FileOperationsEventHandler::publishJobResultEvent(AbstractJobHandler::JobType jobType,
                                                       const QList<QUrl> &srcUrls,
                                                       const QList<QUrl> &destUrls, const QVariantList &customInfos,
                                                       bool ok,
                                                       const QString &errMsg)
{
    fmInfo() << "Publishing job result event: type=" << static_cast<int>(jobType) 
             << "srcCount=" << srcUrls.count() 
             << "destCount=" << destUrls.count() 
             << "success=" << ok;
    
    if (!ok && !errMsg.isEmpty()) {
        fmWarning() << "Job completed with error:" << errMsg;
    }

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
        fmWarning() << "Publishing job result for unknown job type:" << static_cast<int>(jobType);
    }
}

void FileOperationsEventHandler::removeUrlsInClipboard(AbstractJobHandler::JobType jobType, const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok)
{
    if (!ok) {
        fmInfo() << "Skipping clipboard cleanup due to job failure";
        return;
    }

    int urlsToRemove = 0;
    switch (jobType) {
    case AbstractJobHandler::JobType::kDeleteType:
    case AbstractJobHandler::JobType::kMoveToTrashType:
        urlsToRemove = srcUrls.count();
        ClipBoard::instance()->removeUrls(srcUrls);
        break;
    case AbstractJobHandler::JobType::kCleanTrashType:
        urlsToRemove = destUrls.count();
        ClipBoard::instance()->removeUrls(destUrls);
        break;
    case AbstractJobHandler::JobType::kCutType:
        urlsToRemove = srcUrls.count();
        ClipBoard::instance()->removeUrls(srcUrls);
        break;
    default:
        return;
    }
    
    if (urlsToRemove > 0) {
        fmInfo() << "Removed" << urlsToRemove << "URLs from clipboard for job type:" << static_cast<int>(jobType);
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
        fmCritical() << "Cannot handle job result: invalid job type=" << static_cast<int>(jobType) << "or null pointer";
        return;
    }

    fmInfo() << "Setting up job result handling for job type:" << static_cast<int>(jobType);

    QSharedPointer<bool> ok { new bool { true } };
    QSharedPointer<QString> errMsg { new QString };
    connect(ptr.get(), &AbstractJobHandler::errorNotify, this, &FileOperationsEventHandler::handleErrorNotify);
    connect(ptr.get(), &AbstractJobHandler::finishedNotify, this, &FileOperationsEventHandler::handleFinishedNotify);
}

void FileOperationsEventHandler::handleErrorNotify(const JobInfoPointer &jobInfo)
{
    if (!jobInfo) {
        fmWarning() << "Received null job info in error notification";
        return;
    }

    QSharedPointer<bool> ok { new bool { true } };
    QSharedPointer<QString> errMsg { new QString };
    auto errType { jobInfo->value(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey).value<AbstractJobHandler::JobErrorType>() };
    if (errType != AbstractJobHandler::JobErrorType::kNoError) {
        *ok = false;
        *errMsg = jobInfo->value(AbstractJobHandler::NotifyInfoKey::kErrorMsgKey).toString();
        fmWarning() << "Job error notification received: type=" << static_cast<int>(errType) << "message:" << *errMsg;
    }
}

void FileOperationsEventHandler::handleFinishedNotify(const JobInfoPointer &jobInfo)
{
    if (!jobInfo) {
        fmWarning() << "Received null job info in finished notification";
        return;
    }
    
    if (!jobInfo->contains(AbstractJobHandler::NotifyInfoKey::kJobtypeKey)) {
        fmWarning() << "Job finished notification missing job type key";
        return;
    }
    
    QSharedPointer<bool> ok { new bool { true } };
    QSharedPointer<QString> errMsg { new QString };
    auto srcUrls { jobInfo->value(AbstractJobHandler::NotifyInfoKey::kCompleteFilesKey).value<QList<QUrl>>() };
    auto destUrls { jobInfo->value(AbstractJobHandler::NotifyInfoKey::kCompleteTargetFilesKey).value<QList<QUrl>>() };
    auto customInfos = jobInfo->value(AbstractJobHandler::NotifyInfoKey::kCompleteCustomInfosKey).toList();
    auto jobType = jobInfo->value(AbstractJobHandler::NotifyInfoKey::kJobtypeKey).value<DFMBASE_NAMESPACE::AbstractJobHandler::JobType>();
    
    fmInfo() << "Job finished notification: type=" << static_cast<int>(jobType) 
             << "srcCount=" << srcUrls.count() 
             << "destCount=" << destUrls.count() 
             << "customInfoCount=" << customInfos.count();
    
    publishJobResultEvent(jobType, srcUrls, destUrls, customInfos, *ok, *errMsg);
    removeUrlsInClipboard(jobType, srcUrls, destUrls, *ok);
}
