/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
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
#ifndef SQLITECONNECTIONPOOL_H
#define SQLITECONNECTIONPOOL_H

#include "dfm-base/dfm_base_global.h"

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
