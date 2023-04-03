// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SQLITECONNECTIONPOOL_H
#define SQLITECONNECTIONPOOL_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QtSql>

DFMBASE_BEGIN_NAMESPACE

class SqliteConnectionPoolPrivate;
class SqliteConnectionPool : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SqliteConnectionPool)

public:
    static SqliteConnectionPool &instance();
    QSqlDatabase openConnection(const QString &databaseName);

private:
    explicit SqliteConnectionPool(QObject *parent = nullptr);
    ~SqliteConnectionPool();

private:
    QScopedPointer<SqliteConnectionPoolPrivate> d;
};

DFMBASE_END_NAMESPACE

#endif   // SQLITECONNECTIONPOOL_H
