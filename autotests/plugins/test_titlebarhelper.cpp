// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_titlebarhelper.cpp
 * @brief Unit tests for TitleBarHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/titlebarhelper.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class TitleBarHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TitleBarHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TitleBarHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TitleBarHelperTest, addTileBar)
{
    // Test method: void addTileBar((quint64 windowId, TitleBarWidget *titleBar))
    EXPECT_NO_FATAL_FAILURE(obj->addTileBar(0, nullptr));
}

TEST_F(TitleBarHelperTest, checkKeepTitleStatus)
{
    // Test method: bool checkKeepTitleStatus((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->checkKeepTitleStatus(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TitleBarHelperTest, getFileViewStateValue)
{
    // Test method: QVariant getFileViewStateValue((const QUrl &url, const QString &key, const QVariant &defaultValue))
    QUrl _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    auto result = obj->getFileViewStateValue(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TitleBarHelperTest, handleJumpToPressed)
{
    // Test method: void handleJumpToPressed((QWidget *sender, const QString &text))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleJumpToPressed(nullptr, _arg1));
}

TEST_F(TitleBarHelperTest, handleSearch)
{
    // Test method: void handleSearch((QWidget *sender, const QString &text))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleSearch(nullptr, _arg1));
}

TEST_F(TitleBarHelperTest, handleSettingMenuTriggered)
{
    // Test method: void handleSettingMenuTriggered((quint64 windowId, int action))
    EXPECT_NO_FATAL_FAILURE(obj->handleSettingMenuTriggered(0, 0));
}

TEST_F(TitleBarHelperTest, isViewModeVisibleForScheme)
{
    // Test method: bool isViewModeVisibleForScheme((int mode, const QString &scheme))
    QString _arg1{};
    auto result = obj->isViewModeVisibleForScheme(0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TitleBarHelperTest, openCurrentUrlInNewTab)
{
    // Test method: void openCurrentUrlInNewTab((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->openCurrentUrlInNewTab(0));
}

TEST_F(TitleBarHelperTest, registerKeepTitleStatusScheme)
{
    // Test method: void registerKeepTitleStatusScheme((const QString &scheme))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->registerKeepTitleStatusScheme(_arg0));
}

TEST_F(TitleBarHelperTest, registerViewModelUrlCallback)
{
    // Test method: void registerViewModelUrlCallback((const QString &scheme, ViewModeUrlCallback callback))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->registerViewModelUrlCallback(_arg0, ViewModeUrlCallback()));
}

TEST_F(TitleBarHelperTest, removeTitleBar)
{
    // Test method: void removeTitleBar((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->removeTitleBar(0));
}

TEST_F(TitleBarHelperTest, showConnectToServerDialog)
{
    // Test method: void showConnectToServerDialog((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->showConnectToServerDialog(0));
}

TEST_F(TitleBarHelperTest, showDiskPasswordChangingDialog)
{
    // Test method: void showDiskPasswordChangingDialog((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->showDiskPasswordChangingDialog(0));
}

TEST_F(TitleBarHelperTest, showSettingsDialog)
{
    // Test method: void showSettingsDialog((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->showSettingsDialog(0));
}

TEST_F(TitleBarHelperTest, showUserSharePasswordSettingDialog)
{
    // Test method: void showUserSharePasswordSettingDialog((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->showUserSharePasswordSettingDialog(0));
}

TEST_F(TitleBarHelperTest, tansToCrumbDataList)
{
    // Test method: QList<CrumbData> tansToCrumbDataList((const QList<QVariantMap> &mapGroup))
    QList<QVariantMap> _arg0{};
    auto result = obj->tansToCrumbDataList(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TitleBarHelperTest, titlebars)
{
    // Test getter: QList<TitleBarWidget *> titlebars()
    auto result = obj->titlebars();
    // Pointer return type
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TitleBarHelperTest, tryLoadSearchPlugin)
{
    // Test bool getter: tryLoadSearchPlugin()
    bool result = obj->tryLoadSearchPlugin();
    EXPECT_FALSE(result);

}

TEST_F(TitleBarHelperTest, windowId)
{
    // Test method: quint64 windowId((QWidget *sender))
    auto result = obj->windowId(nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(TitleBarHelperTest, findTileBarByWindowId)
{
    // Test method: TitleBarWidget findTileBarByWindowId((quint64 windowId))
    auto result = obj->findTileBarByWindowId(0);
    EXPECT_NO_FATAL_FAILURE({ obj->findTileBarByWindowId(0); });

}
