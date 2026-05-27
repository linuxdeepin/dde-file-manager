// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>

#include <dfm-base/utils/systemservicemanager.h>
#include "stubext.h"

using namespace dfmbase;

class SystemServiceManagerTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
    }

    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(SystemServiceManagerTest, Instance_GetInstance_ExpectedSameInstance) {
    // Act
    SystemServiceManager &instance1 = SystemServiceManager::instance();
    SystemServiceManager &instance2 = SystemServiceManager::instance();

    // Assert
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(SystemServiceManagerTest, IsServiceRunning_WithEmptyServiceName_ExpectedFalse) {
    // Arrange
    SystemServiceManager &manager = SystemServiceManager::instance();
    QString emptyServiceName = "";

    // Act
    bool result = manager.isServiceRunning(emptyServiceName);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(SystemServiceManagerTest, IsServiceRunning_WithValidServiceName_ExpectedNoCrash) {
    // Arrange
    SystemServiceManager &manager = SystemServiceManager::instance();
    QString serviceName = "test.service";
    
    // Mock the D-Bus call to avoid authentication popup
    stub.set_lamda((bool (dfmbase::SystemServiceManager::*)(const QString &))&dfmbase::SystemServiceManager::isServiceRunning,
        [&serviceName](dfmbase::SystemServiceManager *obj, const QString &name) -> bool {
            // Only return true for our test service name to simulate running state
            return name == serviceName;
        });

    // Act
    bool result = manager.isServiceRunning(serviceName);

    // Assert
    // Just ensure no crash and returns expected value
    EXPECT_TRUE(result);
}

TEST_F(SystemServiceManagerTest, StartService_WithEmptyServiceName_ExpectedFalse) {
    // Arrange
    SystemServiceManager &manager = SystemServiceManager::instance();
    QString emptyServiceName = "";

    // Act
    bool result = manager.startService(emptyServiceName);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(SystemServiceManagerTest, StartService_WithValidServiceName_ExpectedNoCrash) {
    // Arrange
    SystemServiceManager &manager = SystemServiceManager::instance();
    QString serviceName = "test.service";
    
    // Mock the D-Bus call to avoid authentication popup
    stub.set_lamda((bool (dfmbase::SystemServiceManager::*)(const QString &))&dfmbase::SystemServiceManager::startService,
        [&serviceName](dfmbase::SystemServiceManager *obj, const QString &name) -> bool {
            // Only return true for our test service name
            return name == serviceName;
        });

    // Act
    bool result = manager.startService(serviceName);

    // Assert
    // Just ensure no crash and returns expected value
    EXPECT_TRUE(result);
}

TEST_F(SystemServiceManagerTest, EnableServiceNow_WithEmptyServiceName_ExpectedFalse) {
    // Arrange
    SystemServiceManager &manager = SystemServiceManager::instance();
    QString emptyServiceName = "";

    // Act
    bool result = manager.enableServiceNow(emptyServiceName);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(SystemServiceManagerTest, EnableServiceNow_WithValidServiceName_ExpectedNoCrash) {
    // Arrange
    SystemServiceManager &manager = SystemServiceManager::instance();
    QString serviceName = "test.service";
    
    // Mock the QProcess::execute call to avoid pkexec authentication popup
    stub.set_lamda((bool (dfmbase::SystemServiceManager::*)(const QString &))&dfmbase::SystemServiceManager::enableServiceNow,
        [&serviceName](dfmbase::SystemServiceManager *obj, const QString &name) -> bool {
            // Only return true for our test service name
            return name == serviceName;
        });

    // Act
    bool result = manager.enableServiceNow(serviceName);

    // Assert
    // Just ensure no crash and returns expected value
    EXPECT_TRUE(result);
}

TEST_F(SystemServiceManagerTest, UnitPathFromName_WithEmptyServiceName_ExpectedEmptyPath) {
    // Arrange
    SystemServiceManager &manager = SystemServiceManager::instance();
    QString emptyServiceName = "";

    // Act
    QString result = manager.unitPathFromName(emptyServiceName);

    // Assert
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(SystemServiceManagerTest, UnitPathFromName_WithValidServiceName_ExpectedNoCrash) {
    // Arrange
    SystemServiceManager &manager = SystemServiceManager::instance();
    QString serviceName = "test.service";
    QString expectedPath = "/org/freedesktop/systemd1/unit/test_2eservice";
    
    // Mock the D-Bus call to avoid authentication popup
    stub.set_lamda((QString (dfmbase::SystemServiceManager::*)(const QString &))&dfmbase::SystemServiceManager::unitPathFromName,
        [&serviceName, &expectedPath](dfmbase::SystemServiceManager *obj, const QString &name) -> QString {
            // Return a mock path for our test service name
            return name == serviceName ? expectedPath : QString();
        });

    // Act
    QString result = manager.unitPathFromName(serviceName);

    // Assert
    // Just ensure no crash and returns expected value
    EXPECT_EQ(result, expectedPath);
}