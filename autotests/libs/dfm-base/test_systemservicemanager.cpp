// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_systemservicemanager.cpp
 * @brief Unit tests for SystemServiceManager (utils/systemservicemanager.cpp).
 *        All calls use an empty service name so every method hits its early
 *        return BEFORE constructing any QDBusInterface — no system bus traffic.
 *        unitPathFromName is private but reachable (the test build uses
 *        -fno-access-control).
 */

#include <gtest/gtest.h>
#include <QString>

#include <dfm-base/utils/systemservicemanager.h>

using namespace dfmbase;

TEST(SystemServiceManagerTest, InstanceReturnsSingleton)
{
    SystemServiceManager &a = SystemServiceManager::instance();
    SystemServiceManager &b = SystemServiceManager::instance();
    EXPECT_EQ(&a, &b);
}

TEST(SystemServiceManagerTest, IsServiceRunningEmptyNameReturnsFalse)
{
    EXPECT_FALSE(SystemServiceManager::instance().isServiceRunning(QString()));
}

TEST(SystemServiceManagerTest, ServiceExistsEmptyNameReturnsFalse)
{
    EXPECT_FALSE(SystemServiceManager::instance().serviceExists(QString()));
}

TEST(SystemServiceManagerTest, StartServiceEmptyNameReturnsFalse)
{
    EXPECT_FALSE(SystemServiceManager::instance().startService(QString()));
}

TEST(SystemServiceManagerTest, EnableServiceNowEmptyNameReturnsFalse)
{
    EXPECT_FALSE(SystemServiceManager::instance().enableServiceNow(QString()));
}

TEST(SystemServiceManagerTest, UnitPathFromNameEmptyReturnsEmpty)
{
    EXPECT_EQ(SystemServiceManager::instance().unitPathFromName(QString()), QString());
}
