// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dragdropoper.cpp
 * @brief Unit tests for DragDropOper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/dragdropoper.h"

#include <QTest>

using namespace ddplugin_canvas;

class DragDropOperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DragDropOper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DragDropOper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DragDropOperTest, checkXdndDirectSave)
{
    // Test method: bool checkXdndDirectSave((QDragEnterEvent *event))
    auto result = obj->checkXdndDirectSave(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DragDropOperTest, dropDirectSaveMode)
{
    // Test method: bool dropDirectSaveMode((QDropEvent *event))
    auto result = obj->dropDirectSaveMode(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DragDropOperTest, dropMimeData)
{
    // Test method: bool dropMimeData((QDropEvent *event))
    auto result = obj->dropMimeData(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DragDropOperTest, enter)
{
    // Test method: bool enter((QDragEnterEvent *event))
    auto result = obj->enter(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DragDropOperTest, hoverIndex)
{
    // Test getter: QModelIndex hoverIndex()
    auto result = obj->hoverIndex();
    EXPECT_FALSE(result.isValid());

}
