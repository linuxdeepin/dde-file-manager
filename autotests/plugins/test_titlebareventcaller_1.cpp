// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_titlebareventcaller_1.cpp
 * @brief Unit tests for TitleBarEventCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/titlebareventcaller.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class TitleBarEventCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TitleBarEventCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TitleBarEventCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TitleBarEventCallerTest, TitleBarEventCaller)
{
    // Test constructor: TitleBarEventCaller(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(TitleBarEventCallerTest, sendChangeCurrentUrl)
{
    // Test method: void sendChangeCurrentUrl((QWidget *sender, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendChangeCurrentUrl(nullptr, _arg1));
}

TEST_F(TitleBarEventCallerTest, sendGetCurrentModelBusy)
{
    // Test method: bool sendGetCurrentModelBusy((QWidget *sender))
    auto result = obj->sendGetCurrentModelBusy(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(TitleBarEventCallerTest, sendGetDefaultViewMode)
{
    // Test method: ViewMode sendGetDefaultViewMode((const QString &scheme))
    QString _arg0{};
    auto result = obj->sendGetDefaultViewMode(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(TitleBarEventCallerTest, sendOpenTab)
{
    // Test method: void sendOpenTab((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenTab(0, _arg1));
}

TEST_F(TitleBarEventCallerTest, sendOpenWindow)
{
    // Test method: void sendOpenWindow((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenWindow(_arg0));
}
