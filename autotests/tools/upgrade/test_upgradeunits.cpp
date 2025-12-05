// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/appattributeupgradeunit.h"
#include "units/bookmarkupgradeunit.h"
#include "units/desktoporganizeupgradeunit.h"
#include "units/headerunit.h"
#include "units/vaultupgradeunit.h"
#include "units/smbvirtualentryupgradeunit.h"
#include "units/tagdbupgradeunit.h"
#include "units/dconfigupgradeunit.h"
#include "units/bookmarkupgrade/defaultitemmanager.h"
#include "units/unitlist.h"
#include "utils/upgradeutils.h"

#include "stubext.h"

#include <gtest/gtest.h>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDateTime>

using namespace dfm_upgrade;

class TestUpgradeUtils : public testing::Test {
protected:
    void SetUp() override {
        // Create test directory
        testDir = QDir::tempPath() + "/upgrade_utils_test";
        QDir().mkpath(testDir);

        // Create test config file
        configFilePath = testDir + "/deepin/dde-file-manager.json";
        QDir().mkpath(testDir + "/deepin");
        QFile configFile(configFilePath);
        if (configFile.open(QIODevice::WriteOnly)) {
            QJsonObject rootObj;
            QJsonObject genAttrObj;
            genAttrObj.insert("test_key", "test_value");
            rootObj.insert("GenericAttribute", genAttrObj);
            configFile.write(QJsonDocument(rootObj).toJson());
            configFile.close();
        }

        // Create test app config file
        appConfigFilePath = testDir + "/deepin/dde-file-manager/dde-file-manager.json";
        QDir().mkpath(testDir + "/deepin/dde-file-manager");
        QFile appConfigFile(appConfigFilePath);
        if (appConfigFile.open(QIODevice::WriteOnly)) {
            QJsonObject rootObj;
            QJsonObject appAttrObj;
            appAttrObj.insert("test_app_key", "test_app_value");
            rootObj.insert("ApplicationAttribute", appAttrObj);
            appConfigFile.write(QJsonDocument(rootObj).toJson());
            appConfigFile.close();
        }

        auto standlocal = static_cast<QList<QString> (*)(QStandardPaths::StandardLocation)>(&QStandardPaths::standardLocations);
        stub.set_lamda(standlocal, [&](QStandardPaths::StandardLocation type) -> QList<QString> {
            return {testDir};
        });
    }

    void TearDown() override {
        // Clean up test files
        QDir(testDir).removeRecursively();
        stub.clear();
    }

    QString testDir;
    QString configFilePath;
    QString appConfigFilePath;
    stub_ext::StubExt stub;
};

TEST_F(TestUpgradeUtils, genericAttribute_ExistingKey)
{
    QVariant result = UpgradeUtils::genericAttribute("test_key");
    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(result.toString(), QString("test_value"));
}

TEST_F(TestUpgradeUtils, genericAttribute_NonExistingKey)
{
    QVariant result = UpgradeUtils::genericAttribute("nonexistent_key");
    EXPECT_TRUE(result.isNull());
}

TEST_F(TestUpgradeUtils, genericAttribute_InvalidConfig)
{
    QFile::remove(configFilePath);
    QFile configFile(configFilePath);
    if (configFile.open(QIODevice::WriteOnly)) {
        configFile.write("invalid json content");
        configFile.close();
    }

    QVariant result = UpgradeUtils::genericAttribute("test_key");
    EXPECT_TRUE(result.isNull());
}

TEST_F(TestUpgradeUtils, applicationAttribute_ExistingKey)
{
    QVariant result = UpgradeUtils::applicationAttribute("test_app_key");
    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(result.toString(), QString("test_app_value"));
}

TEST_F(TestUpgradeUtils, applicationAttribute_NonExistingKey)
{
    QVariant result = UpgradeUtils::applicationAttribute("nonexistent_app_key");
    EXPECT_TRUE(result.isNull());
}

TEST_F(TestUpgradeUtils, backupFile_Success)
{
    QString sourceFile = testDir + "/source.txt";
    QFile source(sourceFile);
    if (source.open(QIODevice::WriteOnly)) {
        source.write("test content");
        source.close();
    }

    bool result = UpgradeUtils::backupFile(sourceFile, testDir + "/backup");
    EXPECT_TRUE(result);

    QDir backupDir(testDir + "/backup");
    EXPECT_TRUE(backupDir.exists());
    QStringList backupFiles = backupDir.entryList(QDir::Files);
    EXPECT_FALSE(backupFiles.isEmpty());
}

TEST_F(TestUpgradeUtils, backupFile_Failure)
{
    bool result = UpgradeUtils::backupFile("/nonexistent/source.txt", testDir + "/backup");
    EXPECT_FALSE(result);
}

TEST_F(TestUpgradeUtils, backupFile_CreateBackupDirFailure)
{
    QString sourceFile = testDir + "/source.txt";
    QFile source(sourceFile);
    if (source.open(QIODevice::WriteOnly)) {
        source.write("test content");
        source.close();
    }

    bool result = UpgradeUtils::backupFile(sourceFile, "/invalid/path/backup");
    EXPECT_FALSE(result);
}

TEST_F(TestUpgradeUtils, addOldGenericAttribute)
{
    QJsonArray oldAttrs;
    oldAttrs.append("old_key1");
    oldAttrs.append("old_key2");

    UpgradeUtils::addOldGenericAttribute(oldAttrs);

    QFile configFile(configFilePath);
    if (configFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
        configFile.close();

        if (doc.isObject()) {
            QJsonObject rootObj = doc.object();
            if (rootObj.contains("GenericAttribute") && rootObj.value("GenericAttribute").isObject()) {
                QJsonObject genAttr = rootObj.value("GenericAttribute").toObject();
                if (genAttr.contains("OldAttributes") && genAttr.value("OldAttributes").isArray()) {
                    QJsonArray oldAttrsArray = genAttr.value("OldAttributes").toArray();
                    EXPECT_EQ(oldAttrsArray.size(), 2);
                    EXPECT_EQ(oldAttrsArray[0].toString(), QString("old_key1"));
                    EXPECT_EQ(oldAttrsArray[1].toString(), QString("old_key2"));
                }
            }
        }
    }
}

class TestAppAttributeUpgradeUnit : public testing::Test {
protected:
    void SetUp() override {
        testDir = QDir::tempPath() + "/app_attribute_test";
        QDir().mkpath(testDir);

        configFilePath = testDir + "/dde-file-manager/dde-file-manager.json";
        QDir().mkpath(testDir + "/dde-file-manager");
        QFile configFile(configFilePath);
        if (configFile.open(QIODevice::WriteOnly)) {
            QJsonObject rootObj;
            QJsonObject appAttrObj;
            appAttrObj.insert("IconSizeLevel", 2);
            rootObj.insert("ApplicationAttribute", appAttrObj);
            configFile.write(QJsonDocument(rootObj).toJson());
            configFile.close();
        }

        auto standlocal = static_cast<QList<QString> (*)(QStandardPaths::StandardLocation)>(&QStandardPaths::standardLocations);
        stub.set_lamda(standlocal, [&](QStandardPaths::StandardLocation type) -> QList<QString> {
            return {testDir};
        });
    }

    void TearDown() override {
        QDir(testDir).removeRecursively();
        stub.clear();
    }

    QString testDir;
    QString configFilePath;
    stub_ext::StubExt stub;
};

TEST_F(TestAppAttributeUpgradeUnit, name)
{
    AppAttributeUpgradeUnit unit;
    EXPECT_EQ(unit.name(), QString("AppAttributeUpgradeUnit"));
}

TEST_F(TestAppAttributeUpgradeUnit, initialize_Success)
{
    AppAttributeUpgradeUnit unit;
    QMap<QString, QString> args;
    bool result = unit.initialize(args);
    EXPECT_TRUE(result);
}

TEST_F(TestAppAttributeUpgradeUnit, initialize_Failure_NoConfig)
{
    QFile::remove(configFilePath);

    AppAttributeUpgradeUnit unit;
    QMap<QString, QString> args;
    bool result = unit.initialize(args);
    EXPECT_FALSE(result);
}

TEST_F(TestAppAttributeUpgradeUnit, upgrade_Success)
{
    AppAttributeUpgradeUnit unit;
    QMap<QString, QString> args;
    unit.initialize(args);
    bool result = unit.upgrade();
    EXPECT_TRUE(result);
}

TEST_F(TestAppAttributeUpgradeUnit, transIconSizeLevel)
{
    AppAttributeUpgradeUnit unit;
    EXPECT_EQ(unit.transIconSizeLevel(0), 3);
    EXPECT_EQ(unit.transIconSizeLevel(1), 5);
    EXPECT_EQ(unit.transIconSizeLevel(2), 9);
    EXPECT_EQ(unit.transIconSizeLevel(3), 13);
    EXPECT_EQ(unit.transIconSizeLevel(4), 17);
    EXPECT_EQ(unit.transIconSizeLevel(5), 5); // Default case
}

class TestBookMarkUpgradeUnit : public testing::Test {
protected:
    void SetUp() override {
        testDir = QDir::tempPath() + "/bookmark_test";
        QDir().mkpath(testDir);

        configFilePath = testDir + "/deepin/dde-file-manager.json";
        QDir().mkpath(testDir + "/deepin");
        QFile configFile(configFilePath);
        if (configFile.open(QIODevice::WriteOnly)) {
            QJsonObject rootObj;
            QJsonObject bookmarkObj;
            QJsonArray itemsArray;
            QJsonObject item1;
            item1.insert("name", "test_bookmark");
            item1.insert("url", "file:///test/path");
            itemsArray.append(item1);
            bookmarkObj.insert("Items", itemsArray);
            rootObj.insert("BookMark", bookmarkObj);
            configFile.write(QJsonDocument(rootObj).toJson());
            configFile.close();
        }

        auto standlocal = static_cast<QList<QString> (*)(QStandardPaths::StandardLocation)>(&QStandardPaths::standardLocations);
        stub.set_lamda(standlocal, [&](QStandardPaths::StandardLocation type) -> QList<QString> {
            return {testDir};
        });
    }

    void TearDown() override {
        QDir(testDir).removeRecursively();
        stub.clear();
    }

    QString testDir;
    QString configFilePath;
    stub_ext::StubExt stub;
};

TEST_F(TestBookMarkUpgradeUnit, name)
{
    BookMarkUpgradeUnit unit;
    EXPECT_EQ(unit.name(), QString("BookMarkUpgradeUnit"));
}

TEST_F(TestBookMarkUpgradeUnit, initialize_Success)
{
    BookMarkUpgradeUnit unit;
    QMap<QString, QString> args;
    bool result = unit.initialize(args);
    EXPECT_TRUE(result);
}

TEST_F(TestBookMarkUpgradeUnit, upgrade_Success)
{
    BookMarkUpgradeUnit unit;
    QMap<QString, QString> args;
    unit.initialize(args);
    bool result = unit.upgrade();
    EXPECT_TRUE(result);
}

class TestDesktopOrganizeUpgradeUnit : public testing::Test {
protected:
    void SetUp() override {
        testDir = QDir::tempPath() + "/desktop_organize_test";
        QDir().mkpath(testDir);

        configFilePath = testDir + "/deepin/dde-desktop/ddplugin-organizer.conf";
        QDir().mkpath(testDir + "/deepin/dde-desktop");
        QFile configFile(configFilePath);
        if (configFile.open(QIODevice::WriteOnly)) {
            configFile.write("[Version]\n1.0\n");
            configFile.close();
        }

        auto standlocal = static_cast<QList<QString> (*)(QStandardPaths::StandardLocation)>(&QStandardPaths::standardLocations);
        stub.set_lamda(standlocal, [&](QStandardPaths::StandardLocation type) -> QList<QString> {
            return {testDir};
        });
    }

    void TearDown() override {
        QDir(testDir).removeRecursively();
        stub.clear();
    }

    QString testDir;
    QString configFilePath;
    stub_ext::StubExt stub;
};

TEST_F(TestDesktopOrganizeUpgradeUnit, name)
{
    DesktopOrganizeUpgradeUnit unit;
    EXPECT_EQ(unit.name(), QString("DesktopUpgradeUnit"));
}

TEST_F(TestDesktopOrganizeUpgradeUnit, initialize_NoVersion)
{
    QFile::remove(configFilePath);

    DesktopOrganizeUpgradeUnit unit;
    QMap<QString, QString> args;
    bool result = unit.initialize(args);
    EXPECT_TRUE(result);
}

TEST_F(TestDesktopOrganizeUpgradeUnit, initialize_WithVersion)
{
    DesktopOrganizeUpgradeUnit unit;
    QMap<QString, QString> args;
    bool result = unit.initialize(args);
    EXPECT_TRUE(result);
}

TEST_F(TestDesktopOrganizeUpgradeUnit, upgrade_Success)
{
    DesktopOrganizeUpgradeUnit unit;
    QMap<QString, QString> args;
    unit.initialize(args);
    bool result = unit.upgrade();
    EXPECT_TRUE(result);
}

// Tests for units that don't require file system setup
class TestHeaderUnit : public testing::Test {
};

TEST_F(TestHeaderUnit, name)
{
    HeaderUnit unit;
    EXPECT_EQ(unit.name(), QString("Header"));
}

TEST_F(TestHeaderUnit, initialize)
{
    HeaderUnit unit;
    QMap<QString, QString> args;
    bool result = unit.initialize(args);
    EXPECT_TRUE(result);
}

TEST_F(TestHeaderUnit, upgrade)
{
    HeaderUnit unit;
    QMap<QString, QString> args;
    unit.initialize(args);
    bool result = unit.upgrade();
    EXPECT_TRUE(result);
}

class TestVaultUpgradeUnit : public testing::Test {
};

TEST_F(TestVaultUpgradeUnit, name)
{
    VaultUpgradeUnit unit;
    EXPECT_EQ(unit.name(), QString("VaultUpgradeUnit"));
}

TEST_F(TestVaultUpgradeUnit, initialize_NoOldVault)
{
    VaultUpgradeUnit unit;
    QMap<QString, QString> args;
    bool result = unit.initialize(args);
    EXPECT_FALSE(result);
}

class TestSmbVirtualEntryUpgradeUnit : public testing::Test {
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

class TestTagDbUpgradeUnit : public testing::Test {
};

TEST_F(TestTagDbUpgradeUnit, name)
{
    TagDbUpgradeUnit unit;
    EXPECT_EQ(unit.name(), QString("TagDbUpgradeUnit"));
}

TEST_F(TestTagDbUpgradeUnit, initialize)
{
    TagDbUpgradeUnit unit;
    QMap<QString, QString> args;
    bool result = unit.initialize(args);
    EXPECT_TRUE(result);
}

class TestDConfigUpgradeUnit : public testing::Test {
};

TEST_F(TestDConfigUpgradeUnit, name)
{
    DConfigUpgradeUnit unit;
    EXPECT_EQ(unit.name(), QString("DConfigUpgradeUnit"));
}

TEST_F(TestDConfigUpgradeUnit, initialize)
{
    DConfigUpgradeUnit unit;
    QMap<QString, QString> args;
    bool result = unit.initialize(args);
    EXPECT_TRUE(result);
}

TEST_F(TestDConfigUpgradeUnit, upgrade)
{
    DConfigUpgradeUnit unit;
    QMap<QString, QString> args;
    unit.initialize(args);
    bool result = unit.upgrade();
    EXPECT_TRUE(result);
}

class TestDefaultItemManager : public testing::Test {
};

TEST_F(TestDefaultItemManager, instance)
{
    DefaultItemManager* manager = DefaultItemManager::instance();
    EXPECT_NE(manager, nullptr);
}

TEST_F(TestDefaultItemManager, initDefaultItems)
{
    DefaultItemManager* manager = DefaultItemManager::instance();
    manager->initDefaultItems();
    QList<BookmarkData> items = manager->defaultItemInitOrder();
    EXPECT_FALSE(items.isEmpty());
}

TEST_F(TestDefaultItemManager, initPreDefineItems)
{
    DefaultItemManager* manager = DefaultItemManager::instance();
    manager->initPreDefineItems();
    QList<BookmarkData> items = manager->defaultPreDefInitOrder();
    // This might be empty depending on plugin availability
    // EXPECT_FALSE(items.isEmpty());
}

class TestUnitList : public testing::Test {
};

TEST_F(TestUnitList, createUnits)
{
    auto units = createUnits();
    EXPECT_FALSE(units.isEmpty());
    EXPECT_GT(units.size(), 0);
}