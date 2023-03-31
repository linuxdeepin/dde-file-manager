// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <dfm-base/base/db/sqliteconnectionpool.h>
#include <dfm-base/base/db/private/sqliteconnectionpool_p.h>

#include <QCryptographicHash>
#include <QtConcurrent>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE

class UT_SqliteConnectionPool : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

public:
    stub_ext::StubExt stub;
    QString test_conn_name { "_test_conn_name_" };
    QString test_open_name { "/tmp/_test_open_name_" };
};

TEST_F(UT_SqliteConnectionPool, private_makeConnectionName)
{
    SqliteConnectionPoolPrivate privateObj;
    QString databaseName { "/home/zhangs/test_conn.db" };   // fake
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(databaseName.toLocal8Bit());
    QString result { QString(hash.result().toHex()) };
    EXPECT_EQ(result, privateObj.makeConnectionName(databaseName));
}

TEST_F(UT_SqliteConnectionPool, private_createConnection)
{
    SqliteConnectionPoolPrivate privateObj;

    auto addDatabase = static_cast<QSqlDatabase (*)(const QString &, const QString &)>(QSqlDatabase::addDatabase);
    auto open = static_cast<bool (QSqlDatabase::*)()>(&QSqlDatabase::open);
    QSqlDatabase mockDb = QSqlDatabase::addDatabase("QSQLITE", test_conn_name);

    // can open
    stub.set_lamda(addDatabase, [mockDb]() { __DBG_STUB_INVOKE__ return mockDb; });
    stub.set_lamda(open, []() { __DBG_STUB_INVOKE__ return true; });
    auto db = privateObj.createConnection("database", "connection");
    EXPECT_EQ(db.connectionName(), test_conn_name);

    // can't open
    stub.set_lamda(open, []() { __DBG_STUB_INVOKE__ return false; });
    db = privateObj.createConnection("database", "connection");
    EXPECT_TRUE(db.connectionName().isEmpty());
    stub.clear();
}

TEST_F(UT_SqliteConnectionPool, instance)
{
    EXPECT_NO_FATAL_FAILURE(SqliteConnectionPool::instance());
}

TEST_F(UT_SqliteConnectionPool, openConnection_NotContains)
{
    SqliteConnectionPoolPrivate privateObj;

    QSqlDatabase mockDb = QSqlDatabase::addDatabase("QSQLITE", test_conn_name);
    stub.set_lamda(ADDR(SqliteConnectionPoolPrivate, createConnection), [mockDb]() { __DBG_STUB_INVOKE__ return mockDb; });
    auto db = SqliteConnectionPool::instance().openConnection(test_open_name);
    EXPECT_EQ(db.connectionName(), test_conn_name);
    EXPECT_TRUE(QSqlDatabase::contains(test_conn_name));
    QSqlDatabase::removeDatabase(test_conn_name);
    stub.clear();
}

TEST_F(UT_SqliteConnectionPool, openConnection_SubThread)
{
    SqliteConnectionPoolPrivate privateObj;
    QString baseConnectionName = "conn_" + QString::number(quint64(QThread::currentThread()), 16);
    QString fullConnectionName = baseConnectionName + "_" + privateObj.makeConnectionName(test_open_name);

    stub.set_lamda(ADDR(SqliteConnectionPoolPrivate, createConnection),
                   [&](SqliteConnectionPoolPrivate *, const QString &, const QString &connectionName) {
                       __DBG_STUB_INVOKE__
                       return QSqlDatabase::addDatabase("QSQLITE", connectionName);
                   });
    auto future = QtConcurrent::run([fullConnectionName, this] {
        auto db = SqliteConnectionPool::instance().openConnection(test_open_name);
        EXPECT_TRUE(QSqlDatabase::contains(db.connectionName()));
        QSqlDatabase::removeDatabase(db.connectionName());
    });

    // clear connect after thread exit
    future.waitForFinished();
    //    EXPECT_FALSE(QSqlDatabase::contains(fullConnectionName));
    stub.clear();
}

TEST_F(UT_SqliteConnectionPool, openConnection_Contains)
{
    SqliteConnectionPoolPrivate privateObj;

    QString baseConnectionName = "conn_" + QString::number(quint64(QThread::currentThread()), 16);
    QString fullConnectionName = baseConnectionName + "_" + privateObj.makeConnectionName(test_open_name);
    QSqlDatabase mockDb = QSqlDatabase::addDatabase("QSQLITE", fullConnectionName);
    stub.set_lamda(ADDR(SqliteConnectionPoolPrivate, createConnection), [mockDb]() { __DBG_STUB_INVOKE__ return mockDb; });
    auto db = SqliteConnectionPool::instance().openConnection(test_open_name);
    EXPECT_EQ(db.connectionName(), fullConnectionName);

    auto open = static_cast<bool (QSqlDatabase::*)()>(&QSqlDatabase::open);
    stub.set_lamda(open, []() { __DBG_STUB_INVOKE__ return true; });

    db = SqliteConnectionPool::instance().openConnection(test_open_name);
    EXPECT_EQ(db.connectionName(), fullConnectionName);
    QSqlDatabase::removeDatabase(fullConnectionName);
    stub.clear();
}
