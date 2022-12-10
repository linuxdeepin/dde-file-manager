// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RLOG_H
#define RLOG_H

#include <QObject>
#include <QHash>
#include <QMap>
#include <QVariantMap>
#include <QJsonObject>

namespace dfmplugin_utils {

class ReportDataInterface;
class CommitLog;
class RLog : public QObject
{
    Q_OBJECT
public:
    void commit(const QString &type, const QVariantMap &args);
    static RLog *instance();
    void init();
    bool registerLogData(const QString &type, ReportDataInterface *dataObj);
    ~RLog();

protected:
    explicit RLog(QObject *parent = nullptr);

signals:
    void appendArgs(const QVariant &args);

private:
    QJsonObject commonData;
    QHash<QString, ReportDataInterface *> logDataObj;
    CommitLog *commitLog = nullptr;
    QThread *commitThread = nullptr;
};

}

#endif   // RLOG_H
