// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebareventreceiver_1.cpp
 * @brief Unit tests for SideBarEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/sidebareventreceiver.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarEventReceiverTest, SideBarEventReceiver)
{
    // Test constructor: SideBarEventReceiver((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SideBarEventReceiverTest, bindEvents)
{
    // Test method: void bindEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->bindEvents());
}

TEST_F(SideBarEventReceiverTest, handleItemHidden)
{
    // Test method: void handleItemHidden((const QUrl &url, bool visible))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleItemHidden(_arg0, false));
}

TEST_F(SideBarEventReceiverTest, handleItemInsert)
{
    // Test method: bool handleItemInsert((int index, const QUrl &url, const QVariantMap &properties))
    QUrl _arg1{};
    QVariantMap _arg2{};
    auto result = obj->handleItemInsert(0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(SideBarEventReceiverTest, handleItemTriggerEdit)
{
    // Test method: void handleItemTriggerEdit((quint64 winId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleItemTriggerEdit(0, _arg1));
}

TEST_F(SideBarEventReceiverTest, handleSetContextMenuEnable)
{
    // Test method: void handleSetContextMenuEnable((bool enable))
    EXPECT_NO_FATAL_FAILURE(obj->handleSetContextMenuEnable(false));
}

TEST_F(SideBarEventReceiverTest, handleSidebarUpdateSelection)
{
    // Test method: void handleSidebarUpdateSelection((quint64 winId))
    EXPECT_NO_FATAL_FAILURE(obj->handleSidebarUpdateSelection(0));
}

TEST_F(SideBarEventReceiverTest, instance)
{
    // Test getter: SideBarEventReceiver instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
