// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_gridcore_1.cpp
 * @brief Unit tests for GridCore methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "grid/gridcore.h"

#include <QTest>

using namespace ddplugin_canvas;

class GridCoreTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new GridCore();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    GridCore *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(GridCoreTest, GridCore)
{
    // Test constructor: GridCore((const GridCore &other))
    ASSERT_NE(obj, nullptr);
}

TEST_F(GridCoreTest, isFull)
{
    // Test method: bool isFull((int index))
    auto result = obj->isFull(0);
    EXPECT_FALSE(result);

}

TEST_F(GridCoreTest, isVoid)
{
    // Test bool getter: isVoid()
    bool result = obj->isVoid();
    EXPECT_FALSE(result);

}

TEST_F(GridCoreTest, item)
{
    // Test method: QString item((const GridPos &pos))
    GridPos _arg0{};
    auto result = obj->item(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(GridCoreTest, pushOverload)
{
    // Test method: void pushOverload(())
    EXPECT_NO_FATAL_FAILURE(obj->pushOverload());
}

TEST_F(GridCoreTest, surfaceSize)
{
    // Test getter: QSize surfaceSize()
    auto result = obj->surfaceSize();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(GridCoreTest, GridCore_Destructor)
{
    // Test method:  ~GridCore(())
    EXPECT_NO_FATAL_FAILURE({ GridCore *tmp = new GridCore(); delete tmp; });
}
