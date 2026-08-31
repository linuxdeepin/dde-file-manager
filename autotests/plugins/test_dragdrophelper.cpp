// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dragdrophelper.cpp
 * @brief Unit tests for DragDropHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/dragdrophelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class DragDropHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DragDropHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DragDropHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DragDropHelperTest, processDragAutoScroll)
{
    // Test bool getter: processDragAutoScroll()
    bool result = obj->processDragAutoScroll();
    EXPECT_FALSE(result);

}

TEST_F(DragDropHelperTest, resetDragState)
{
    // Test method: void resetDragState(())
    EXPECT_NO_FATAL_FAILURE(obj->resetDragState());
}
