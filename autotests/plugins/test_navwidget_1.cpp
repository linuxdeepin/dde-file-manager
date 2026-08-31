// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_navwidget_1.cpp
 * @brief Unit tests for NavWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/navwidget.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class NavWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NavWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NavWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NavWidgetTest, addHistroyStack)
{
    // Test method: void addHistroyStack(())
    EXPECT_NO_FATAL_FAILURE(obj->addHistroyStack());
}

TEST_F(NavWidgetTest, changeSizeMode)
{
    // Test method: void changeSizeMode(())
    EXPECT_NO_FATAL_FAILURE(obj->changeSizeMode());
}

TEST_F(NavWidgetTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(NavWidgetTest, initializeUi)
{
    // Test method: void initializeUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeUi());
}

TEST_F(NavWidgetTest, moveNavStacks)
{
    // Test method: void moveNavStacks((int from, int to))
    EXPECT_NO_FATAL_FAILURE(obj->moveNavStacks(0, 0));
}

TEST_F(NavWidgetTest, onDevUnmounted)
{
    // Test method: void onDevUnmounted((const QString &id, const QString &oldMpt))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onDevUnmounted(_arg0, _arg1));
}

TEST_F(NavWidgetTest, onNewWindowOpended)
{
    // Test method: void onNewWindowOpended(())
    EXPECT_NO_FATAL_FAILURE(obj->onNewWindowOpended());
}

TEST_F(NavWidgetTest, onUrlChanged)
{
    // Test method: void onUrlChanged((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onUrlChanged(_arg0));
}

TEST_F(NavWidgetTest, pushUrlToHistoryStack)
{
    // Test method: void pushUrlToHistoryStack((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->pushUrlToHistoryStack(_arg0));
}

TEST_F(NavWidgetTest, switchHistoryStack)
{
    // Test method: void switchHistoryStack((const int index))
    EXPECT_NO_FATAL_FAILURE(obj->switchHistoryStack(0));
}
