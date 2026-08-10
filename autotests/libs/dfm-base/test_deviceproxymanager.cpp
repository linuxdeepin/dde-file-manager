// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_deviceproxymanager.cpp
 * @brief Unit tests for DeviceProxyManager (deviceproxymanager.cpp)
 *
 * DeviceProxyManager is a singleton proxy that chooses between DBus and
 * direct API for device operations. In the unit-test environment the DBus
 * service is not running, so isDBusRunning() returns false and the
 * direct-API fallback paths are used. No real device hardware is needed.
 */

#include <gtest/gtest.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/devicemanager.h>

#include <QString>
#include <QStringList>
#include <QVariantMap>

using namespace dfmbase;

TEST(DeviceProxyManagerTest, InstanceReturnsNonNullSingleton)
{
    auto *a = DeviceProxyManager::instance();
    auto *b = DeviceProxyManager::instance();
    ASSERT_NE(a, nullptr);
    EXPECT_EQ(a, b);
}

TEST(DeviceProxyManagerTest, IsDBusRunningReturnsFalseWithoutService)
{
    auto *m = DeviceProxyManager::instance();
    EXPECT_FALSE(m->isDBusRuning());
}

TEST(DeviceProxyManagerTest, GetAllBlockIdsDoesNotCrash)
{
    auto *m = DeviceProxyManager::instance();
    QStringList ids = m->getAllBlockIds();
    // Without hardware/DBus, this returns via DevMngIns which is also empty.
    (void)ids;
    SUCCEED();
}

TEST(DeviceProxyManagerTest, GetAllProtocolIdsesDoesNotCrash)
{
    auto *m = DeviceProxyManager::instance();
    QStringList ids = m->getAllProtocolIds();
    (void)ids;
    SUCCEED();
}

TEST(DeviceProxyManagerTest, QueryBlockInfoReturnsEmptyForNonExistentDevice)
{
    auto *m = DeviceProxyManager::instance();
    QVariantMap info = m->queryBlockInfo(QStringLiteral("/dev/nonexistent-ut-12345"));
    EXPECT_TRUE(info.isEmpty());
}

TEST(DeviceProxyManagerTest, QueryProtocolInfoReturnsEmptyForNonExistentDevice)
{
    auto *m = DeviceProxyManager::instance();
    QVariantMap info = m->queryProtocolInfo(QStringLiteral("nonexistent-ut-67890"));
    EXPECT_TRUE(info.isEmpty());
}
