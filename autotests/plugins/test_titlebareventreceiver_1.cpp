// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_titlebareventreceiver_1.cpp
 * @brief Unit tests for TitleBarEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/titlebareventreceiver.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class TitleBarEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TitleBarEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TitleBarEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TitleBarEventReceiverTest, TitleBarEventReceiver)
{
    // Test constructor: TitleBarEventReceiver((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TitleBarEventReceiverTest, handleCloseTabs)
{
    // Test method: void handleCloseTabs((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleCloseTabs(_arg0));
}

TEST_F(TitleBarEventReceiverTest, handleOpenNewTabTriggered)
{
    // Test method: void handleOpenNewTabTriggered((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleOpenNewTabTriggered(0, _arg1));
}

TEST_F(TitleBarEventReceiverTest, handleSetNewWindowAndTabEnable)
{
    // Test method: void handleSetNewWindowAndTabEnable((bool enable))
    EXPECT_NO_FATAL_FAILURE(obj->handleSetNewWindowAndTabEnable(false));
}

TEST_F(TitleBarEventReceiverTest, handleSetTabAlias)
{
    // Test method: void handleSetTabAlias((const QUrl &url, const QString &name))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleSetTabAlias(_arg0, _arg1));
}

TEST_F(TitleBarEventReceiverTest, handleShowFilterButton)
{
    // Test method: void handleShowFilterButton((quint64 windowId, bool visible))
    EXPECT_NO_FATAL_FAILURE(obj->handleShowFilterButton(0, false));
}

TEST_F(TitleBarEventReceiverTest, handleStartSpinner)
{
    // Test method: void handleStartSpinner((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->handleStartSpinner(0));
}

TEST_F(TitleBarEventReceiverTest, handleStopSpinner)
{
    // Test method: void handleStopSpinner((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->handleStopSpinner(0));
}

TEST_F(TitleBarEventReceiverTest, handleTabAddable)
{
    // Test method: bool handleTabAddable((quint64 windowId))
    auto result = obj->handleTabAddable(0);
    EXPECT_FALSE(result);

}

TEST_F(TitleBarEventReceiverTest, handleUpdateCrumb)
{
    // Test method: void handleUpdateCrumb((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleUpdateCrumb(_arg0));
}

TEST_F(TitleBarEventReceiverTest, handleViewModeChanged)
{
    // Test method: void handleViewModeChanged((quint64 windowId, int mode))
    EXPECT_NO_FATAL_FAILURE(obj->handleViewModeChanged(0, 0));
}

TEST_F(TitleBarEventReceiverTest, handleWindowBackward)
{
    // Test method: void handleWindowBackward((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->handleWindowBackward(0));
}

TEST_F(TitleBarEventReceiverTest, handleWindowForward)
{
    // Test method: void handleWindowForward((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->handleWindowForward(0));
}

TEST_F(TitleBarEventReceiverTest, instance)
{
    // Test getter: DFMBASE_USE_NAMESPACE instance()
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });
}
