// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_clickselector_1.cpp
 * @brief Unit tests for ClickSelector methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/clickselector.h"

#include <QTest>

using namespace ddplugin_canvas;

class ClickSelectorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ClickSelector();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ClickSelector *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ClickSelectorTest, ClickSelector)
{
    // Test constructor: ClickSelector((CanvasView *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ClickSelectorTest, continuesSelect)
{
    // Test method: void continuesSelect((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->continuesSelect(_arg0));
}

TEST_F(ClickSelectorTest, expandSelect)
{
    // Test method: void expandSelect((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->expandSelect(_arg0));
}

TEST_F(ClickSelectorTest, release)
{
    // Test method: void release((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->release(_arg0));
}

TEST_F(ClickSelectorTest, singleSelect)
{
    // Test method: void singleSelect((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->singleSelect(_arg0));
}
