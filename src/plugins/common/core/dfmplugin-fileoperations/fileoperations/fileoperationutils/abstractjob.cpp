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
#include "abstractjob.h"
#include "abstractworker.h"

#include <QDebug>
#include <QUrl>

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
        qWarning() << "JobHandlePointer is a nullptr, setJobArgs failed!";
        return;
    }
    connect(handle.get(), &AbstractJobHandler::userAction, this, &AbstractJob::operateCopy);
    connect(this, &AbstractJob::requestShowTipsDialog, handle.get(), &AbstractJobHandler::requestShowTipsDialog);
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
        connect(this, &AbstractJob::startWork, doWorker, &AbstractWorker::doWork);
        connect(doWorker, &AbstractWorker::finishedNotify, this, &AbstractJob::deleteLater);
        connect(doWorker, &AbstractWorker::requestShowTipsDialog, this, &AbstractJob::requestShowTipsDialog);
    }
}

/*!
 * \brief operateCopy 处理handle上的用户操作
 * \param actions 操作类型
 */
void AbstractJob::operateCopy(AbstractJobHandler::SupportActions actions)
{
    if (actions.testFlag(AbstractJobHandler::SupportAction::kStartAction)) {
        start();
        emit startWork();
    } else {
        if (doWorker)
            doWorker->doOperateWork(actions);
    }
}

AbstractJob::~AbstractJob()
{
    thread.quit();
    thread.wait();
}
