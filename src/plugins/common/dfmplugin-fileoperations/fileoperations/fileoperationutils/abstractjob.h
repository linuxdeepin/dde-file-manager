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
    void startWork();
    void requestShowTipsDialog(DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType type, const QList<QUrl> list);

protected slots:
    void operateCopy(AbstractJobHandler::SupportActions actions);

protected:
    void start();
    explicit AbstractJob(AbstractWorker *doWorker, QObject *parent = nullptr);

public:
    virtual ~AbstractJob();

protected:
    QSharedPointer<AbstractWorker> doWorker { nullptr };   // 线程执行函数
    QThread thread;   // 线程
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // ABSTRACTJOB_H
