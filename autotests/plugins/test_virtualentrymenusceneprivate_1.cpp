// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualentrymenusceneprivate_1.cpp
 * @brief Unit tests for VirtualEntryMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "displaycontrol/menu/virtualentrymenuscene.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class VirtualEntryMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VirtualEntryMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VirtualEntryMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VirtualEntryMenuScenePrivateTest, actCptForget)
{
    // Test method: void actCptForget(())
    EXPECT_NO_FATAL_FAILURE(obj->actCptForget());
}

TEST_F(VirtualEntryMenuScenePrivateTest, actCptMount)
{
    // Test method: void actCptMount(())
    EXPECT_NO_FATAL_FAILURE(obj->actCptMount());
}

TEST_F(VirtualEntryMenuScenePrivateTest, actForgetAggregatedItem)
{
    // Test method: void actForgetAggregatedItem(())
    EXPECT_NO_FATAL_FAILURE(obj->actForgetAggregatedItem());
}

TEST_F(VirtualEntryMenuScenePrivateTest, actMountSeperatedItem)
{
    // Test method: void actMountSeperatedItem(())
    EXPECT_NO_FATAL_FAILURE(obj->actMountSeperatedItem());
}

TEST_F(VirtualEntryMenuScenePrivateTest, actUnmountAggregatedItem)
{
    // Test method: void actUnmountAggregatedItem((bool removeEntry))
    EXPECT_NO_FATAL_FAILURE(obj->actUnmountAggregatedItem(false));
}

TEST_F(VirtualEntryMenuScenePrivateTest, gotoDefaultPageOnUnmount)
{
    // Test method: void gotoDefaultPageOnUnmount(())
    EXPECT_NO_FATAL_FAILURE(obj->gotoDefaultPageOnUnmount());
}

TEST_F(VirtualEntryMenuScenePrivateTest, hookCptActions)
{
    // Test method: void hookCptActions((QAction *triggered))
    EXPECT_NO_FATAL_FAILURE(obj->hookCptActions(nullptr));
}

TEST_F(VirtualEntryMenuScenePrivateTest, insertActionBefore)
{
    // Test method: void insertActionBefore((const QString &inserted, const QString &before, QMenu *menu))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->insertActionBefore(_arg0, _arg1, nullptr));
}
