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
#include "sqliteconnectionpool.h"
#include "dfm-base/base/db/private/sqliteconnectionpool_p.h"

#include <QDebug>
#include <QString>
#include <QThread>
#include <QCoreApplication>
#include <QCryptographicHash>

DFMBASE_USE_NAMESPACE

static constexpr char kDatabaseType[] { "QSQLITE" };
static constexpr char kTestSql[] { "SELECT 1" };

SqliteConnectionPoolPrivate::SqliteConnectionPoolPrivate()
{
}

QString SqliteConnectionPoolPrivate::makeConnectionName(const QString &databaseName)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(databaseName.toLocal8Bit());
    return QString(hash.result().toHex());
}

QSqlDatabase SqliteConnectionPoolPrivate::createConnection(const QString &databaseName, const QString &connectionName)
{
    static int sn = 0;

    QSqlDatabase db = QSqlDatabase::addDatabase(kDatabaseType, connectionName);
    db.setDatabaseName(databaseName);

    if (db.open()) {
        qInfo().noquote() << QString("Connection created: %1, sn: %2").arg(connectionName).arg(++sn);
        return db;
    } else {
        qWarning().noquote() << "Create connection error:" << db.lastError().text();
        return QSqlDatabase();
    }
}

SqliteConnectionPool::SqliteConnectionPool(QObject *parent)
    : QObject(parent), d(new SqliteConnectionPoolPrivate)
{
}

SqliteConnectionPool::~SqliteConnectionPool()
{
}

SqliteConnectionPool &SqliteConnectionPool::instance()
{
    static SqliteConnectionPool ins;
    return ins;
}

QSqlDatabase SqliteConnectionPool::openConnection(const QString &databaseName)
{
    assert(!databaseName.isEmpty());
    assert(QUrl::fromLocalFile(databaseName).isValid());

    QString baseConnectionName = "conn_" + QString::number(quint64(QThread::currentThread()), 16);
    QString fullConnectionName = baseConnectionName + "_" + d->makeConnectionName(databaseName);

    if (QSqlDatabase::contains(fullConnectionName)) {
        QSqlDatabase existingDb = QSqlDatabase::database(fullConnectionName);
        qDebug().noquote() << QString("Test connection on borrow, execute: %1, for connection %2")
                                      .arg(kTestSql)
                                      .arg(fullConnectionName);
        QSqlQuery query(kTestSql, existingDb);
        if (query.lastError().type() != QSqlError::NoError && !existingDb.open()) {
            qCritical().noquote() << "Open datatabase error:" << existingDb.lastError().text();
            return QSqlDatabase();
        }
        return existingDb;
    } else {
        if (qApp != nullptr) {
            QObject::connect(QThread::currentThread(), &QThread::finished, qApp, [fullConnectionName] {
                if (QSqlDatabase::contains(fullConnectionName)) {
                    QSqlDatabase::removeDatabase(fullConnectionName);
                    qInfo().noquote() << QString("Connection deleted: %1").arg(fullConnectionName);
                }
            });
        }

        return d->createConnection(databaseName, fullConnectionName);
    }
}
