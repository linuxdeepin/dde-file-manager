// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbbrowsereventcaller_1.cpp
 * @brief Unit tests for SmbBrowserEventCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/smbbrowsereventcaller.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class SmbBrowserEventCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SmbBrowserEventCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SmbBrowserEventCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SmbBrowserEventCallerTest, sendCheckTabAddable)
{
    // Test method: bool sendCheckTabAddable((quint64 windowId))
    auto result = obj->sendCheckTabAddable(0);
    EXPECT_FALSE(result);

}

TEST_F(SmbBrowserEventCallerTest, sendOpenTab)
{
    // Test method: void sendOpenTab((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenTab(0, _arg1));
}

TEST_F(SmbBrowserEventCallerTest, sendShowPropertyDialog)
{
    // Test method: void sendShowPropertyDialog((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendShowPropertyDialog(_arg0));
}
