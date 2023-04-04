// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/dialogmanager.h>

#include <QUrl>
#include <QList>

using namespace dfmbase;

AbstractJobHandler::AbstractJobHandler(QObject *parent)
    : QObject(parent)
{
    connect(this, &AbstractJobHandler::requestShowTipsDialog, this, [=](DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType type, const QList<QUrl> urls) {
        switch (type) {
        case DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType::kRestoreFailed:
            DialogManagerInstance->showRestoreFailedDialog(urls.count());
            break;
        case DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType::kCopyMoveToSelf:
            DialogManagerInstance->showCopyMoveToSelfDialog();
            break;
        }
    });
}

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

QMap<AbstractJobHandler::NotifyType, JobInfoPointer> AbstractJobHandler::getAllTaskInfo()
{
    QMutexLocker lk(&taskInfoMutex);
    return taskInfo;
}

JobInfoPointer AbstractJobHandler::getTaskInfoByNotifyType(const AbstractJobHandler::NotifyType &notifyType)
{
    QMutexLocker lk(&taskInfoMutex);
    return taskInfo.value(notifyType);
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

void AbstractJobHandler::onProccessChanged(const JobInfoPointer jobInfo)
{
    //TODO:: do save something to use
    {
        QMutexLocker lk(&taskInfoMutex);
        taskInfo.insert(NotifyType::kNotifyProccessChangedKey, jobInfo);
    }
    if (isSignalConnectOver)
        emit proccessChangedNotify(jobInfo);
}

void AbstractJobHandler::onStateChanged(const JobInfoPointer jobInfo)
{
    //TODO:: do save something to use
    {
        QMutexLocker lk(&taskInfoMutex);
        taskInfo.insert(NotifyType::kNotifyStateChangedKey, jobInfo);
    }
    if (isSignalConnectOver)
        emit stateChangedNotify(jobInfo);
}

void AbstractJobHandler::onCurrentTask(const JobInfoPointer jobInfo)
{
    //TODO:: do save something to use
    {
        QMutexLocker lk(&taskInfoMutex);
        taskInfo.insert(NotifyType::kNotifyCurrentTaskKey, jobInfo);
    }
    if (isSignalConnectOver)
        emit currentTaskNotify(jobInfo);
}

void AbstractJobHandler::onError(const JobInfoPointer jobInfo)
{
    //TODO:: do save something to use
    {
        QMutexLocker lk(&taskInfoMutex);
        JobInfoPointer tempInfo(new QMap<quint8, QVariant>);
        *tempInfo = *jobInfo;
        tempInfo->remove(int(NotifyInfoKey::kJobHandlePointer));
        taskInfo.insert(NotifyType::kNotifyErrorTaskKey, tempInfo);
    }
    emit errorNotify(jobInfo);
}

void AbstractJobHandler::onFinished(const JobInfoPointer jobInfo)
{
    {
        QMutexLocker lk(&taskInfoMutex);
        JobInfoPointer tempInfo(new QMap<quint8, QVariant>);
        *tempInfo = *jobInfo;
        tempInfo->remove(int(NotifyInfoKey::kJobHandlePointer));
        taskInfo.insert(NotifyType::kNotifyFinishedKey, tempInfo);
    }

    emit finishedNotify(jobInfo);
}

void AbstractJobHandler::onSpeedUpdated(const JobInfoPointer jobInfo)
{
    //TODO:: do save something to use
    {
        QMutexLocker lk(&taskInfoMutex);
        taskInfo.insert(NotifyType::kNotifySpeedUpdatedTaskKey, jobInfo);
    }
    if (isSignalConnectOver)
        emit speedUpdatedNotify(jobInfo);
}

void AbstractJobHandler::start()
{
    operateTaskJob(SupportAction::kStartAction);
}
