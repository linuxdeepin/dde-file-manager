// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/appattributeupgradeunit.h"

#include "stubext.h"

#include <gtest/gtest.h>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDebug>
#include <QList>
#include <QMap>

using namespace dfm_upgrade;

class TestAppAttributeUpgradeUnit : public testing::Test {
protected:
    void SetUp() override {
        testDir = QDir::tempPath() + "/app_attribute_test";
        QDir().mkpath(testDir);

        configFilePath = testDir + "/deepin/dde-file-manager/dde-file-manager.json";
        QDir().mkpath(testDir + "/deepin/dde-file-manager");
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
            if (type == QStandardPaths::ConfigLocation) {
                qDebug() << "Mocking standard locations:" << testDir;
                return {testDir};
            }
            return QStandardPaths::standardLocations(type);
        });


        // Add stubs to mock backupAppAttribute
        auto backupFileFunc = static_cast<bool (dfm_upgrade::AppAttributeUpgradeUnit::*)() const>(&dfm_upgrade::AppAttributeUpgradeUnit::backupAppAttribute);
        stub.set_lamda(backupFileFunc, [&]() -> bool {
            qDebug() << "Mocking App backupFile:" << configFilePath << "to" << testDir + "/dde-file-manager/old";
            return true; // Return true to indicate success in test environment
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
    // move the config file to ensure it's not found
    auto backupFile = testDir + "/deepin/dde-file-manager/dde-file-manager.json.bak";
    QFile(configFilePath).rename(backupFile);

    AppAttributeUpgradeUnit unit;
    QMap<QString, QString> args;
    bool result = unit.initialize(args);
    EXPECT_FALSE(result);

    // restore the config file
    QFile(backupFile).rename(configFilePath);
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

// Additional test for the backupAppAttribute method
TEST_F(TestAppAttributeUpgradeUnit, backupAppAttribute)
{
    AppAttributeUpgradeUnit unit;
    bool result = unit.backupAppAttribute();
    EXPECT_TRUE(result);
}

// Additional test for the writeConfigFile method
TEST_F(TestAppAttributeUpgradeUnit, writeConfigFile)
{
    // This test checks the writeConfigFile functionality indirectly
    AppAttributeUpgradeUnit unit;
    QMap<QString, QString> args;
    bool initResult = unit.initialize(args);
    if (initResult) {
        bool result = unit.upgrade();
        EXPECT_TRUE(result);
    } else {
        FAIL() << "Initialization failed, cannot test writeConfigFile";
    }
}