// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_browserpage.cpp
 * @brief Unit tests for BrowserPage methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/pdf-preview/browserpage.h"

#include <QTest>

using namespace src;

class BrowserPageTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BrowserPage();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BrowserPage *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BrowserPageTest, clearPixmap)
{
    // Test method: void clearPixmap(())
    EXPECT_NO_FATAL_FAILURE(obj->clearPixmap());
}

TEST_F(BrowserPageTest, getTopLeftPos)
{
    // Test getter: QPointF getTopLeftPos()
    auto result = obj->getTopLeftPos();
    EXPECT_TRUE(result.isNull());

}

TEST_F(BrowserPageTest, itemIndex)
{
    // Test getter: int itemIndex()
    auto result = obj->itemIndex();
    EXPECT_EQ(result, 0);

}

TEST_F(BrowserPageTest, rect)
{
    // Test getter: QRectF rect()
    auto result = obj->rect();
    EXPECT_FALSE(result.isValid());

}
