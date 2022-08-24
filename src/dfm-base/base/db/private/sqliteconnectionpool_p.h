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
#ifndef SQLITECONNECTIONPOOL_P_H
#define SQLITECONNECTIONPOOL_P_H

#include "dfm-base/dfm_base_global.h"

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
