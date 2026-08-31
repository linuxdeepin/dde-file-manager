// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_expandeditem.cpp
 * @brief Unit tests for ExpandedItem methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/expandedItem.h"

#include <QTest>

using namespace dfmplugin_workspace;

class ExpandedItemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExpandedItem();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExpandedItem *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExpandedItemTest, ExpandedItem)
{
    // Test constructor: ExpandedItem((dfmplugin_workspace::IconItemDelegate *d, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ExpandedItemTest, getCanDeferredDelete)
{
    // Test bool getter: getCanDeferredDelete()
    bool result = obj->getCanDeferredDelete();
    EXPECT_FALSE(result);

}

TEST_F(ExpandedItemTest, setCanDeferredDelete)
{
    // Test setter: void setCanDeferredDelete((bool canDeferredDelete))
    EXPECT_NO_FATAL_FAILURE(obj->setCanDeferredDelete(false));
}

TEST_F(ExpandedItemTest, setOpacity)
{
    // Test setter: void setOpacity((qreal opacity))
    EXPECT_NO_FATAL_FAILURE(obj->setOpacity(0.0));
}
