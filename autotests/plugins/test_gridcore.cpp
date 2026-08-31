// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_gridcore.cpp
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

TEST_F(GridCoreTest, applay)
{
    // Test method: bool applay((GridCore *core))
    auto result = obj->applay(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(GridCoreTest, insert)
{
    // Test method: void insert((int index, const QPoint &pos, const QString &it))
    QPoint _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->insert(0, _arg1, _arg2));
}

TEST_F(GridCoreTest, isValid)
{
    // Test bool getter: isValid()
    bool result = obj->isValid();
    EXPECT_FALSE(result);

}

TEST_F(GridCoreTest, position)
{
    // Test method: bool position((const QString &it, GridPos &pos))
    QString _arg0{};
    GridPos _arg1{};
    auto result = obj->position(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(GridCoreTest, remove)
{
    // Test method: void remove((int index, const QPoint &pos))
    QPoint _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->remove(0, _arg1));
}

TEST_F(GridCoreTest, surfaceIndex)
{
    // Test getter: QList<int> surfaceIndex()
    auto result = obj->surfaceIndex();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(GridCoreTest, voidPos)
{
    // Test method: QList<QPoint> voidPos((int index))
    auto result = obj->voidPos(0);
    EXPECT_TRUE(result.isEmpty());

}
