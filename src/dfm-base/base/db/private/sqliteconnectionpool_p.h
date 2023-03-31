// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SQLITECONNECTIONPOOL_P_H
#define SQLITECONNECTIONPOOL_P_H

#include <dfm-base/dfm_base_global.h>

#include <QString>
#include <QtSql>

DFMBASE_BEGIN_NAMESPACE

class SqliteConnectionPoolPrivate
{
public:
    SqliteConnectionPoolPrivate();
    QString makeConnectionName(const QString &databaseName);
    QSqlDatabase createConnection(const QString &databaseName, const QString &connectionName);

public:
    QString connectionName;
};

DFMBASE_END_NAMESPACE

#endif   // SQLITECONNECTIONPOOL_P_H
