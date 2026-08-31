// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sheetbrowser.cpp
 * @brief Unit tests for SheetBrowser methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/pdf-preview/sheetbrowser.h"

#include <QTest>

using namespace src;

class SheetBrowserTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SheetBrowser();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SheetBrowser *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SheetBrowserTest, init)
{
    // Test method: void init((SheetOperation &operation))
    SheetOperation _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->init(_arg0));
}

TEST_F(SheetBrowserTest, maxHeight)
{
    // Test getter: qreal maxHeight()
    auto result = obj->maxHeight();
    EXPECT_EQ(result, 0.0);

}

TEST_F(SheetBrowserTest, maxWidth)
{
    // Test getter: qreal maxWidth()
    auto result = obj->maxWidth();
    EXPECT_EQ(result, 0.0);

}

TEST_F(SheetBrowserTest, onRemoveDocSlideGesture)
{
    // Test method: void onRemoveDocSlideGesture(())
    EXPECT_NO_FATAL_FAILURE(obj->onRemoveDocSlideGesture());
}

TEST_F(SheetBrowserTest, setCurrentPage)
{
    // Test setter: void setCurrentPage((int page))
    EXPECT_NO_FATAL_FAILURE(obj->setCurrentPage(0));
}
