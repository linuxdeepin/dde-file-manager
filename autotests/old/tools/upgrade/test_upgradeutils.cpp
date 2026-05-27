// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
#include <QList>
#include <QVariant>

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
            qDebug() << "Test upgrade mock standard location" << type << testDir;
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