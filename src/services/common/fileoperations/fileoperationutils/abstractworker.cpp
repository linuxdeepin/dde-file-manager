/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#include "abstractworker.h"

#include <QUrl>
#include <QDebug>
#include <QWaitCondition>

DSC_USE_NAMESPACE
/*!
 * \brief setWorkArgs 设置当前任务的参数
 * \param args 参数
 */
void AbstractWorker::setWorkArgs(const JobHandlePointer &handle, const QList<QUrl> &sources, const QUrl &target,
                                 const AbstractJobHandler::JobFlags &flags)
{
    if (!handle) {
        qWarning() << "JobHandlePointer is a nullptr, setWorkArgs failed!";
        return;
    }
    initHandleConnects(handle);
    this->sources = sources;
    this->target = target;
    jobFlags = flags;
}

/*!
 * \brief doOperateWork 处理用户的操作 不在拷贝线程执行的函数，协同类直接调用
 * \param actions 当前操作
 */
void AbstractWorker::doOperateWork(AbstractJobHandler::SupportActions actions)
{
    if (actions.testFlag(AbstractJobHandler::SupportAction::kStopAction)) {
        stop();
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kPauseAction)) {
        pause();
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kResumAction)) {
        resume();
    } else {
        if (actions.testFlag(AbstractJobHandler::SupportAction::kCancelAction)) {
            currentAction = AbstractJobHandler::SupportAction::kCancelAction;
        } else if (actions.testFlag(AbstractJobHandler::SupportAction::kCoexistAction)) {
            currentAction = AbstractJobHandler::SupportAction::kCoexistAction;
        } else if (actions.testFlag(AbstractJobHandler::SupportAction::kSkipAction)) {
            currentAction = AbstractJobHandler::SupportAction::kSkipAction;
        } else if (actions.testFlag(AbstractJobHandler::SupportAction::kMergeAction)) {
            currentAction = AbstractJobHandler::SupportAction::kMergeAction;
        } else if (actions.testFlag(AbstractJobHandler::SupportAction::kReplaceAction)) {
            currentAction = AbstractJobHandler::SupportAction::kReplaceAction;
        } else if (actions.testFlag(AbstractJobHandler::SupportAction::kRetryAction)) {
            currentAction = AbstractJobHandler::SupportAction::kRetryAction;
        } else if (actions.testFlag(AbstractJobHandler::SupportAction::kEnforceAction)) {
            currentAction = AbstractJobHandler::SupportAction::kEnforceAction;
        } else {
            currentAction = AbstractJobHandler::SupportAction::kNoAction;
        }

        if (handlingErrorCondition)
            handlingErrorCondition->wakeAll();
    }
}

AbstractWorker::AbstractWorker(QObject *parent)
    : QObject(parent)
{
}

/*!
 * \brief AbstractWorker::initHandleConnects 初始化当前信号的连接
 * \param handle 任务控制处理器
 */
void AbstractWorker::initHandleConnects(const JobHandlePointer &handle)
{
    if (!handle) {
        qWarning() << "JobHandlePointer is a nullptr,so connects failed!";
        return;
    }

    connect(handle.data(), &AbstractJobHandler::userAction, this, &AbstractWorker::doOperateWork, Qt::QueuedConnection);

    connect(this, &AbstractWorker::proccessChangedNotify, handle.data(), &AbstractJobHandler::onProccessChanged,
            Qt::QueuedConnection);
    connect(this, &AbstractWorker::stateChangedNotify, handle.data(), &AbstractJobHandler::onStateChanged,
            Qt::QueuedConnection);
    connect(this, &AbstractWorker::currentTaskNotify, handle.data(), &AbstractJobHandler::onCurrentTask,
            Qt::QueuedConnection);
    connect(this, &AbstractWorker::finishedNotify, handle.data(), &AbstractJobHandler::onFinished,
            Qt::QueuedConnection);
    connect(this, &AbstractWorker::errorNotify, handle.data(), &AbstractJobHandler::onError,
            Qt::QueuedConnection);
    connect(this, &AbstractWorker::speedUpdatedNotify, handle.data(), &AbstractJobHandler::onSpeedUpdated,
            Qt::QueuedConnection);
}
