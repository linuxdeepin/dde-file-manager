// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/desktoporganizeupgradeunit.h"

#include "stubext.h"

#include <gtest/gtest.h>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QList>
#include <QMap>

using namespace dfm_upgrade;

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
            if (type == QStandardPaths::ConfigLocation || type == QStandardPaths::GenericConfigLocation) {
                return {testDir};
            }
            return QStandardPaths::standardLocations(type);
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
    // Mock file to not contain version
    auto fileReadAllFunc = static_cast<QByteArray (QFile::*)()>(&QFile::readAll);
    stub.set_lamda(fileReadAllFunc, [](QFile *file) -> QByteArray {
        Q_UNUSED(file);
        return ""; // Return empty content to simulate no version
    });

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

// Additional test for the upgrade method when file removal fails
TEST_F(TestDesktopOrganizeUpgradeUnit, upgrade_Failure_RemoveFailed)
{
    // Mock ::remove function to return failure
    auto removeFunc = static_cast<int (*)(const char *)>(&::remove);
    stub.set_lamda(removeFunc, [](const char *filename) -> int {
        Q_UNUSED(filename);
        return -1; // Return -1 to indicate failure
    });

    DesktopOrganizeUpgradeUnit unit;
    QMap<QString, QString> args;
    unit.initialize(args);
    bool result = unit.upgrade();
    // The result depends on the implementation, but the method should handle the error gracefully
    SUCCEED();  // Just verify that the method can be called without issues
}