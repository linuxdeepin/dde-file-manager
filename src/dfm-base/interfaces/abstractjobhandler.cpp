/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "abstractjobhandler.h"

DFMBASE_USE_NAMESPACE

AbstractJobHandler::AbstractJobHandler(QObject *parent)
    : QObject(parent) {}

AbstractJobHandler::~AbstractJobHandler() {}
/*!
 * @brief AbstractJobHandler::getCurrentJobProcess 获取当前任务的进度
 * 进度是当前任务拷贝的百分比*100,例如，拷贝过程中，当前拷贝了2300kb，总大小是250000kb，当前的进度是
 * (2300kb/250000kb) = 0.92
 * @return qreal 当前任务拷贝的进度
 */
qreal AbstractJobHandler::currentJobProcess() const
{
    return 0.0;
}
/*!
 * @brief AbstractJobHandler::getTotalSize 获取当前任务所有文件的总大小
 * 获取任务总共大小，如拷贝任务，获取就是当前任务拷贝的总大小，（拷贝任务可能才开始获取的不准确，开异步线程统计需要时间）
 * @return qreal 当前任务执行的进度
 */
qint64 AbstractJobHandler::totalSize() const
{
    return 0;
}
/*!
 * @brief getCurrentSize 获取当前任务的进度（执行了任务的大小）
 * 获取任务总共大小，如拷贝任务，获取就是当前任务拷贝了的文件大小，（包含跳过了文件的大小）
 * @return qint64 当前任务的总大小
 */
qint64 AbstractJobHandler::currentSize() const
{
    return 0;
}
/*!
 * @brief getCurrentState 获取当前任务状态
 * @return qreal 返回当前任务的状态
 */
AbstractJobHandler::JobState AbstractJobHandler::currentState() const
{
    return JobState::kUnknowState;
}
/*!
 * \brief AbstractJobHandler::setSignalConnectFinished 设置连接信号处理槽函数完成
 * 处理类连接了所有的要处理的槽函数后设置，才会发送相应的信号，信号之前的信息都可以使用相应的接口获取
 */
void AbstractJobHandler::setSignalConnectFinished()
{
    isSignalConnectOver = true;
}
/*!
 * \brief AbstractJobHandler::operateTaskJob 对任务进行操作
 * 如：停止任务，暂停任务，任务重试，替换操作
 * \param actions 操作的动作 这里的动作只能是action的一种和kRememberAction并存
 */
void AbstractJobHandler::operateTaskJob(SupportActions actions)
{
    emit userAction(actions);
}

void AbstractJobHandler::onProccessChanged(const JobInfoPointer JobInfo)
{
    //TODO:: do save something to use

    if (isSignalConnectOver)
        emit proccessChangedNotify(JobInfo);
}

void AbstractJobHandler::onStateChanged(const JobInfoPointer JobInfo)
{
    //TODO:: do save something to use

    if (isSignalConnectOver)
        emit stateChangedNotify(JobInfo);
}

void AbstractJobHandler::onCurrentTask(const JobInfoPointer JobInfo)
{
    //TODO:: do save something to use

    if (isSignalConnectOver)
        emit currentTaskNotify(JobInfo);
}

void AbstractJobHandler::onError(const JobInfoPointer JobInfo)
{
    //TODO:: do save something to use
    if (isSignalConnectOver)
        emit currentTaskNotify(JobInfo);
}

void AbstractJobHandler::onFinished()
{
    if (isSignalConnectOver)
        emit finishedNotify();
}

void AbstractJobHandler::onSpeedUpdated(const JobInfoPointer JobInfo)
{
    //TODO:: do save something to use

    if (isSignalConnectOver)
        emit speedUpdatedNotify(JobInfo);
}
