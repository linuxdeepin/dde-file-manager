// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_configsynchronizer.cpp
 * @brief Unit tests for ConfigSynchronizer (configsynchronizer.cpp)
 */

#include <gtest/gtest.h>

#include <dfm-base/base/configs/configsynchronizer.h>

using namespace dfmbase;

TEST(ConfigSynchronizerTest, InstanceReturnsPointer)
{
    EXPECT_NE(ConfigSynchronizer::instance(), nullptr);
}

TEST(ConfigSynchronizerTest, WatchChangeInvalidPairReturnsFalse)
{
    SyncPair pair;   // default-constructed: type == kNone -> isValid() false
    EXPECT_FALSE(ConfigSynchronizer::instance()->watchChange(pair));
}

// ---- Coverage additions: dtor + dconf change handler ----

#include "dfm-base/base/configs/private/configsynchronizer_p.h"

TEST(ConfigSynchronizerTest, OnDConfChangedForUnknownConfigIsSafe)
{
    // Unknown config path: syncToAppSet won't find a matching sync pair.
    EXPECT_NO_FATAL_FAILURE({
        ConfigSynchronizer::instance()->d->onDConfChanged("org.deepin.ut.unknown", "key");
    });
}
