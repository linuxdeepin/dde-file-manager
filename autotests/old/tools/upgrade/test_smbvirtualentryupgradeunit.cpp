// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/smbvirtualentryupgradeunit.h"
#include "units/beans/virtualentrydata.h"

#include "stubext.h"

#include <gtest/gtest.h>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QList>
#include <QMap>
#include <QSqlDatabase>

using namespace dfm_upgrade;

class TestSmbVirtualEntryUpgradeUnit : public testing::Test {
protected:
    void SetUp() override {
        testDir = QDir::tempPath() + "/smb_virtual_entry_test";
        QDir().mkpath(testDir);

        // Mock QStandardPaths::writableLocation to use test directory
        auto writableLocationFunc = static_cast<QString (*)(QStandardPaths::StandardLocation)>(&QStandardPaths::writableLocation);
        stub.set_lamda(writableLocationFunc, [&](QStandardPaths::StandardLocation type) -> QString {
            return testDir;
        });
    }

    void TearDown() override {
        QDir(testDir).removeRecursively();
        stub.clear();
    }

    QString testDir;
    stub_ext::StubExt stub;
};

TEST_F(TestSmbVirtualEntryUpgradeUnit, name)
{
    SmbVirtualEntryUpgradeUnit unit;
    EXPECT_EQ(unit.name(), QString("SmbVirtualEntryUpgradeUnit"));
}

TEST_F(TestSmbVirtualEntryUpgradeUnit, initialize)
{
    SmbVirtualEntryUpgradeUnit unit;
    QMap<QString, QString> args;
    bool result = unit.initialize(args);
    EXPECT_TRUE(result);
}

// Add tests for SmbVirtualEntryUpgradeUnit methods
TEST_F(TestSmbVirtualEntryUpgradeUnit, upgrade)
{
    // Mock SqliteConnectionPool operations
    auto openConnectionFunc = [](const QString &dbPath) -> QSqlDatabase {
        Q_UNUSED(dbPath);
        static QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_connection_pool");
        db.setDatabaseName(":memory:");  // Use in-memory database for tests
        if (!db.isOpen())
            db.open();
        return db;
    };

    SmbVirtualEntryUpgradeUnit unit;
    QMap<QString, QString> args;
    unit.initialize(args);
    bool result = unit.upgrade();
    EXPECT_TRUE(result);
}

TEST_F(TestSmbVirtualEntryUpgradeUnit, completed)
{
    SmbVirtualEntryUpgradeUnit unit;
    unit.completed();  // Should not crash or throw
    SUCCEED();  // Just verify that the method can be called without issues
}
