/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef FILEOPERATIONSUTILS_H
#define FILEOPERATIONSUTILS_H

#include "dfm_common_service_global.h"
#include "dfm-base/utils/fileutils.h"

#include <QSharedPointer>
#include <QThread>

DSC_BEGIN_NAMESPACE
class UpdateProccessTimer : public QObject
{
    Q_OBJECT
    friend class AbstractWorker;
    friend class DoCopyFilesWorker;
    explicit UpdateProccessTimer(QObject *parent = nullptr)
        : QObject(parent) {}
    void stopTimer()
    {
        isStop = true;
    }
signals:
    void updateProccessNotify();
private slots:
    void doStartTime()
    {
        while (true) {
            emit updateProccessNotify();
            QThread::msleep(500);
            if (isStop)
                return;
        }
    }

public:
    ~UpdateProccessTimer() = default;

private:
    QAtomicInteger<bool> isStop { false };
};
class FileOperationsUtils
{
    friend class AbstractWorker;
    friend class DoCopyFilesWorker;
    friend class DoCutFilesWorker;
    friend class DoStatisticsFilesWorker;
    friend class DoMoveToTrashFilesWorker;
    friend class DoCleanTrashFilesWorker;
    friend class DoRestoreTrashFilesWorker;
    friend class FileOperateBaseWorker;

private:
    static SizeInfoPointer statisticsFilesSize(const QList<QUrl> &files, const bool &isRecordUrl = false);
    static bool isFilesSizeOutLimit(const QUrl &url, const qint64 limitSize);
    static void statisticFilesSize(const QUrl &url, SizeInfoPointer &sizeInfo, const bool &isRecordUrl = false);
    static bool isAncestorUrl(const QUrl &from, const QUrl &to);
    static bool isFileOnDisk(const QUrl &url);
    static void getDirFiles(const QUrl &url, QList<QUrl> &files);
};

DSC_END_NAMESPACE

#endif   // FILEOPERATIONSUTILS_H
