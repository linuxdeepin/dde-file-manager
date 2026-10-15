// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_diskpluginitem.cpp - DiskPluginItem 单元测试
// 插件 qrc 已编入测试目标，updateIcon 的资源回退图标可用。

#include <gtest/gtest.h>

#include <QApplication>
#include <QtTest>
#include <QVariant>

#include <DGuiApplicationHelper>

#include "stubext.h"

#include "diskpluginitem.h"

using namespace Dtk::Gui;

class DiskPluginItemTest : public testing::Test
{
protected:
    void SetUp() override { stub.clear(); }
    void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

// 构造 + setDockDisplayMode：显示模式记录且图标重建
TEST_F(DiskPluginItemTest, ConstructAndSetDockDisplayMode_RebuildsIcon)
{
    // Arrange / Act：dtk6 无 setThemeType，用信号驱动浅色主题
    DiskPluginItem item;
    Q_EMIT DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::LightType);
    item.setDockDisplayMode(Dock::Fashion);

    // Assert
    EXPECT_EQ(item.displayMode, Dock::Fashion);
    EXPECT_FALSE(item.icon.isNull());   // qrc 回退图标（浅色主题 → -dark 后缀）
    EXPECT_FALSE(item.getIcon().isNull());
}

// updateIcon：主题切换仍能取到图标
TEST_F(DiskPluginItemTest, UpdateIcon_ThemeSwitch_KeepsIconAvailable)
{
    // Arrange：浅色主题
    DiskPluginItem item;
    Q_EMIT DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::LightType);
    item.updateIcon();
    const QSize lightSize = item.icon.size();

    // Act：切到深色
    Q_EMIT DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::DarkType);
    item.updateIcon();

    // Assert：两种主题均能取到图标（主题命中或 qrc 回退）
    EXPECT_FALSE(item.icon.isNull());
    EXPECT_EQ(item.icon.size(), lightSize);   // 尺寸一致（同一 kWidth）
    EXPECT_FALSE(item.getIcon().isNull());
}

// paintEvent：离屏渲染不崩溃
TEST_F(DiskPluginItemTest, PaintEvent_OffscreenRender_Succeeds)
{
    // Arrange
    DiskPluginItem item;
    item.resize(20, 20);

    // Act / Assert
    const QPixmap px = item.grab();
    EXPECT_FALSE(px.isNull());
    EXPECT_EQ(px.width(), 20);
}

// resizeEvent：位置属性决定宽/高约束方向（两分支）
// 隐藏窗口的 resizeEvent 由 Qt 延迟到 show() 时派发，故两分支各建实例并显示
TEST_F(DiskPluginItemTest, ResizeEvent_PositionProperty_SetsMatchingConstraint)
{
    // Arrange：Bottom → 宽度受高度约束
    {
        DiskPluginItem item;
        QCoreApplication::instance()->setProperty(PROP_POSITION, QVariant::fromValue(Dock::Bottom));

        // Act
        item.resize(30, 24);
        item.show();
        QTest::qWait(20);

        // Assert
        EXPECT_EQ(item.maximumWidth(), 24);   // = height()
        EXPECT_EQ(item.maximumHeight(), QWIDGETSIZE_MAX);
    }

    // Arrange：Left（非 Bottom/Top）→ 高度受宽度约束
    {
        DiskPluginItem item;
        QCoreApplication::instance()->setProperty(PROP_POSITION, QVariant::fromValue(Dock::Left));

        // Act
        item.resize(30, 20);
        item.show();
        QTest::qWait(20);

        // Assert
        EXPECT_EQ(item.maximumHeight(), 30);   // = width()
        EXPECT_EQ(item.maximumWidth(), QWIDGETSIZE_MAX);
    }

    QCoreApplication::instance()->setProperty(PROP_POSITION, QVariant());   // 还原
}

// 主题变化信号触发构造中的 lambda（updateIcon）
TEST_F(DiskPluginItemTest, ThemeChange_ConstructorLambda_RefreshesIcon)
{
    // Arrange
    DiskPluginItem item;
    item.setDockDisplayMode(Dock::Efficient);

    // Act：切主题（构造中 lambda 已连接 themeTypeChanged → updateIcon）
    Q_EMIT DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::DarkType);
    QTest::qWait(20);

    // Assert
    EXPECT_FALSE(item.icon.isNull());   // lambda 执行后图标仍可用
    EXPECT_FALSE(item.getIcon().isNull());
}
