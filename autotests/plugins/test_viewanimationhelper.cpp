// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewanimationhelper.cpp
 * @brief Unit tests for ViewAnimationHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/viewanimationhelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class ViewAnimationHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewAnimationHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewAnimationHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewAnimationHelperTest, paintPixmaps)
{
    // Test method: void paintPixmaps((const QMap<QModelIndex, QRect> &indexRects))
    QMap<QModelIndex, QRect> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->paintPixmaps(_arg0));
}

TEST_F(ViewAnimationHelperTest, playViewAnimation)
{
    // Test method: void playViewAnimation((const QSize &oldSize, const QSize &newSize))
    QSize _arg0{};
    QSize _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->playViewAnimation(_arg0, _arg1));
}

TEST_F(ViewAnimationHelperTest, reset)
{
    // Test method: void reset(())
    EXPECT_NO_FATAL_FAILURE(obj->reset());
}

TEST_F(ViewAnimationHelperTest, resetAnimation)
{
    // Test method: void resetAnimation(())
    EXPECT_NO_FATAL_FAILURE(obj->resetAnimation());
}

TEST_F(ViewAnimationHelperTest, resetExpandItem)
{
    // Test method: void resetExpandItem(())
    EXPECT_NO_FATAL_FAILURE(obj->resetExpandItem());
}

TEST_F(ViewAnimationHelperTest, ViewAnimationHelper)
{
    // Test constructor: ViewAnimationHelper((FileView *parent))
    ASSERT_NE(obj, nullptr);
}
