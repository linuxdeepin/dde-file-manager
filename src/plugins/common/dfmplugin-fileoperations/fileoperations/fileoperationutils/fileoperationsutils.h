// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEOPERATIONSUTILS_H
#define FILEOPERATIONSUTILS_H

#include "dfmplugin_fileoperations_global.h"
#include <dfm-base/utils/fileutils.h>

#include <QSharedPointer>
#include <QThread>
#include <QTimer>
#include <QPointer>

DPFILEOPERATIONS_BEGIN_NAMESPACE
class UpdateProgressTimer : public QObject
{
    Q_OBJECT
    friend class AbstractWorker;
    friend class DoCopyFilesWorker;
    explicit UpdateProgressTimer(QObject *parent = nullptr)
        : QObject(parent) {}
    void stopTimer()
    {
        isStop = true;
    }
signals:
    void updateProgressNotify();
private slots:
    void handleTimeOut()
    {
        if (Q_UNLIKELY(isStop)) {
            timer->stop();
        } else {
            emit updateProgressNotify();
        }
    }

    void doStartTime()
    {
        if (!timer)
            timer = new QTimer;
        connect(timer, &QTimer::timeout, this, &UpdateProgressTimer::handleTimeOut, Qt::ConnectionType(Qt::DirectConnection | Qt::UniqueConnection));
        timer->start(500);
    }

public:
    ~UpdateProgressTimer()
    {
        if (timer)
            timer->deleteLater();
    }

private:
    QAtomicInteger<bool> isStop { false };
    QPointer<QTimer> timer { nullptr };
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
    friend class ErrorMessageAndAction;

private:
    static SizeInfoPointer statisticsFilesSize(const QList<QUrl> &files, const bool &isRecordUrl = false);
    static bool isFilesSizeOutLimit(const QUrl &url, const qint64 limitSize);
    static void statisticFilesSize(const QUrl &url, SizeInfoPointer &sizeInfo, const bool &isRecordUrl = false);
    static bool isAncestorUrl(const QUrl &from, const QUrl &to);
    static bool isFileOnDisk(const QUrl &url);
    static qint64 bigFileSize();
    static bool blockSync();
    static QUrl parentUrl(const QUrl &url);
    static bool canBroadcastPaste();

private:
    static QSet<QString> fileNameUsing;
    static QMutex mutex;
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // FILEOPERATIONSUTILS_H
