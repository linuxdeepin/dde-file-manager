// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/dconfigupgradeunit.h"
#include "utils/upgradeutils.h"

#include "stubext.h"

#include <gtest/gtest.h>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QList>
#include <QMap>
#include <QVariant>

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

DFMBASE_USE_NAMESPACE
using namespace dfm_upgrade;

class TestDConfigUpgradeUnit : public testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for test
        testDir = QDir::tempPath() + "/dconfig_test";
        QDir().mkpath(testDir);

        // Mock QStandardPaths::standardLocations to use test directory
        auto standlocal = static_cast<QList<QString> (*)(QStandardPaths::StandardLocation)>(&QStandardPaths::standardLocations);
        stub.set_lamda(standlocal, [&](QStandardPaths::StandardLocation type) -> QList<QString> {
            if (type == QStandardPaths::ConfigLocation) {
                return {testDir};
            }
            return QStandardPaths::standardLocations(type);
        });

        // Mock UpgradeUtils operations since DConfigUpgradeUnit uses them
        auto genericAttributeFunc = static_cast<QVariant (*)(const QString &)>(&UpgradeUtils::genericAttribute);
        stub.set_lamda(genericAttributeFunc, [&](const QString &key) -> QVariant {
            Q_UNUSED(key);
            // Return appropriate test values based on the key
            if (key.contains("AlwaysShowOfflineRemoteConnections")) {
                return true;
            } else if (key.contains("ShowRecentFileEntry")) {
                return true;
            } else if (key.contains("IndexFullTextSearch")) {
                return false;
            }
            return QVariant();
        });

        auto addOldGenericAttributeFunc = static_cast<void (*)(const QJsonArray &)>(&UpgradeUtils::addOldGenericAttribute);
        stub.set_lamda(addOldGenericAttributeFunc, [&](const QJsonArray &attrs) {
            Q_UNUSED(attrs);
            // Mock adding old generic attributes without actually changing system config
        });
    }

    void TearDown() override {
        QDir(testDir).removeRecursively();
        stub.clear();
    }

    QString testDir;
    stub_ext::StubExt stub;
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

// Test specific upgrade functionality
TEST_F(TestDConfigUpgradeUnit, upgradeMenuConfigs)
{
    // This test specifically targets the menu config upgrade functionality
    DConfigUpgradeUnit unit;
    bool result = unit.upgradeMenuConfigs();
    EXPECT_TRUE(result);
}

// Test SMB config upgrade
TEST_F(TestDConfigUpgradeUnit, upgradeSmbConfigs)
{
    // This test specifically targets the SMB config upgrade functionality
    DConfigUpgradeUnit unit;
    bool result = unit.upgradeSmbConfigs();
    EXPECT_TRUE(result);
}

// Test recent config upgrade
TEST_F(TestDConfigUpgradeUnit, upgradeRecentConfigs)
{
    // Mock DConfigManager::addConfig
    bool addResult = true;
    QString errMsg;

    stub.set_lamda(ADDR(DConfigManager, addConfig), [&](DConfigManager *obj, const QString &config, QString *err) {
        if (err) *err = errMsg;
        return addResult;
    });

    // This test specifically targets the recent config upgrade functionality
    DConfigUpgradeUnit unit;
    bool result = unit.upgradeRecentConfigs();
    EXPECT_TRUE(result);
}

// Test search config upgrade
TEST_F(TestDConfigUpgradeUnit, upgradeSearchConfigs)
{
    // Mock DConfigManager::addConfig
    bool addResult = true;
    QString errMsg;

    stub.set_lamda(ADDR(DConfigManager, addConfig), [&](DConfigManager *obj, const QString &config, QString *err) {
        if (err) *err = errMsg;
        return addResult;
    });

    // This test specifically targets the search config upgrade functionality
    DConfigUpgradeUnit unit;
    bool result = unit.upgradeSearchConfigs();
    EXPECT_TRUE(result);
}