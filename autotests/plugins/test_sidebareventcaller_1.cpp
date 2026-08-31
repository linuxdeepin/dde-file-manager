// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebareventcaller_1.cpp
 * @brief Unit tests for SideBarEventCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/sidebareventcaller.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarEventCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarEventCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarEventCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarEventCallerTest, sendCheckTabAddable)
{
    // Test method: bool sendCheckTabAddable((quint64 windowId))
    auto result = obj->sendCheckTabAddable(0);
    EXPECT_FALSE(result);

}

TEST_F(SideBarEventCallerTest, sendOpenTab)
{
    // Test method: void sendOpenTab((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenTab(0, _arg1));
}

TEST_F(SideBarEventCallerTest, sendOpenWindow)
{
    // Test method: void sendOpenWindow((const QUrl &url, const bool isNew))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenWindow(_arg0, false));
}

TEST_F(SideBarEventCallerTest, sendShowFilePropertyDialog)
{
    // Test method: void sendShowFilePropertyDialog((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendShowFilePropertyDialog(_arg0));
}
