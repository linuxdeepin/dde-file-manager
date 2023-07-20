// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "reportlogmanager.h"
#include "reportlogworker.h"

#include <QThread>
#include <QUrl>
#include <QDebug>

DPUTILS_USE_NAMESPACE

ReportLogManager *ReportLogManager::instance()
{
    static ReportLogManager ins;
    return &ins;
}

ReportLogManager::ReportLogManager(QObject *parent)
    : QObject (parent)
{
}

ReportLogManager::~ReportLogManager()
{
    if (reportWorkThread) {
        qInfo() << "Log thread start to quit";
        reportWorkThread->quit();
        reportWorkThread->wait(2000);
        qInfo() << "Log thread quited.";
    }
}

void ReportLogManager::init()
{
    reportWorker = new ReportLogWorker();
    if (!reportWorker->init()) {
        reportWorker->deleteLater();
        return;
    }

    reportWorkThread = new QThread();
    connect(reportWorkThread, &QThread::finished, [&]() {
        reportWorker->deleteLater();
    });
    reportWorker->moveToThread(reportWorkThread);

    initConnection();

    reportWorkThread->start();
}

void ReportLogManager::commit(const QString &type, const QVariantMap &args)
{
    Q_EMIT requestCommitLog(type, args);
}

void ReportLogManager::reportMenuData(const QString &name, const QList<QUrl> &urlList)
{
    Q_EMIT requestReportMenuData(name, urlList);
}

void ReportLogManager::reportNetworkMountData(bool ret, dfmmount::DeviceError err, const QString &msg)
{
    Q_EMIT requestReportNetworkMountData(ret, err, msg);
}

void ReportLogManager::reportBlockMountData(const QString &id, bool result)
{
    Q_EMIT requestReportBlockMountData(id, result);
}

void ReportLogManager::initConnection()
{
    connect(this, &ReportLogManager::requestCommitLog, reportWorker, &ReportLogWorker::commitLog, Qt::QueuedConnection);
    connect(this, &ReportLogManager::requestReportMenuData, reportWorker, &ReportLogWorker::handleMenuData, Qt::QueuedConnection);
    connect(this, &ReportLogManager::requestReportNetworkMountData, reportWorker, &ReportLogWorker::handleMountNetworkResult, Qt::QueuedConnection);
    connect(this, &ReportLogManager::requestReportBlockMountData, reportWorker, &ReportLogWorker::handleBlockMountData, Qt::QueuedConnection);
}
