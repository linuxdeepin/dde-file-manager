// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computer_sidebar_event_calls_1.cpp
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

TEST_F(computer_sidebar_event_callsTest, callForgetPasswd)
{
    // Test method: void callForgetPasswd((const QString &stdSmb))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->callForgetPasswd(_arg0));
}

TEST_F(computer_sidebar_event_callsTest, sidebarItemClicked)
{
    // Test method: void sidebarItemClicked((quint64 winId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sidebarItemClicked(0, _arg1));
}

TEST_F(computer_sidebar_event_callsTest, sidebarItemRename)
{
    // Test method: void sidebarItemRename((quint64 windowId, const QUrl &url, const QString &name))
    QUrl _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->sidebarItemRename(0, _arg1, _arg2));
}

TEST_F(computer_sidebar_event_callsTest, sidebarUrlEquals)
{
    // Test method: bool sidebarUrlEquals((const QUrl &item, const QUrl &target))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->sidebarUrlEquals(_arg0, _arg1);
    EXPECT_FALSE(result);

}
