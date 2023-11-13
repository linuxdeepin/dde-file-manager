// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sqliteconnectionpool.h"
#include <dfm-base/base/db/private/sqliteconnectionpool_p.h>

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
        qCInfo(logDFMBase).noquote() << QString("Connection created: %1, sn: %2").arg(connectionName).arg(++sn);
        return db;
    } else {
        qCWarning(logDFMBase).noquote() << "Create connection error:" << db.lastError().text();
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
        qCDebug(logDFMBase).noquote() << QString("Test connection on borrow, execute: %1, for connection %2")
                                                 .arg(kTestSql)
                                                 .arg(fullConnectionName);
        QSqlQuery query(kTestSql, existingDb);
        if (query.lastError().type() != QSqlError::NoError && !existingDb.open()) {
            qCCritical(logDFMBase).noquote() << "Open datatabase error:" << existingDb.lastError().text();
            return QSqlDatabase();
        }
        return existingDb;
    } else {
        if (qApp != nullptr) {
            QObject::connect(QThread::currentThread(), &QThread::finished, qApp, [fullConnectionName] {
                if (QSqlDatabase::contains(fullConnectionName)) {
                    QSqlDatabase::removeDatabase(fullConnectionName);
                    qCInfo(logDFMBase).noquote() << QString("Connection deleted: %1").arg(fullConnectionName);
                }
            });
        }

        return d->createConnection(databaseName, fullConnectionName);
    }
}
