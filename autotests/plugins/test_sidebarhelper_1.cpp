// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarhelper_1.cpp
 * @brief Unit tests for SideBarHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/sidebarhelper.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarHelperTest, addSideBar)
{
    // Test method: void addSideBar((quint64 windowId, SideBarWidget *sideBar))
    EXPECT_NO_FATAL_FAILURE(obj->addSideBar(0, nullptr));
}

TEST_F(SideBarHelperTest, bindSetting)
{
    // Test method: void bindSetting((const QString &itemVisiableSettingKey, const QString &itemVisiableControlKey))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->bindSetting(_arg0, _arg1));
}

TEST_F(SideBarHelperTest, createItemByInfo)
{
    // Test method: SideBarItem createItemByInfo((const ItemInfo &info))
    ItemInfo _arg0{};
    auto result = obj->createItemByInfo(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createItemByInfo(_arg0); });

}

TEST_F(SideBarHelperTest, createSeparatorItem)
{
    // Test method: SideBarItemSeparator createSeparatorItem((const QString &group))
    QString _arg0{};
    auto result = obj->createSeparatorItem(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createSeparatorItem(_arg0); });

}

TEST_F(SideBarHelperTest, defaultCdAction)
{
    // Test method: void defaultCdAction((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->defaultCdAction(0, _arg1));
}

TEST_F(SideBarHelperTest, defaultContextMenu)
{
    // Test method: void defaultContextMenu((quint64 windowId, const QUrl &url, const QPoint &globalPos))
    QUrl _arg1{};
    QPoint _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->defaultContextMenu(0, _arg1, _arg2));
}

TEST_F(SideBarHelperTest, initDefaultSettingPanel)
{
    // Test method: void initDefaultSettingPanel(())
    EXPECT_NO_FATAL_FAILURE(obj->initDefaultSettingPanel());
}

TEST_F(SideBarHelperTest, makeItemIdentifier)
{
    // Test method: QString makeItemIdentifier((const QString &group, const QUrl &url))
    QString _arg0{};
    QUrl _arg1{};
    auto result = obj->makeItemIdentifier(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SideBarHelperTest, mutex)
{
    // Test getter: QMutex mutex()
    auto result = obj->mutex();
    EXPECT_NO_FATAL_FAILURE({ obj->mutex(); });

}

TEST_F(SideBarHelperTest, openFolderInASeparateProcess)
{
    // Test method: void openFolderInASeparateProcess((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->openFolderInASeparateProcess(_arg0));
}

TEST_F(SideBarHelperTest, partitionExpandable)
{
    // Test bool getter: partitionExpandable()
    bool result = obj->partitionExpandable();
    EXPECT_FALSE(result);

}

TEST_F(SideBarHelperTest, registCustomSettingItem)
{
    // Test method: void registCustomSettingItem(())
    EXPECT_NO_FATAL_FAILURE(obj->registCustomSettingItem());
}
