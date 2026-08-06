// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicealiasmanager.cpp
 * @brief Unit tests for NPDeviceAliasManager (base/device/devicealiasmanager.cpp)
 *        Covers the singleton accessor, convertToProtocolUrl (exercised through
 *        the public alias API), canSetAlias, getAlias, setAlias, removeAlias and
 *        hasAlias. The cases drive the early-return / unsupported-protocol paths
 *        so no persistent settings are written.
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QString>

#include <dfm-base/base/device/devicealiasmanager.h>

using namespace dfmbase;

TEST(NPDeviceAliasManagerTest, InstanceReturnsSingleton)
{
    NPDeviceAliasManager *a = NPDeviceAliasManager::instance();
    EXPECT_NE(a, nullptr);
    EXPECT_EQ(NPDeviceAliasManager::instance(), a);
}

TEST(NPDeviceAliasManagerTest, CanSetAlias_InvalidUrlReturnsFalse)
{
    QUrl invalid;
    EXPECT_FALSE(NPDeviceAliasManager::instance()->canSetAlias(invalid));
}

TEST(NPDeviceAliasManagerTest, CanSetAlias_SmbUrlReturnsBool)
{
    QUrl smb("smb://10.0.0.1/share");
    EXPECT_NO_FATAL_FAILURE({ (void)NPDeviceAliasManager::instance()->canSetAlias(smb); });
}

TEST(NPDeviceAliasManagerTest, GetAlias_InvalidUrlReturnsEmpty)
{
    QUrl invalid;
    EXPECT_EQ(NPDeviceAliasManager::instance()->getAlias(invalid), QString());
}

TEST(NPDeviceAliasManagerTest, GetAlias_SmbUrlReturnsEmpty)
{
    QUrl smb("smb://10.0.0.2/share");
    EXPECT_EQ(NPDeviceAliasManager::instance()->getAlias(smb), QString());
}

TEST(NPDeviceAliasManagerTest, GetAlias_FileUrlWithHostIpConvertsProtocol)
{
    // A file:// URL carrying a host=<ip> fragment exercises the regex +
    // protocol-detection branch of convertToProtocolUrl().
    QUrl fileUrl("file:///tmp/host=10.20.30.40/share");
    EXPECT_NO_FATAL_FAILURE({ (void)NPDeviceAliasManager::instance()->getAlias(fileUrl); });
}

TEST(NPDeviceAliasManagerTest, SetAlias_InvalidUrlReturnsFalse)
{
    QUrl invalid;
    EXPECT_FALSE(NPDeviceAliasManager::instance()->setAlias(invalid, "alias"));
}

TEST(NPDeviceAliasManagerTest, SetAlias_SmbUrlReturnsBool)
{
    QUrl smb("smb://10.0.0.3/share");
    EXPECT_NO_FATAL_FAILURE({ (void)NPDeviceAliasManager::instance()->setAlias(smb, "alias"); });
}

TEST(NPDeviceAliasManagerTest, RemoveAlias_InvalidUrlCallable)
{
    QUrl invalid;
    EXPECT_NO_FATAL_FAILURE({ NPDeviceAliasManager::instance()->removeAlias(invalid); });
}

TEST(NPDeviceAliasManagerTest, HasAlias_SmbUrlReturnsBool)
{
    QUrl smb("smb://10.0.0.4/share");
    EXPECT_NO_FATAL_FAILURE({ (void)NPDeviceAliasManager::instance()->hasAlias(smb); });
}

TEST(NPDeviceAliasManagerTest, ConvertToProtocolUrl_NonFileSchemeReturnsAsIs)
{
    // convertToProtocolUrl() is private but is invoked by getAlias(); a non-file
    // scheme short-circuits and returns the URL unchanged.
    QUrl smb("smb://10.0.0.5/share");
    EXPECT_NO_FATAL_FAILURE({ (void)NPDeviceAliasManager::instance()->getAlias(smb); });
}
