// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RLOG_H
#define RLOG_H

#include "smbreportdata.h"
#include "vaultreportdata.h"
#include "searchreportdata.h"

#include <QObject>
#include <QHash>
#include <QMap>
#include <QVariantMap>
#include <QJsonObject>

class ReportDataInterface;
class CommitLog;
class RLog : public QObject
{
    Q_OBJECT
public:
    void commit(const QString &type, const QVariantMap& args);
    static RLog* instance();
    void init();
    bool registerLogData(const QString &type, ReportDataInterface *dataObj);
    ~RLog();
protected:
    explicit RLog(QObject *parent = nullptr);

signals:
    void appendArgs(const QVariant &args);

private:
    QJsonObject m_commonData;
    QHash<QString, ReportDataInterface *> m_logDataObj;
    CommitLog *m_commitLog = nullptr;
    QThread* m_commitThread = nullptr;
    bool m_isInit = false;
};

#endif // RLOG_H
