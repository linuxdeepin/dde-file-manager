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
#ifndef ABSTRACTJOB_H
#define ABSTRACTJOB_H

#include "dfmplugin_fileoperations_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"

#include <QObject>
#include <QUrl>
#include <QThread>
#include <QSharedPointer>
#include <QQueue>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_BEGIN_NAMESPACE
class AbstractWorker;
class AbstractJob : public QObject
{
    Q_OBJECT
    friend class FileOperationsService;
    virtual void setJobArgs(const JobHandlePointer handle, const QList<QUrl> &sources, const QUrl &target = QUrl(),
                            const AbstractJobHandler::JobFlags &flags = AbstractJobHandler::JobFlag::kNoHint);
signals:   // 对线程协同的worker使用
    /*!
     * \brief operateWork 操作当前任务
     * \param actions 操作类型
     */
    void operateWork(AbstractJobHandler::SupportActions actions);
    /*!
     * \brief errorNotify 错误信息，此信号都可能是异步连接，所以所有参数都没有使用引用
     * \param info 这个Varint信息map
     * 在我们自己提供的fileoperations服务中，这个VarintMap里面会存在kJobtypeKey任务类型，类型JobType）、source（源文件url，类型：QUrl）
     * 、target（源文件url，类型：QUrl）、errorType（错误类型，类型：JobErrorType）、sourceMsg（源文件url拼接的显示字符串，
     * 类型：QString）、targetMsg（目标文件url拼接的显示字符串，类型：QString）、kErrorMsgKey（错误信息字符串，类型：QString）、
     * kActionsKey（支持的操作，类型：SupportActions）
     * 在我们自己提供的dailog服务中，这个VarintMap必须有存在sourceMsg（显示任务的左第一个label的显示，类型：QString）、
     * targetMsg（显示任务的左第二个label的显示，类型：QString）、kErrorMsgKey（显示任务的左第三个label的显示，类型：QString）、
     * kActionsKey（支持的操作，用来显示那些按钮，类型：SupportActions）
     */
    void errorNotify(const JobInfoPointer jobInfo);

    void requestShowTipsDialog(DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType type, const QList<QUrl> list);

protected slots:
    void operateAation(AbstractJobHandler::SupportActions actions);
    void handleError(const JobInfoPointer jobInfo);
    void handleRetryErrorSuccess(const quint64 Id);

protected:
    void start();
    explicit AbstractJob(AbstractWorker *doWorker, QObject *parent = nullptr);

public:
    virtual ~AbstractJob();

protected:
    QSharedPointer<AbstractWorker> doWorker { nullptr };   // 线程执行函数
    QThread thread;   // 线程
private:
    QQueue<JobInfoPointer> errorQueue;
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // ABSTRACTJOB_H
