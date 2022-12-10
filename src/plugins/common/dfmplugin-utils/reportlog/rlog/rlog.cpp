// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rlog.h"
#include "committhread.h"

#include "datas/blockmountreportdata.h"
#include "datas/smbreportdata.h"
#include "datas/vaultreportdata.h"
#include "datas/searchreportdata.h"
#include "datas/sidebarreportdata.h"
#include "datas/filemenureportdata.h"
#include "datas/appstartupreportdata.h"
#include "datas/reportdatainterface.h"

#include <QApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QThread>

using namespace dfmplugin_utils;

RLog::RLog(QObject *parent)
    : QObject(parent)
{
}
class RLog_ : public RLog
{
};

void RLog::commit(const QString &type, const QVariantMap &args)
{
    static std::once_flag flg;
    std::call_once(flg, []() {
        RLog::instance()->init();
    });

    ReportDataInterface *interface = logDataObj.value(type, nullptr);
    if (!interface) {
        qInfo() << "Error: Log data object is not registed.";
        return;
    }
    QJsonObject jsonObject = interface->prepareData(args);

    const QStringList &keys = commonData.keys();
    foreach (const QString &key, keys) {
        jsonObject.insert(key, commonData.value(key));   //add common data for each log commit
    }
    emit appendArgs(jsonObject.toVariantHash());   //Send log data to the thread
}

RLog *RLog::instance()
{
    static RLog ins;
    return &ins;
}

RLog::~RLog()
{
    qInfo() << "RLog start destroy";
    qDeleteAll(logDataObj.begin(), logDataObj.end());
    logDataObj.clear();
    if (commitThread) {
        qInfo() << "Log thread start to quit";
        commitThread->quit();
        commitThread->wait(2000);
        qInfo() << "Log thread quited.";
    }
    qInfo() << "RLog already destroy";
}

void RLog::init()
{
    qInfo() << " - start register RLog data";
    QList<ReportDataInterface *> datas {
        new BlockMountReportData,
        new SmbReportData,
        new SidebarReportData,
        new SearchReportData,
        new VaultReportData,
        new FileMenuReportData,
        new AppStartupReportData
    };

    std::for_each(datas.cbegin(), datas.cend(), [this](ReportDataInterface *dat) { registerLogData(dat->type(), dat); });
    qInfo() << " - end register RLog data";

    qInfo() << " - start init RLog";
    commonData.insert("dfmVersion", QApplication::applicationVersion());

    commitLog = new CommitLog();
    if (!commitLog->init())
        return;

    commitThread = new QThread();
    connect(this, &RLog::appendArgs, commitLog, &CommitLog::commit);
    connect(commitThread, &QThread::finished, [&]() {
        commitLog->deleteLater();
    });
    commitLog->moveToThread(commitThread);
    commitThread->start();

    qInfo() << " - end init RLog, commit thread is started.";
}

bool RLog::registerLogData(const QString &type, ReportDataInterface *dataObj)
{
    if (logDataObj.contains(type)) {
        qInfo() << "Log type already existed.";
        return false;
    }
    logDataObj.insert(type, dataObj);
    return true;
}
