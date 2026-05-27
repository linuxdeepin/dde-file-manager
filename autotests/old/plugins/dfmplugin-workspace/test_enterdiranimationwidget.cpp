// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "views/enterdiranimationwidget.h"

#include <QWidget>
#include <QPixmap>
#include <QSize>
#include <QPaintEvent>
#include <QPropertyAnimation>

using namespace dfmplugin_workspace;

class EnterDirAnimationWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        animationWidget = new EnterDirAnimationWidget();
    }

    void TearDown() override
    {
        if (animationWidget) {
            delete animationWidget;
            animationWidget = nullptr;
        }
        stub.clear();
    }

    EnterDirAnimationWidget *animationWidget { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(EnterDirAnimationWidgetTest, Constructor_SetsDefaultValues)
{
    EXPECT_NE(animationWidget, nullptr);
    EXPECT_NEAR(animationWidget->getAppearProcess(), 0.0, 0.01);
    EXPECT_NEAR(animationWidget->getDisappearProcess(), 0.0, 0.01);
}

TEST_F(EnterDirAnimationWidgetTest, SetAppearPixmap_SetsPixmap)
{
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::red);
    
    animationWidget->setAppearPixmap(pixmap);
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationWidget->setAppearPixmap(pixmap);
    });
}

TEST_F(EnterDirAnimationWidgetTest, SetDisappearPixmap_SetsPixmap)
{
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::blue);
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationWidget->setDisappearPixmap(pixmap);
    });
}

TEST_F(EnterDirAnimationWidgetTest, ResetWidgetSize_SetsSize)
{
    QSize size(200, 150);
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationWidget->resetWidgetSize(size);
    });
}

TEST_F(EnterDirAnimationWidgetTest, PlayAppear_PlaysAnimation)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationWidget->playAppear();
    });
}

TEST_F(EnterDirAnimationWidgetTest, PlayDisappear_PlaysAnimation)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationWidget->playDisappear();
    });
}

TEST_F(EnterDirAnimationWidgetTest, StopAndHide_StopsAndHides)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationWidget->stopAndHide();
    });
}

TEST_F(EnterDirAnimationWidgetTest, SetAppearProcess_SetsValue)
{
    double process = 0.5;
    
    animationWidget->setAppearProcess(process);
    
    EXPECT_NEAR(animationWidget->getAppearProcess(), process, 0.01);
}

TEST_F(EnterDirAnimationWidgetTest, GetAppearProcess_ReturnsValue)
{
    double process = 0.75;
    animationWidget->setAppearProcess(process);
    
    double result = animationWidget->getAppearProcess();
    
    EXPECT_NEAR(result, process, 0.01);
}

TEST_F(EnterDirAnimationWidgetTest, SetDisappearProcess_SetsValue)
{
    double process = 0.3;
    
    animationWidget->setDisappearProcess(process);
    
    EXPECT_NEAR(animationWidget->getDisappearProcess(), process, 0.01);
}

TEST_F(EnterDirAnimationWidgetTest, GetDisappearProcess_ReturnsValue)
{
    double process = 0.8;
    animationWidget->setDisappearProcess(process);
    
    double result = animationWidget->getDisappearProcess();
    
    EXPECT_NEAR(result, process, 0.01);
}

TEST_F(EnterDirAnimationWidgetTest, PaintEvent_DoesNotCrash)
{
    QPaintEvent event(QRect(0, 0, 100, 100));
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationWidget->paintEvent(&event);
    });
}
