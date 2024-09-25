// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractjob.h"
#include "abstractworker.h"

#include <QDebug>
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
        fmWarning() << "JobHandlePointer is a nullptr, setJobArgs failed!";
        return;
    }
    connect(handle.get(), &AbstractJobHandler::userAction, this, &AbstractJob::operateAation);
    connect(this, &AbstractJob::requestShowTipsDialog, handle.get(), &AbstractJobHandler::requestShowTipsDialog);

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
    thread.start();
}

AbstractJob::AbstractJob(AbstractWorker *doWorker, QObject *parent)
    : QObject(parent), doWorker(doWorker)
{
    if (this->doWorker) {
        this->doWorker->moveToThread(&thread);
        connect(doWorker, &AbstractWorker::workerFinish, this, &AbstractJob::deleteLater);
        connect(doWorker, &AbstractWorker::requestShowTipsDialog, this, &AbstractJob::requestShowTipsDialog);
        connect(doWorker, &AbstractWorker::retryErrSuccess, this, &AbstractJob::handleRetryErrorSuccess, Qt::QueuedConnection);
        connect(qApp, &QCoreApplication::aboutToQuit, this, [=]() {
            thread.quit();
            // When manipulating a file,
            // if the TaskDialog has not been popped up yet,
            // immediately close dde-file-manage at this time,
            // here will cause the dde-file-manager process
            // to be blocked in the main thread,
            // and then can not open the new dde-file-manage process,
            // so here to add a timeout time.
            // see: bug-272373
            thread.wait(3000);
        });
        start();
    }
}

/*!
 * \brief operateCopy 处理handle上的用户操作
 * \param actions 操作类型
 */
void AbstractJob::operateAation(AbstractJobHandler::SupportActions actions)
{
    if (actions.testFlag(AbstractJobHandler::SupportAction::kStartAction)) {
        emit doWorker->startWork();
    } else {
        if (actions.testFlag(AbstractJobHandler::SupportAction::kStopAction) || actions.testFlag(AbstractJobHandler::SupportAction::kCancelAction)) {
            errorQueue.clear();
            return doWorker->stopAllThread();
        }

        // 处理当前的错误
        if (errorQueue.size() > 0) {
            auto isRetry = actions.testFlag(AbstractJobHandler::SupportAction::kRetryAction);
            auto error = errorQueue.head()->value(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey).value<AbstractJobHandler::JobErrorType>();
            auto id = errorQueue.head()->value(AbstractJobHandler::NotifyInfoKey::kWorkerPointer).value<quint64>();
            if (!isRetry)
                errorQueue.dequeue();
            doWorker->doOperateWork(actions, error, id);
            // not retry,dealing next error
            if (!isRetry && errorQueue.size() > 0) {
                emit errorNotify(errorQueue.head());
            } else if (!actions.testFlag(AbstractJobHandler::SupportAction::kStopAction)
                       && !actions.testFlag(AbstractJobHandler::SupportAction::kPauseAction)
                       && !actions.testFlag(AbstractJobHandler::SupportAction::kResumAction)) {
                //no error thread resume
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
    if (errorQueue.size() > 1)
        return;
    // 进行错误处理
    emit errorNotify(jobInfo);
}

void AbstractJob::handleRetryErrorSuccess(const quint64 Id)
{
    // retry error dealing success and dealing next error
    if (errorQueue.size() <= 0 || errorQueue.head()->value(AbstractJobHandler::NotifyInfoKey::kWorkerPointer).value<quint64>() != Id) {
        if (errorQueue.size() > 0 && errorQueue.head()->value(AbstractJobHandler::NotifyInfoKey::kWorkerPointer).value<quint64>() != Id)
            fmCritical() << "error current error thread id = " << Id << " error Queue error id = " << errorQueue.head()->value(AbstractJobHandler::NotifyInfoKey::kWorkerPointer);
        return;
    }
    errorQueue.dequeue();
    if (errorQueue.size() > 0) {
        emit errorNotify(errorQueue.head());
    } else {
        doWorker->resumeAllThread();
    }
}

AbstractJob::~AbstractJob()
{
    thread.quit();
    thread.wait();
}
