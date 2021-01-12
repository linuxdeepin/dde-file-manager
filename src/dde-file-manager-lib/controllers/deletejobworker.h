/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef DELETEJOBWORKER_H
#define DELETEJOBWORKER_H

#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QMap>
#include <QString>
class DeleteJobInterface;


class DeletejobWorker : public QObject
{
    Q_OBJECT
public:
    explicit DeletejobWorker(const QStringList& files, QObject *parent = 0);
    ~DeletejobWorker();
    void initConnect();

signals:
    void startJob();
    void finished();

public slots:
    void start();
    void deleteFiles(const QStringList& files);
    void connectDeleteJobSignal();
    void disconnectDeleteJobSignal();
    void deleteJobExcuteFinished();
    void deleteJobAbort();
    void deleteJobAbortFinished();
    void onDeletingFile(QString file);
    void setTotalAmount(qlonglong amount, ushort type);
    void onDeletingProcessAmount(qlonglong progress, ushort info);
    void onProcessedPercent(qlonglong percent);
    void handleTimeout();
    void handleFinished();
    void handleTaskAborted(const QMap<QString, QString>& jobDetail);

private:
    DeleteJobInterface* m_deleteJobInterface = NULL;
    QStringList m_deletefiles;

    QString m_deletejobPath;
    QTimer* m_progressTimer;
    QMap<QString, QString> m_jobDetail;
    QMap<QString, QString> m_jobDataDetail;
    qlonglong m_totalAmout;
    QTime* m_time;
    qlonglong m_lastProgress = 0;
    qlonglong m_currentProgress = 0;
    qlonglong m_processedPercent = 0;
    int m_elapsedTime;
};

#endif // DELETEJOBWORKER_H
