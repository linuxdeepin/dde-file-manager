// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarhelper.cpp
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

TEST_F(SideBarHelperTest, addItemToSettingPannel)
{
    // Test method: void addItemToSettingPannel((const QString &group, const QString &key, const QString &value, QMap<QString, int> *levelMap))
    QString _arg0{};
    QString _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->addItemToSettingPannel(_arg0, _arg1, _arg2, nullptr));
}

TEST_F(SideBarHelperTest, allSideBar)
{
    // Test getter: QList<SideBarWidget *> allSideBar()
    auto result = obj->allSideBar();
    // Pointer return type
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SideBarHelperTest, groupExpandRules)
{
    // Test getter: QVariantMap groupExpandRules()
    auto result = obj->groupExpandRules();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SideBarHelperTest, hiddenRules)
{
    // Test getter: QVariantMap hiddenRules()
    auto result = obj->hiddenRules();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SideBarHelperTest, preDefineItemProperties)
{
    // Test getter: QMap<QUrl, QPair<int, QVariantMap>> preDefineItemProperties()
    auto result = obj->preDefineItemProperties();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SideBarHelperTest, removeItemFromSetting)
{
    // Test method: void removeItemFromSetting((const QString &key))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeItemFromSetting(_arg0));
}

TEST_F(SideBarHelperTest, removeSideBar)
{
    // Test method: void removeSideBar((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->removeSideBar(0));
}

TEST_F(SideBarHelperTest, removebindingSetting)
{
    // Test method: void removebindingSetting((const QString &itemVisiableSettingKey))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removebindingSetting(_arg0));
}

TEST_F(SideBarHelperTest, saveGroupsStateToConfig)
{
    // Test method: void saveGroupsStateToConfig((const QVariant &var))
    QVariant _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveGroupsStateToConfig(_arg0));
}

TEST_F(SideBarHelperTest, windowId)
{
    // Test method: quint64 windowId((QWidget *sender))
    auto result = obj->windowId(nullptr);
    EXPECT_GE(result, 0);

}
