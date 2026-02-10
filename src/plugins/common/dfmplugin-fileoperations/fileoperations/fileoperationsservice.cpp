// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileoperationsservice.h"
#include "copyfiles/copyfiles.h"
#include "cutfiles/cutfiles.h"
#include "deletefiles/deletefiles.h"
#include "trashfiles/movetotrashfiles.h"
#include "trashfiles/restoretrashfiles.h"
#include "trashfiles/copyfromtrashfiles.h"
#include "cleantrash/cleantrashfiles.h"

#include <QUrl>

/*!
 * 特殊说明，调用当前服务的5个服务时，返回的JobHandlePointer时就已经启动了线程去处理相应的任务，那么使用JobHandlePointer做信号链接时，
 * 错误信息可能信号已发送了，只能使用JobHandlePointer去判断是否有错误，并做相应的处理
 */

DPFILEOPERATIONS_USE_NAMESPACE
FileOperationsService::FileOperationsService(QObject *parent)
    : QObject(parent)
{
}

FileOperationsService::~FileOperationsService()
{
}

/*!
 * \brief FileOperationsService::copy 拷贝文件有UI界面
 *  这个接口只能拷贝源文件在同一目录的文件，也就是源文件都是同一个设备上的文件。拷贝根据源文件的scheme来进行创建不同的file
 *  执行不同scheme的read、write和同步
 *  如果出入的scheme是自定义的（dfm-io不支持的scheme），那么拷贝任务执行拷贝时使用的read和write都使用dfm-io提供的默认操作
 * \param sources 源文件的列表
 * \param target 目标目录
 * \return QSharedPointer<AbstractJobHandler> 任务控制器
 */
JobHandlePointer FileOperationsService::copy(const QList<QUrl> &sources, const QUrl &target,
                                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags)
{
    JobHandlePointer jobHandler(new DFMBASE_NAMESPACE::AbstractJobHandler);
    CopyFiles *task = new CopyFiles();
    task->setJobArgs(jobHandler, sources, target, flags);
    connect(jobHandler.data(), &DFMBASE_NAMESPACE::AbstractJobHandler::workerFinish, this, &FileOperationsService::handleWorkerFinish);
    jobs.insert(QString::number(quintptr(jobHandler.data()), 16), jobHandler);
    return jobHandler;
}

JobHandlePointer FileOperationsService::copyFromTrash(const QList<QUrl> &sources, const QUrl &target, const AbstractJobHandler::JobFlags &flags)
{
    JobHandlePointer jobHandler(new DFMBASE_NAMESPACE::AbstractJobHandler);
    CopyFromTrashTrashFiles *task = new CopyFromTrashTrashFiles();
    task->setJobArgs(jobHandler, sources, target, flags);
    connect(jobHandler.data(), &DFMBASE_NAMESPACE::AbstractJobHandler::workerFinish, this, &FileOperationsService::handleWorkerFinish);
    jobs.insert(QString::number(quintptr(jobHandler.data()), 16), jobHandler);
    return jobHandler;
}
/*!
 * \brief FileOperationsService::moveToTrash 移动文件到回收站
 *  一个移动到回收站的任务的源文件都是在同一目录下，所以源文件都是在同一个设备上。移动到回收站只能是不可移除设备
 * （系统盘，或者安装系统时挂载的机械硬盘），如果文件的大小操过1g（目前设计是这样）不能放入回收站，只能删除
 * \param sources 移动到回收站的源文件
 * \return JobHandlePointer 任务控制器
 */
JobHandlePointer FileOperationsService::moveToTrash(const QList<QUrl> &sources,
                                                    const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags)
{
    JobHandlePointer jobHandler(new DFMBASE_NAMESPACE::AbstractJobHandler);
    MoveToTrashFiles *task = new MoveToTrashFiles();
    task->setJobArgs(jobHandler, sources, QUrl(), flags);
    connect(jobHandler.data(), &DFMBASE_NAMESPACE::AbstractJobHandler::workerFinish, this, &FileOperationsService::handleWorkerFinish);
    jobs.insert(QString::number(quintptr(jobHandler.data()), 16), jobHandler);
    return jobHandler;
}
/*!
 * \brief FileOperationsService::restoreFromTrash
 * 从回收站还原文件，原目录下有相同文件，提示替换或者共存
 * \param sources 需要还原的文件
 * \return JobHandlePointer 任务控制器
 */
JobHandlePointer FileOperationsService::restoreFromTrash(const QList<QUrl> &sources, const QUrl &target,
                                                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags)
{
    JobHandlePointer jobHandler(new DFMBASE_NAMESPACE::AbstractJobHandler);
    RestoreTrashFiles *task = new RestoreTrashFiles();
    task->setJobArgs(jobHandler, sources, target, flags);
    connect(jobHandler.data(), &DFMBASE_NAMESPACE::AbstractJobHandler::workerFinish, this, &FileOperationsService::handleWorkerFinish);
    jobs.insert(QString::number(quintptr(jobHandler.data()), 16), jobHandler);
    return jobHandler;
}
/*!
 * \brief FileOperationsService::deletes 删除文件
 * 一个删除的任务的源文件都是在同一目录下，所以源文件都是在同一个设备上。
 * \param sources 需要删除的源文件
 * \return JobHandlePointer 任务控制器
 */
JobHandlePointer FileOperationsService::deletes(const QList<QUrl> &sources,
                                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags)
{
    JobHandlePointer jobHandler(new DFMBASE_NAMESPACE::AbstractJobHandler);
    DeleteFiles *task = new DeleteFiles();
    task->setJobArgs(jobHandler, sources, QUrl(), flags);
    connect(jobHandler.data(), &DFMBASE_NAMESPACE::AbstractJobHandler::workerFinish, this, &FileOperationsService::handleWorkerFinish);
    jobs.insert(QString::number(quintptr(jobHandler.data()), 16), jobHandler);
    return jobHandler;
}
/*!
 * \brief FileOperationsService::cut 剪切文件
 * 一个剪切的任务的源文件都是在同一目录下，所以源文件都是在同一个设备上。判断源文件和目标文件是否在同一个设备上
 * 在同一个目录上执行dorename，如果dorename失败或者不在同一个设备上就执行先拷贝，在剪切
 * \param sources 源文件的列表
 * \param target 目标目录
 * \return JobHandlePointer 任务控制器
 */
JobHandlePointer FileOperationsService::cut(const QList<QUrl> &sources, const QUrl &target,
                                            const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags)
{
    JobHandlePointer jobHandler(new DFMBASE_NAMESPACE::AbstractJobHandler);
    CutFiles *task = new CutFiles();
    task->setJobArgs(jobHandler, sources, target, flags);
    connect(jobHandler.data(), &DFMBASE_NAMESPACE::AbstractJobHandler::workerFinish, this, &FileOperationsService::handleWorkerFinish);
    jobs.insert(QString::number(quintptr(jobHandler.data()), 16), jobHandler);
    return jobHandler;
}
/*!
 * \brief FileOperationsService::cleanTrash 清理回收站文件
 * 一个清理回收站的任务的源文件都是在同一目录下，所以源文件都是在同一个设备上。但是要清理
 * \param sources 源文件的列表
 * \return JobHandlePointer 任务控制器
 */
JobHandlePointer FileOperationsService::cleanTrash(const QList<QUrl> &sources)
{
    JobHandlePointer jobHandler(new DFMBASE_NAMESPACE::AbstractJobHandler);
    CleanTrashFiles *task = new CleanTrashFiles();
    task->setJobArgs(jobHandler, sources);
    connect(jobHandler.data(), &DFMBASE_NAMESPACE::AbstractJobHandler::workerFinish, this, &FileOperationsService::handleWorkerFinish);
    jobs.insert(QString::number(quintptr(jobHandler.data()), 16), jobHandler);
    return jobHandler;
}

void FileOperationsService::handleWorkerFinish()
{
    if (!sender())
        return;
    jobs.remove(QString::number(quintptr(sender()), 16));
}
