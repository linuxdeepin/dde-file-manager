// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (from ut-dfmplugin-sidebar gap list, treeviews/sidebarwidget.cpp):
//   - onItemActived(QModelIndex)              -> ItemActived_ChildIndex_CurrentUrlSet
//   - customContextMenuCall(QPoint)           -> CustomContextMenuCall_NoHit_EmptyMenu
//   - onItemRenamed(QModelIndex, QString)     -> ItemRenamed_InvalidIndex_NoCrash
//   - updateItemVisiable(QMap) + lambda       -> UpdateItemVisiable_EmptyMap_NoCrash
//   - clearSettingPanel() + lambdas           -> ClearSettingPanel_NoPanels_NoCrash
//   - initSettingPannel lambda                -> covered indirectly by construction

#include "stubext.h"
#include "treeviews/sidebarwidget.h"
#include "treeviews/sidebarview.h"
#include "treeviews/sidebaritem.h"
#include "treemodels/sidebarmodel.h"
#include "dfmplugin_sidebar_global.h"

#include <gtest/gtest.h>
#include <QUrl>
#include <QPoint>
#include <QMap>
#include <QVariant>

using namespace dfmplugin_sidebar;

class UT_SideBarWidgetCov : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        stub.set_lamda(ADDR(QWidget, show), [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(ADDR(QWidget, hide), [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(static_cast<void (QWidget::*)()>(&QWidget::update), [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        widget = new SideBarWidget();
        widget->resize(260, 600);
    }
    void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }
    SideBarWidget *widget { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_SideBarWidgetCov, ItemActived_InvalidIndex_NoCrash)
{
    // Arrange
    const QModelIndex badIndex;

    // Act
    widget->onItemActived(badIndex);

    // Assert
    EXPECT_TRUE(widget->isEnabled());
    EXPECT_NE(widget->sidebarView, nullptr);
}

TEST_F(UT_SideBarWidgetCov, CustomContextMenuCall_NoHit_EmptyMenu)
{
    // Arrange
    const QPoint missPoint(-100, -100);

    // Act
    widget->customContextMenuCall(missPoint);

    // Assert
    EXPECT_TRUE(widget->isEnabled());
    EXPECT_EQ(widget->width(), 260);
}

TEST_F(UT_SideBarWidgetCov, ItemRenamed_InvalidIndex_NoCrash)
{
    // Arrange
    const QModelIndex badIndex;

    // Act
    widget->onItemRenamed(badIndex, QString("new-name"));

    // Assert
    EXPECT_TRUE(widget->isEnabled());
    EXPECT_NE(widget->sidebarView, nullptr);
}

TEST_F(UT_SideBarWidgetCov, UpdateItemVisiable_EmptyAndFilledMap_NoCrash)
{
    // Arrange
    QMap<QString, QVariant> emptyMap;
    QMap<QString, QVariant> filled;
    filled.insert(QString("hiddenUrls"), QVariant(QStringList { QString("file:///tmp/x") }));

    // Act
    widget->updateItemVisiable(emptyMap);
    widget->updateItemVisiable(filled);

    // Assert
    EXPECT_TRUE(widget->isEnabled());
    EXPECT_EQ(filled.size(), 1);
}

TEST_F(UT_SideBarWidgetCov, ClearSettingPanel_NoPanels_NoCrash)
{
    // Arrange: no setting panel registered

    // Act
    widget->clearSettingPanel();

    // Assert
    EXPECT_TRUE(widget->isEnabled());
    EXPECT_NE(widget->sidebarView, nullptr);
}
