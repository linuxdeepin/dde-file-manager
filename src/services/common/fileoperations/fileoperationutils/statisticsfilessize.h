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
#ifndef STATISTICSFILESSIZE_H
#define STATISTICSFILESSIZE_H

#include "dfm_common_service_global.h"
#include "fileoperationsutils.h"

#include <QThread>
#include <QSharedPointer>
#include <QUrl>

DSC_BEGIN_NAMESPACE
class DoStatisticsFilesWorker;
class StatisticsFilesSize : public QObject
{
    Q_OBJECT
    friend class AbstractWorker;
    friend class DoCopyFilesWorker;
    explicit StatisticsFilesSize(const QList<QUrl> &files, const bool isRecordAll = false, QObject *parent = nullptr);

    void start();
    void stop();
    bool isFinished() const;
    SizeInfoPoiter sizeInfo() const;

public:
    ~StatisticsFilesSize();

signals:   // 控制worker使用的
    /*!
     * \brief startWork 通知worker执行线程函数
     */
    void startWork();
    /*!
     * \brief stopWork 通知worker停止执行线程函数
     */
    void stopWork();
signals:   // 发送给外部使用的
    /*!
     * \brief finished 统计结束信号
     * \param sizeInfo 统计文件大小信息
     */
    void finished(const SizeInfoPoiter sizeInfo);

private slots:
    void onFinished(const SizeInfoPoiter sizeInfo);

private:
    QThread thread;   // 线程
    QSharedPointer<DoStatisticsFilesWorker> doWorker { nullptr };   // 线程执行类
    SizeInfoPoiter sizeInfoPinter { nullptr };   // 统计文件的大小信息
    bool isFinishedState { false };   // 统计任务是否完成
    bool isPositiveStop { false };   // 是否是自己主动停止
};
DSC_END_NAMESPACE

#endif   // STATISTICSFILESSIZE_H
