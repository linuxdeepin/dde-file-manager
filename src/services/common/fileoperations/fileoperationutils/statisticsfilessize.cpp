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
#include "statisticsfilessize.h"
#include "dostatisticsfilesworker.h"

DSC_USE_NAMESPACE
StatisticsFilesSize::StatisticsFilesSize(const QList<QUrl> &files, const bool isRecordAll, QObject *parent)
    : QObject(parent), doWorker(new DoStatisticsFilesWorker(files, isRecordAll))
{
    doWorker->moveToThread(&thread);
    connect(this, &StatisticsFilesSize::startWork, doWorker.data(), &DoStatisticsFilesWorker::doWork);
    connect(this, &StatisticsFilesSize::stopWork, doWorker.data(), &DoStatisticsFilesWorker::stopWork);

    connect(doWorker.data(), &DoStatisticsFilesWorker::finished, this, &StatisticsFilesSize::onFinished, Qt::QueuedConnection);
}

StatisticsFilesSize::~StatisticsFilesSize()
{
    thread.quit();
    thread.wait();
}
/*!
 * \brief StatisticsFilesSize::start 启动线程去统计文件大小
 */
void StatisticsFilesSize::start()
{
    if (!thread.isRunning()) {
        thread.start();
        emit startWork();
    }
}
/*!
 * \brief StatisticsFilesSize::stop 停止线程
 */
void StatisticsFilesSize::stop()
{
    isPositiveStop = true;
    emit stopWork();
    thread.quit();
    thread.wait();
}
/*!
 * \brief StatisticsFilesSize::isFinished 统计大小线程是否结束
 * \return
 */
bool StatisticsFilesSize::isFinished() const
{
    return isFinishedState;
}
/*!
 * \brief StatisticsFilesSize::sizeInfo 获取统计的所有文件大小信息
 * \return SizeInfoPoiter 文件大小信息
 */
SizeInfoPoiter StatisticsFilesSize::sizeInfo() const
{
    return sizeInfoPinter;
}
/*!
 * \brief StatisticsFilesSize::onFinished 收到worker的结束信号，处理相应的信息
 * \param sizeInfo
 */
void StatisticsFilesSize::onFinished(const SizeInfoPoiter sizeInfo)
{
    sizeInfoPinter = sizeInfo;
    isFinishedState = true;
    // 主动停止就不用发送完成信号，一般就是拷贝结束了
    if (!isPositiveStop)
        emit finished(sizeInfoPinter);
}
