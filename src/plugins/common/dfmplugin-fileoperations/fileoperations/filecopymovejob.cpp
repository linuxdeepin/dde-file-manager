// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filecopymovejob.h"

#include <QUrl>
#include <QTimer>

/*!
 * 特殊说明，调用当前服务的5个服务时，返回的JobHandlePointer时就已经启动了线程去处理相应的任务，那么使用JobHandlePointer做信号链接时，
 * 错误信息可能信号已发送了，只能使用JobHandlePointer去判断是否有错误，并做相应的处理
 */
DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE
FileCopyMoveJob::FileCopyMoveJob(QObject *parent)
    : QObject(parent)
{
    copyMoveTaskMutex.reset(new QMutex);
}

FileCopyMoveJob::~FileCopyMoveJob()
{
}

bool FileCopyMoveJob::getOperationsAndDialogService()
{
    if (!operationsService)
        operationsService.reset(new FileOperationsService(this));

    if (!dialogManager)
        dialogManager = DialogManagerInstance;

    return operationsService && dialogManager;
}

void FileCopyMoveJob::onHandleAddTask()
{
    QMutexLocker lk(copyMoveTaskMutex.data());
    QObject *send = sender();
    JobHandlePointer jobHandler = send->property("jobPointer").value<JobHandlePointer>();
    send->setProperty("jobPointer", QVariant());
    if (!getOperationsAndDialogService()) {
        fmCritical() << "Failed to get operations service or dialog manager";
        return;
    }
    dialogManager->addTask(jobHandler);
    jobHandler->disconnect(jobHandler.get(), &AbstractJobHandler::finishedNotify, this, &FileCopyMoveJob::onHandleTaskFinished);
}

void FileCopyMoveJob::onHandleAddTaskWithArgs(const JobInfoPointer info)
{
    QMutexLocker lk(copyMoveTaskMutex.data());

    JobHandlePointer jobHandler = info->value(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer).value<JobHandlePointer>();
    if (!getOperationsAndDialogService()) {
        fmCritical() << "Failed to get operations service or dialog manager";
        return;
    }

    if (dialogManager)
        dialogManager->addTask(jobHandler);
}

void FileCopyMoveJob::onHandleTaskFinished(const JobInfoPointer info)
{
    JobHandlePointer jobHandler = info->value(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer).value<JobHandlePointer>();
    {
        QMutexLocker lk(copyMoveTaskMutex.data());
        copyMoveTask.remove(jobHandler);
    }
}

void FileCopyMoveJob::initArguments(const JobHandlePointer handler, const AbstractJobHandler::JobFlags flags)
{
    if (flags.testFlag(AbstractJobHandler::JobFlag::kCopyRemote)) {
        handler->connect(handler.get(), &AbstractJobHandler::errorNotify, this, &FileCopyMoveJob::onHandleAddTaskWithArgs);
        handler->connect(handler.get(), &AbstractJobHandler::finishedNotify, this, &FileCopyMoveJob::onHandleTaskFinished);
        connect(handler.get(), &AbstractJobHandler::requestTaskDailog, this, [this, handler]() {
            startAddTaskTimer(handler, true);
        });
        handler->start();
        return;
    }
    startAddTaskTimer(handler, false);
}

void FileCopyMoveJob::startAddTaskTimer(const JobHandlePointer handler, const bool isRemote)
{
    if (!isRemote) {
        handler->connect(handler.get(), &AbstractJobHandler::errorNotify, this, &FileCopyMoveJob::onHandleAddTaskWithArgs);
        handler->connect(handler.get(), &AbstractJobHandler::finishedNotify, this, &FileCopyMoveJob::onHandleTaskFinished);
    }

    QSharedPointer<QTimer> timer(new QTimer);
    timer->setSingleShot(true);
    timer->setInterval(1000);
    timer->connect(timer.data(), &QTimer::timeout, this, &FileCopyMoveJob::onHandleAddTask);

    timer->setProperty("jobPointer", QVariant::fromValue(handler));
    {
        QMutexLocker lk(copyMoveTaskMutex.data());
        copyMoveTask.insert(handler, timer);
    }
    timer->start();
    if (!isRemote)
        handler->start();
}
/*!
 * \brief FileCopyMoveJob::copy 拷贝文件有UI界面
 *  这个接口只能拷贝源文件在同一目录的文件，也就是源文件都是同一个设备上的文件。拷贝根据源文件的scheme来进行创建不同的file
 *  执行不同scheme的read、write和同步
 *  如果出入的scheme是自定义的（dfm-io不支持的scheme），那么拷贝任务执行拷贝时使用的read和write都使用dfm-io提供的默认操作
 * \param sources 源文件的列表
 * \param target 目标目录
 * \return QSharedPointer<AbstractJobHandler> 任务控制器
 */
JobHandlePointer FileCopyMoveJob::copy(const QList<QUrl> &sources, const QUrl &target,
                                       const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags)
{
    if (!getOperationsAndDialogService()) {
        fmCritical() << "Failed to get operations service or dialog manager";
        return nullptr;
    }

    JobHandlePointer jobHandle = operationsService->copy(sources, target, flags);
    initArguments(jobHandle, flags);

    return jobHandle;
}

JobHandlePointer FileCopyMoveJob::copyFromTrash(const QList<QUrl> &sources, const QUrl &target, const AbstractJobHandler::JobFlags &flags)
{
    if (!getOperationsAndDialogService()) {
        fmCritical() << "Failed to get operations service or dialog manager";
        return nullptr;
    }

    JobHandlePointer jobHandle = operationsService->copyFromTrash(sources, target, flags);
    initArguments(jobHandle);

    return jobHandle;
}
/*!
 * \brief FileCopyMoveJob::moveToTrash 移动文件到回收站
 *  一个移动到回收站的任务的源文件都是在同一目录下，所以源文件都是在同一个设备上。移动到回收站只能是不可移除设备
 * （系统盘，或者安装系统时挂载的机械硬盘），如果文件的大小操过1g（目前设计是这样）不能放入回收站，只能删除
 * \param sources 移动到回收站的源文件
 * \return JobHandlePointer 任务控制器
 */
JobHandlePointer FileCopyMoveJob::moveToTrash(const QList<QUrl> &sources,
                                              const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags, const bool isInit)
{
    if (!getOperationsAndDialogService()) {
        fmCritical() << "Failed to get operations service or dialog manager";
        return nullptr;
    }

    JobHandlePointer jobHandle = operationsService->moveToTrash(sources, flags);
    if (isInit)
        initArguments(jobHandle);

    return jobHandle;
}
/*!
 * \brief FileCopyMoveJob::restoreFromTrash
 * 从回收站还原文件，原目录下有相同文件，提示替换或者共存
 * \param sources 需要还原的文件
 * \return JobHandlePointer 任务控制器
 */
JobHandlePointer FileCopyMoveJob::restoreFromTrash(const QList<QUrl> &sources, const QUrl &target,
                                                   const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags, const bool isInit)
{
    if (!getOperationsAndDialogService()) {
        fmCritical() << "Failed to get operations service or dialog manager";
        return nullptr;
    }

    JobHandlePointer jobHandle = operationsService->restoreFromTrash(sources, target, flags);
    if (isInit)
        initArguments(jobHandle);

    return jobHandle;
}
/*!
 * \brief FileCopyMoveJob::deletes 删除文件
 * 一个删除的任务的源文件都是在同一目录下，所以源文件都是在同一个设备上。
 * \param sources 需要删除的源文件
 * \return JobHandlePointer 任务控制器
 */
JobHandlePointer FileCopyMoveJob::deletes(const QList<QUrl> &sources,
                                          const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags, const bool isInit)
{
    if (!getOperationsAndDialogService()) {
        fmCritical() << "Failed to get operations service or dialog manager";
        return nullptr;
    }

    JobHandlePointer jobHandle = operationsService->deletes(sources, flags);

    if (isInit)
        initArguments(jobHandle);

    return jobHandle;
}
/*!
 * \brief FileCopyMoveJob::cut 剪切文件
 * 一个剪切的任务的源文件都是在同一目录下，所以源文件都是在同一个设备上。判断源文件和目标文件是否在同一个设备上
 * 在同一个目录上执行dorename，如果dorename失败或者不在同一个设备上就执行先拷贝，在剪切
 * \param sources 源文件的列表
 * \param target 目标目录
 * \return JobHandlePointer 任务控制器
 */
JobHandlePointer FileCopyMoveJob::cut(const QList<QUrl> &sources, const QUrl &target,
                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags,
                                      const bool isInit)
{
    if (!getOperationsAndDialogService()) {
        fmCritical() << "Failed to get operations service or dialog manager";
        return nullptr;
    }

    JobHandlePointer jobHandle = operationsService->cut(sources, target, flags);
    if (isInit)
        initArguments(jobHandle);

    return jobHandle;
}
/*!
 * \brief FileCopyMoveJob::cleanTrash 清理回收站文件
 * 一个清理回收站的任务的源文件都是在同一目录下，所以源文件都是在同一个设备上。但是要清理
 * \param sources 源文件的列表
 * \return JobHandlePointer 任务控制器
 */
JobHandlePointer FileCopyMoveJob::cleanTrash(const QList<QUrl> &sources)
{
    if (!getOperationsAndDialogService()) {
        fmCritical() << "Failed to get operations service or dialog manager";
        return nullptr;
    }

    JobHandlePointer jobHandle = operationsService->cleanTrash(sources);
    initArguments(jobHandle);

    return jobHandle;
}
