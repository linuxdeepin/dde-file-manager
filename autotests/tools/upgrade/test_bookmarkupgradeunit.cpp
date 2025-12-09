// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/bookmarkupgradeunit.h"
#include "utils/upgradeutils.h"

#include "stubext.h"

#include <gtest/gtest.h>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QList>
#include <QMap>
#include <QFileInfo>
#include <QDebug>

using namespace dfm_upgrade;

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

        // Add stubs to mock file operations and prevent actual modification of user bookmarks
        oldDirPath = testDir + "/deepin/dde-file-manager/old";
        auto backupFileFunc = static_cast<bool (*)(const QString &, const QString &)>(&UpgradeUtils::backupFile);
        stub.set_lamda(backupFileFunc, [&](const QString &source, const QString &backupDir) -> bool {
            if (source == configFilePath && backupDir == oldDirPath) {
                return true;
            }
            qDebug() << "Mocking backupFile:" << source << "to" << backupDir;
            QDir().mkpath(oldDirPath);
            QFile file(configFilePath);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray data = file.readAll();
                file.close();
                QString backupFileName = oldDirPath + "/backup_" + QFileInfo(configFilePath).fileName();
                QFile backupFile(backupFileName);
                if (backupFile.open(QIODevice::WriteOnly)) {
                    backupFile.write(data);
                    backupFile.close();
                    return true;
                }
            }
            return true;  // Return true to indicate success in test environment
        });
    }

    void TearDown() override {
        QDir(testDir).removeRecursively();
        stub.clear();
    }

    QString testDir;
    QString configFilePath;
    QString oldDirPath;
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

// Test the completed method for BookMarkUpgradeUnit
TEST_F(TestBookMarkUpgradeUnit, completed)
{
    BookMarkUpgradeUnit unit;
    unit.completed(); // Should not crash or throw
    SUCCEED();  // Just verify that the method can be called without issues
}

// Additional test for the initData method functionality
TEST_F(TestBookMarkUpgradeUnit, initData)
{
    BookMarkUpgradeUnit unit;
    QMap<QString, QString> args;
    unit.initialize(args);

    // Call the protected method indirectly by calling upgrade which uses initData
    bool result = unit.upgrade();
    EXPECT_TRUE(result);
}

// Additional test for the doUpgrade method functionality
TEST_F(TestBookMarkUpgradeUnit, doUpgrade)
{
    BookMarkUpgradeUnit unit;
    QMap<QString, QString> args;
    unit.initialize(args);

    bool result = unit.upgrade();
    EXPECT_TRUE(result);
}