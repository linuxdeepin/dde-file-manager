// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "units/smbvirtualentryupgradeunit.h"
#include "units/beans/virtualentrydata.h"

#include <dfm-base/base/db/sqlitehandle.h>
#include <dfm-base/dfm_global_defines.h>

#include <QTemporaryDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

using namespace dfm_upgrade;
DFMBASE_USE_NAMESPACE

class SmbVirtualEntryUpgradeUnitTest : public testing::Test
{
protected:
    void SetUp() override
    {
        dir.reset(new QTemporaryDir());
        ASSERT_TRUE(dir->isValid());
        // both the migrated config file and the runtime database live at fixed
        // sandbox paths -> wipe them so results do not accumulate across tests
        QFile::remove(configPath());
        QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
             + "/deepin/dde-file-manager/database").removeRecursively();
        unit = new SmbVirtualEntryUpgradeUnit();
    }

    void TearDown() override
    {
        delete unit;   // deletes the handler if any
        unit = nullptr;
        dir.reset();
    }

    // the migrated config file lives in the sandbox config dir
    QString configPath() const
    {
        return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                + "/deepin/dde-file-manager.json";
    }

    bool writeConfig(const QJsonObject &root)
    {
        QFile f(configPath());
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
            return false;
        f.write(QJsonDocument(root).toJson());
        f.close();
        return true;
    }

    QJsonObject readConfig()
    {
        QFile f(configPath());
        if (!f.open(QIODevice::ReadOnly))
            return {};
        return QJsonDocument::fromJson(f.readAll()).object();
    }

    std::unique_ptr<QTemporaryDir> dir;
    SmbVirtualEntryUpgradeUnit *unit { nullptr };
};

TEST_F(SmbVirtualEntryUpgradeUnitTest, Name_And_Initialize_BasicContract)
{
    // Arrange
    QMap<QString, QString> args;

    // Act
    const QString name = unit->name();
    const bool inited = unit->initialize(args);

    // Assert
    EXPECT_EQ(name, QString("SmbVirtualEntryUpgradeUnit"));
    EXPECT_TRUE(inited);
}

TEST_F(SmbVirtualEntryUpgradeUnitTest, CreateDB_CreatesDatabaseInConfigDir)
{
    // Arrange / Act
    const bool ok = unit->createDB();

    // Assert
    EXPECT_TRUE(ok);
    ASSERT_NE(unit->handler, nullptr);
    const QString dbFile = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
            + "/deepin/dde-file-manager/database/"
            + DFMBASE_NAMESPACE::Global::DataBase::kDfmDBName;
    EXPECT_TRUE(QFile::exists(dbFile));
}

TEST_F(SmbVirtualEntryUpgradeUnitTest, CreateTable_CreatesVirtualEntryTable)
{
    // Arrange
    ASSERT_TRUE(unit->createDB());

    // Act
    const bool ok = unit->createTable();

    // Assert
    EXPECT_TRUE(ok);
    const auto beans = unit->handler->query<VirtualEntryData>().toBeans();
    EXPECT_TRUE(beans.isEmpty());
}

TEST_F(SmbVirtualEntryUpgradeUnitTest, ConvertFromMap_CompleteAndIncompleteData)
{
    // Arrange
    QVariantMap complete;
    complete.insert("protocol", "smb");
    complete.insert("host", "192.168.1.10");
    complete.insert("share", "public");
    complete.insert("name", "NAS");

    QVariantMap incomplete;
    incomplete.insert("protocol", "smb");

    // Act
    const VirtualEntryData good = unit->convertFromMap(complete);
    const VirtualEntryData bad = unit->convertFromMap(incomplete);

    // Assert
    EXPECT_EQ(good.getKey(), QString("smb://192.168.1.10/public/"));
    EXPECT_EQ(good.getHost(), QString("192.168.1.10"));
    EXPECT_EQ(good.getDisplayName(), QString("NAS"));
    EXPECT_TRUE(bad.getKey().isEmpty());
    EXPECT_TRUE(bad.getHost().isEmpty());
}

TEST_F(SmbVirtualEntryUpgradeUnitTest, ReadOldItems_NoConfigOrBadJson_ReturnsEmpty)
{
    // Arrange: no config file at all
    QFile::remove(configPath());

    // Act
    EXPECT_TRUE(unit->readOldItems().isEmpty());

    // Arrange: invalid json
    QFile bad(configPath());
    ASSERT_TRUE(bad.open(QIODevice::WriteOnly | QIODevice::Truncate));
    bad.write("this is not json");
    bad.close();

    // Act & Assert
    EXPECT_TRUE(unit->readOldItems().isEmpty());
}

TEST_F(SmbVirtualEntryUpgradeUnitTest, ReadOldItems_ParsesRemoteMountsAndStashedDevices)
{
    // Arrange
    QJsonObject remoteMount;
    QJsonObject entry;
    entry.insert("protocol", "smb");
    entry.insert("host", "192.168.1.20");
    entry.insert("share", "media");
    entry.insert("name", "Movies");
    remoteMount.insert("item1", entry);
    remoteMount.insert("notAnObject", QString("garbage"));   // skipped
    QJsonObject emptyHost;
    emptyHost.insert("protocol", "smb");
    remoteMount.insert("item2", emptyHost);   // skipped (no host)

    QJsonArray integrations;
    integrations.append(QString("smb://user@10.0.0.5/docs"));
    integrations.append(QString(""));   // skipped
    QJsonObject stashed;
    stashed.insert("SmbIntegrations", integrations);

    QJsonObject root;
    root.insert("RemoteMounts", remoteMount);
    root.insert("StashedSmbDevices", stashed);
    ASSERT_TRUE(writeConfig(root));

    // Act
    const QList<VirtualEntryData> items = unit->readOldItems();

    // Assert
    ASSERT_EQ(items.size(), 2);
    EXPECT_EQ(items.at(0).getKey(), QString("smb://192.168.1.20/media/"));
    EXPECT_EQ(items.at(0).getDisplayName(), QString("Movies"));
    EXPECT_EQ(items.at(1).getKey(), QString("smb://user@10.0.0.5/docs"));
}

TEST_F(SmbVirtualEntryUpgradeUnitTest, ClearOldItems_RemovesLegacyKeysKeepsOthers)
{
    // Arrange
    QJsonObject root;
    root.insert("RemoteMounts", QJsonObject());
    root.insert("StashedSmbDevices", QJsonObject());
    root.insert("GenericAttribute", QJsonObject({ { "keep", 1 } }));
    ASSERT_TRUE(writeConfig(root));

    // Act
    unit->clearOldItems();

    // Assert
    const QJsonObject after = readConfig();
    EXPECT_FALSE(after.contains("RemoteMounts"));
    EXPECT_FALSE(after.contains("StashedSmbDevices"));
    EXPECT_TRUE(after.contains("GenericAttribute"));
}

TEST_F(SmbVirtualEntryUpgradeUnitTest, SaveToDb_InsertsAllEntries)
{
    // Arrange
    ASSERT_TRUE(unit->createDB());
    QList<VirtualEntryData> entries;
    VirtualEntryData first;
    first.setKey("smb://a/b");
    first.setHost("a");
    VirtualEntryData second;
    second.setKey("smb://c/d");
    entries << first << second;

    // Act
    unit->saveToDb(entries);

    // Assert
    const auto beans = unit->handler->query<VirtualEntryData>().toBeans();
    ASSERT_EQ(beans.size(), 2);
}

TEST_F(SmbVirtualEntryUpgradeUnitTest, Upgrade_FullFlow_MigratesAndClearsConfig)
{
    // Arrange
    QJsonObject remoteMount;
    QJsonObject entry;
    entry.insert("protocol", "smb");
    entry.insert("host", "192.168.1.30");
    entry.insert("share", "share");
    entry.insert("name", "Share");
    remoteMount.insert("e", entry);
    QJsonObject root;
    root.insert("RemoteMounts", remoteMount);
    root.insert("OtherKey", QString("untouched"));
    ASSERT_TRUE(writeConfig(root));

    // Act
    const bool ok = unit->upgrade();

    // Assert
    EXPECT_TRUE(ok);
    ASSERT_NE(unit->handler, nullptr);
    EXPECT_EQ(unit->handler->query<VirtualEntryData>().toBeans().size(), 1);
    const QJsonObject after = readConfig();
    EXPECT_FALSE(after.contains("RemoteMounts"));
    EXPECT_EQ(after.value("OtherKey").toString(), QString("untouched"));
}
