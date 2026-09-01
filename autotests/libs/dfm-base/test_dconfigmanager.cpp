// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dconfigmanager.cpp
 * @brief Unit tests for DConfigManager (base/configs/dconfig/dconfigmanager.cpp)
 *
 * Assertions are kept deterministic by operating on unknown config names so the
 * behaviour is independent of whether the dde-file-manager dconfig schemas are
 * installed on the host.
 */

#include <gtest/gtest.h>
#include <QVariant>
#include <QString>
#include <QStringList>

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/configs/dconfig/global_dconf_defines.h>

using namespace dfmbase;
using namespace GlobalDConfDefines::ConfigPath;

TEST(DConfigManagerTest, InstanceReturnsSamePointer)
{
    EXPECT_EQ(DConfigManager::instance(), DConfigManager::instance());
}

TEST(DConfigManagerTest, KeysForUnknownConfigIsEmpty)
{
    EXPECT_TRUE(DConfigManager::instance()->keys("org.deepin.ut.unknown.config.zzz").isEmpty());
}

TEST(DConfigManagerTest, ContainsForUnknownConfigIsFalse)
{
    EXPECT_FALSE(DConfigManager::instance()->contains("org.deepin.ut.unknown.config.zzz", "somekey"));
}

TEST(DConfigManagerTest, ContainsWithEmptyKeyIsFalse)
{
    EXPECT_FALSE(DConfigManager::instance()->contains("org.deepin.ut.unknown.config.zzz", ""));
    EXPECT_FALSE(DConfigManager::instance()->contains(kDefaultCfgPath, ""));
}

TEST(DConfigManagerTest, ValueReturnsFallbackForUnknownConfig)
{
    const QVariant fallback("fallback-value");
    const QVariant v = DConfigManager::instance()->value("org.deepin.ut.unknown.config.zzz", "k", fallback);
    EXPECT_EQ(v.toString().toStdString(), "fallback-value");
}

TEST(DConfigManagerTest, SetValueForUnknownConfigIsSafe)
{
    EXPECT_NO_FATAL_FAILURE({
        DConfigManager::instance()->setValue("org.deepin.ut.unknown.config.zzz", "k", QVariant(1));
    });
}

TEST(DConfigManagerTest, RemoveConfigForUnknownReturnsTrue)
{
    QString err;
    EXPECT_TRUE(DConfigManager::instance()->removeConfig("org.deepin.ut.unknown.config.zzz", &err));
}

TEST(DConfigManagerTest, ValidateConfigsIsCallable)
{
    QStringList invalid;
    bool ok = DConfigManager::instance()->validateConfigs(invalid);
    // Either every registered config is valid (ok == true) or invalid ones are
    // reported in the list; both states are acceptable here.
    EXPECT_TRUE(ok || !invalid.isEmpty());
}

TEST(DConfigManagerTest, AddConfigForUnknownSchemaIsCallable)
{
    QString err;
    bool r = false;
    EXPECT_NO_FATAL_FAILURE({ r = DConfigManager::instance()->addConfig("org.deepin.ut.unknown.config.zzz", &err); });
    // An unregistered schema cannot become a valid config; either way the call
    // must not crash and must report a reason on failure.
    EXPECT_TRUE(r || !err.isEmpty());
}
