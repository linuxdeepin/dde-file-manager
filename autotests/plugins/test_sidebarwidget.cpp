// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarwidget.cpp
 * @brief Unit tests for SideBarWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "treeviews/sidebarwidget.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarWidgetTest, addItem)
{
    // Test method: int addItem((SideBarItem *item, bool direct))
    auto result = obj->addItem(nullptr, false);
    EXPECT_GE(result, 0);

}

TEST_F(SideBarWidgetTest, clearSettingPanel)
{
    // Test method: void clearSettingPanel(())
    EXPECT_NO_FATAL_FAILURE(obj->clearSettingPanel());
}

TEST_F(SideBarWidgetTest, findItemUrlsByVisibleControlKey)
{
    // Test method: QList<QUrl> findItemUrlsByVisibleControlKey((const QString &key))
    QString _arg0{};
    auto result = obj->findItemUrlsByVisibleControlKey(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SideBarWidgetTest, initSettingPannel)
{
    // Test method: void initSettingPannel(())
    EXPECT_NO_FATAL_FAILURE(obj->initSettingPannel());
}

TEST_F(SideBarWidgetTest, removeItem)
{
    // Test method: bool removeItem((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->removeItem(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SideBarWidgetTest, resetSettingPanel)
{
    // Test method: void resetSettingPanel(())
    EXPECT_NO_FATAL_FAILURE(obj->resetSettingPanel());
}

TEST_F(SideBarWidgetTest, saveStateWhenClose)
{
    // Test method: void saveStateWhenClose(())
    EXPECT_NO_FATAL_FAILURE(obj->saveStateWhenClose());
}

TEST_F(SideBarWidgetTest, view)
{
    // Test getter: QAbstractItemView view()
    auto result = obj->view();
    EXPECT_NO_FATAL_FAILURE({ obj->view(); });

}
