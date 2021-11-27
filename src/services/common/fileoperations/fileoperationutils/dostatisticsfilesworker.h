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
#ifndef DOSTATISTICSFILESWORKER_H
#define DOSTATISTICSFILESWORKER_H

#include "dfm_common_service_global.h"
#include "fileoperationsutils.h"

#include <QObject>
#include <QUrl>
#include <QWaitCondition>
#include <QMutex>

DSC_BEGIN_NAMESPACE
class DoStatisticsFilesWorker : public QObject
{
    friend class StatisticsFilesSize;
    Q_OBJECT
    explicit DoStatisticsFilesWorker(const QList<QUrl> &files, const bool isRecordAll, QObject *parent = nullptr);

public:
    ~DoStatisticsFilesWorker() = default;

signals:
    /*!
     * \brief finished 统计所有文件大小信息完成
     * \param sizeInfo 统计所有文件大小信息
     */
    void finished(const SizeInfoPoiter sizeInfo);

private slots:
    void doWork();
    void stopWork();

protected:
    QList<QUrl> files;   // 所有源文件的列表
    bool isRecordAll { false };   // 是否记录统计的所有文件及子目录下的所有文件
    QAtomicInteger<bool> isStoped { false };   // 是否停止
};
DSC_END_NAMESPACE

#endif   // DOSTATISTICSFILESWORKER_H
