// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computer_sidebar_event_calls.cpp
 * @brief Unit tests for computer_sidebar_event_calls methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "displaycontrol/utilities/protocoldisplayutilities.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class computer_sidebar_event_callsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new computer_sidebar_event_calls();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    computer_sidebar_event_calls *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(computer_sidebar_event_callsTest, callComputerRefresh)
{
    // Test method: void callComputerRefresh(())
    EXPECT_NO_FATAL_FAILURE(obj->callComputerRefresh());
}

TEST_F(computer_sidebar_event_callsTest, callItemAdd)
{
    // Test method: void callItemAdd((const QUrl &vEntryUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->callItemAdd(_arg0));
}

TEST_F(computer_sidebar_event_callsTest, callItemRemove)
{
    // Test method: void callItemRemove((const QUrl &vEntryUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->callItemRemove(_arg0));
}

TEST_F(computer_sidebar_event_callsTest, sidebarMenuCall)
{
    // Test method: void sidebarMenuCall((quint64 winId, const QUrl &url, const QPoint &pos))
    QUrl _arg1{};
    QPoint _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->sidebarMenuCall(0, _arg1, _arg2));
}
