// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sqlitemaster.h"

SqliteMaster::SqliteMaster(QObject *parent)
    : QObject(parent)
{
}

QString SqliteMaster::getType() const
{
    return type;
}

void SqliteMaster::setType(const QString &value)
{
    type = value;
}

QString SqliteMaster::getName() const
{
    return name;
}

void SqliteMaster::setName(const QString &value)
{
    name = value;
}

QString SqliteMaster::getTbl_name() const
{
    return tbl_name;
}

void SqliteMaster::setTbl_name(const QString &value)
{
    tbl_name = value;
}

int SqliteMaster::getRootpage() const
{
    return rootpage;
}

void SqliteMaster::setRootpage(int value)
{
    rootpage = value;
}

QString SqliteMaster::getSql() const
{
    return sql;
}

void SqliteMaster::setSql(const QString &value)
{
    sql = value;
}
