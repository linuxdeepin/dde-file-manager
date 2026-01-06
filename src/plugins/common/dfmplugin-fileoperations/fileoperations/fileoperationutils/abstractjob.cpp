// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractjob.h"
#include "abstractworker.h"

#include <QUrl>
#include <QCoreApplication>

DPFILEOPERATIONS_USE_NAMESPACE
/*!
 * \brief AbstractJob::setJobArgs 设置任务的参数
 * \param handle 任务处理控制器
 * \param sources 源文件列表
 * \param target 目标文件
 * \param flags 任务的标志
 */
void AbstractJob::setJobArgs(const JobHandlePointer handle, const QList<QUrl> &sources, const QUrl &target,
                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags)
{
    if (!handle) {
        fmCritical() << "Job handle pointer is null, cannot set job arguments";
        return;
    }
    
    fmInfo() << "Setting job arguments - sources count:" << sources.count() << "target:" << target << "flags:" << static_cast<int>(flags);
    
    connect(handle.get(), &AbstractJobHandler::userAction, this, &AbstractJob::operateAation);
    connect(this, &AbstractJob::requestShowTipsDialog, handle.get(), &AbstractJobHandler::requestShowTipsDialog);
    connect(this, &AbstractJob::requestShowFailedDialog, handle.get(), &AbstractJobHandler::requestShowFailedDialog);

    // 连接worker中的所有错误信号，转发给jobhandler
    connect(doWorker.data(), &AbstractWorker::errorNotify, this, &AbstractJob::handleError, Qt::QueuedConnection);
    connect(this, &AbstractJob::errorNotify, handle.get(), &AbstractJobHandler::onError);
    connect(doWorker.data(), &AbstractWorker::workerFinish, handle.get(), &AbstractJobHandler::workerFinish, Qt::QueuedConnection);
    connect(doWorker.data(), &AbstractWorker::requestSaveRedoOperation, handle.get(), &AbstractJobHandler::requestSaveRedoOperation, Qt::QueuedConnection);
    doWorker->setWorkArgs(handle, sources, target, flags);
}

/*!
 * \brief AbstractJob::start 启动任务线程
 */
void AbstractJob::start()
{
    fmInfo() << "Starting job thread";
    thread.start();
}

AbstractJob::AbstractJob(AbstractWorker *doWorker, QObject *parent)
    : QObject(parent), doWorker(doWorker)
{
    if (this->doWorker) {
        this->doWorker->moveToThread(&thread);
        connect(doWorker, &AbstractWorker::workerFinish, this, &AbstractJob::deleteLater);
        connect(doWorker, &AbstractWorker::requestShowTipsDialog, this, &AbstractJob::requestShowTipsDialog);
        connect(doWorker, &AbstractWorker::requestShowFailedDialog, this, &AbstractJob::requestShowFailedDialog);
        connect(doWorker, &AbstractWorker::retryErrSuccess, this, &AbstractJob::handleRetryErrorSuccess, Qt::QueuedConnection);
        connect(doWorker, &AbstractWorker::fileAdded, this, &AbstractJob::handleFileAdded, Qt::QueuedConnection);
        connect(doWorker, &AbstractWorker::fileDeleted, this, &AbstractJob::handleFileDeleted, Qt::QueuedConnection);
        connect(doWorker, &AbstractWorker::fileRenamed, this, &AbstractJob::handleFileRenamed, Qt::QueuedConnection);
        connect(qApp, &QCoreApplication::aboutToQuit, this, [=]() {
            fmInfo() << "Application quitting, stopping job thread";
            thread.quit();
            // When manipulating a file,
            // if the TaskDialog has not been popped up yet,
            // immediately close dde-file-manage at this time,
            // here will cause the dde-file-manager process
            // to be blocked in the main thread,
            // and then can not open the new dde-file-manage process,
            // so here to add a timeout time.
            // see: bug-272373
            if (!thread.wait(3000)) {
                fmWarning() << "Job thread did not finish within timeout, forcing termination";
            }
        });
        start();
    } else {
        fmCritical() << "Worker is null, cannot create job";
    }
}

/*!
 * \brief operateCopy 处理handle上的用户操作
 * \param actions 操作类型
 */
void AbstractJob::operateAation(AbstractJobHandler::SupportActions actions)
{
    if (actions.testFlag(AbstractJobHandler::SupportAction::kStartAction)) {
        fmInfo() << "Starting work operation";
        emit doWorker->startWork();
    } else {
        if (actions.testFlag(AbstractJobHandler::SupportAction::kStopAction) || actions.testFlag(AbstractJobHandler::SupportAction::kCancelAction)) {
            fmInfo() << "Stopping/cancelling operation, clearing error queue";
            errorQueue.clear();
            return doWorker->stopAllThread();
        }

        // 处理当前的错误
        if (errorQueue.size() > 0) {
            auto isRetry = actions.testFlag(AbstractJobHandler::SupportAction::kRetryAction);
            auto error = errorQueue.head()->value(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey).value<AbstractJobHandler::JobErrorType>();
            auto id = errorQueue.head()->value(AbstractJobHandler::NotifyInfoKey::kWorkerPointer).value<quint64>();
            
            fmDebug() << "Processing error - type:" << static_cast<int>(error) << "worker ID:" << id << "retry:" << isRetry;
            
            if (!isRetry)
                errorQueue.dequeue();
            doWorker->doOperateWork(actions, error, id);
            // not retry,dealing next error
            if (!isRetry && errorQueue.size() > 0) {
                emit errorNotify(errorQueue.head());
            } else if (!actions.testFlag(AbstractJobHandler::SupportAction::kStopAction)
                       && !actions.testFlag(AbstractJobHandler::SupportAction::kPauseAction)
                       && !actions.testFlag(AbstractJobHandler::SupportAction::kResumAction)) {
                // no error thread resume
                QList<quint64> errorIds;
                for (const auto &info : errorQueue) {
                    errorIds.append(info->value(AbstractJobHandler::NotifyInfoKey::kWorkerPointer).value<quint64>());
                }
                doWorker->resumeThread(errorIds);
            }

        } else {
            doWorker->doOperateWork(actions);
        }
    }
}

void AbstractJob::handleError(const JobInfoPointer jobInfo)
{
    doWorker->pauseAllThread();
    // retry error
    if (errorQueue.size() > 0
        && errorQueue.head()->value(AbstractJobHandler::NotifyInfoKey::kWorkerPointer).value<quint64>()
                == jobInfo->value(AbstractJobHandler::NotifyInfoKey::kWorkerPointer).value<quint64>()) {

        if (errorQueue.head()->value(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey).value<AbstractJobHandler::JobErrorType>()
            != jobInfo->value(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey).value<AbstractJobHandler::JobErrorType>())
            errorQueue.push_front(jobInfo);

        emit errorNotify(jobInfo);
        return;
    }
    // new error
    errorQueue.enqueue(jobInfo);
    if (errorQueue.size() > 1) {
        fmDebug() << "Error queued, total errors in queue:" << errorQueue.size();
        return;
    }
    // 进行错误处理
    fmWarning() << "Handling new error, error type:" << jobInfo->value(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey).toInt();
    emit errorNotify(jobInfo);
}

void AbstractJob::handleRetryErrorSuccess(const quint64 Id)
{
    // retry error dealing success and dealing next error
    if (errorQueue.size() <= 0 || errorQueue.head()->value(AbstractJobHandler::NotifyInfoKey::kWorkerPointer).value<quint64>() != Id) {
        if (errorQueue.size() > 0 && errorQueue.head()->value(AbstractJobHandler::NotifyInfoKey::kWorkerPointer).value<quint64>() != Id)
            fmWarning() << "Error handling mismatch - current thread ID:" << Id << "expected ID:" << errorQueue.head()->value(AbstractJobHandler::NotifyInfoKey::kWorkerPointer).value<quint64>();
        return;
    }
    
    fmDebug() << "Retry error handling successful for worker ID:" << Id;
    errorQueue.dequeue();
    if (errorQueue.size() > 0) {
        fmDebug() << "Processing next error in queue, remaining errors:" << errorQueue.size();
        emit errorNotify(errorQueue.head());
    } else {
        fmDebug() << "All errors processed, resuming all threads";
        doWorker->resumeAllThread();
    }
}

void AbstractJob::handleFileRenamed(const QUrl &old, const QUrl &cur)
{
    fmDebug() << "File renamed from:" << old << "to:" << cur;
    dpfSignalDispatcher->publish("dfmplugin_fileoperations", "signal_File_Rename", old, cur);
}

void AbstractJob::handleFileDeleted(const QUrl &url)
{
    fmDebug() << "File deleted:" << url;
    dpfSignalDispatcher->publish("dfmplugin_fileoperations", "signal_File_Delete", url);
}

void AbstractJob::handleFileAdded(const QUrl &url)
{
    fmDebug() << "File added:" << url;
    dpfSignalDispatcher->publish("dfmplugin_fileoperations", "signal_File_Add", url);
}

AbstractJob::~AbstractJob()
{
    fmDebug() << "Destroying job, stopping thread";
    thread.quit();
    if (!thread.wait(5000)) {
        fmWarning() << "Job thread did not finish within timeout during destruction";
    }
}
