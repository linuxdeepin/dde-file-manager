// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_boxselector.cpp
 * @brief Unit tests for BoxSelector methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/boxselector.h"

#include <QTest>

using namespace ddplugin_canvas;

class BoxSelectorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BoxSelector();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BoxSelector *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BoxSelectorTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *watched, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(BoxSelectorTest, updateCurrentIndex)
{
    // Test method: void updateCurrentIndex(())
    EXPECT_NO_FATAL_FAILURE(obj->updateCurrentIndex());
}

TEST_F(BoxSelectorTest, validRect)
{
    // Test method: QRect validRect((CanvasView *w))
    auto result = obj->validRect(nullptr);
    EXPECT_FALSE(result.isValid());

}
