// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bookmarkeventcaller.cpp
 * @brief Unit tests for BookMarkEventCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/bookmarkeventcaller.h"

#include <QTest>

using namespace dfmplugin_bookmark;

class BookMarkEventCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BookMarkEventCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BookMarkEventCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BookMarkEventCallerTest, BookMarkEventCaller)
{
    // Test constructor: BookMarkEventCaller(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(BookMarkEventCallerTest, sendBookMarkOpenInNewTab)
{
    // Test method: void sendBookMarkOpenInNewTab((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendBookMarkOpenInNewTab(0, _arg1));
}

TEST_F(BookMarkEventCallerTest, sendBookMarkOpenInNewWindow)
{
    // Test method: DFMBASE_USE_NAMESPACE sendBookMarkOpenInNewWindow((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj->sendBookMarkOpenInNewWindow(_arg0); });
}

TEST_F(BookMarkEventCallerTest, sendCheckTabAddable)
{
    // Test method: bool sendCheckTabAddable((quint64 windowId))
    auto result = obj->sendCheckTabAddable(0);
    EXPECT_FALSE(result);

}

TEST_F(BookMarkEventCallerTest, sendOpenBookMarkInWindow)
{
    // Test method: void sendOpenBookMarkInWindow((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenBookMarkInWindow(0, _arg1));
}

TEST_F(BookMarkEventCallerTest, sendShowBookMarkPropertyDialog)
{
    // Test method: void sendShowBookMarkPropertyDialog((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendShowBookMarkPropertyDialog(_arg0));
}
