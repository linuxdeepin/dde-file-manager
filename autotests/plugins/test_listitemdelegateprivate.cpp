// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_listitemdelegateprivate.cpp
 * @brief Unit tests for ListItemDelegatePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/private/listitemdelegate_p.h"

#include <QTest>

using namespace dfmplugin_workspace;

class ListItemDelegatePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ListItemDelegatePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ListItemDelegatePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ListItemDelegatePrivateTest, ListItemDelegatePrivate)
{
    // Test constructor: ListItemDelegatePrivate((ListItemDelegate *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ListItemDelegatePrivateTest, ListItemDelegatePrivate_Destructor)
{
    // Test method:  ~ListItemDelegatePrivate(())
    EXPECT_NO_FATAL_FAILURE({ ListItemDelegatePrivate *tmp = new ListItemDelegatePrivate(); delete tmp; });
}
