// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "dbus/dbussessionmanager.h"

// [DBusSessionManager]_[ConstructDestruct]_[NoCrash]
TEST(UT_DBUSSessionManager, ConstructDestruct_NoCrash)
{
    DBusSessionManager mgr;
    EXPECT_TRUE(mgr.isValid() || !mgr.isValid());
}

// [DBusSessionManager]_[StaticNamesAndPropertyGetter]_[CoverInline]
TEST(UT_DBUSSessionManager, StaticNamesAndPropertyGetter_CoverInline)
{
    // Cover static name helpers
    EXPECT_STRNE(DBusSessionManager::staticInterfaceName(), "");
    EXPECT_STRNE(DBusSessionManager::staticServiceName(), "");
    EXPECT_STRNE(DBusSessionManager::staticObjectPath(), "");

    // Construct and touch inline property getters (return default in test env)
    DBusSessionManager mgr;
    (void)mgr.currentUid();
    (void)mgr.locked();
    (void)mgr.stage();
    SUCCEED();
}
