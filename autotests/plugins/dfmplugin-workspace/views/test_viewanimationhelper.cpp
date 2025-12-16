// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "utils/viewanimationhelper.h"
#include "views/fileview.h"
#include "views/baseitemdelegate.h"

#include <QAbstractItemView>
#include <QPropertyAnimation>
#include <QTimer>
#include <QModelIndex>
#include <QRect>
#include <QPoint>
#include <QPixmap>
#include <QMap>

using namespace dfmplugin_workspace;

class ViewAnimationHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        testUrl = QUrl::fromLocalFile("/tmp/test");
        fileView = new FileView(testUrl);
        animationHelper = new ViewAnimationHelper(fileView);
    }

    void TearDown() override
    {
        delete animationHelper;
        delete fileView;
        stub.clear();
    }

    QUrl testUrl;
    FileView *fileView;
    ViewAnimationHelper *animationHelper;
    stub_ext::StubExt stub;
};

TEST_F(ViewAnimationHelperTest, Constructor_SetsProperties)
{
    EXPECT_NE(animationHelper, nullptr);
    EXPECT_EQ(animationHelper->parent(), fileView);
}

TEST_F(ViewAnimationHelperTest, CanGetAnimProcess)
{
    double result = animationHelper->getAnimProcess();
    
    // Should return default value
    EXPECT_DOUBLE_EQ(result, 0.0);
}

TEST_F(ViewAnimationHelperTest, CanSetAnimProcess)
{
    double testValue = 0.5;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationHelper->setAnimProcess(testValue);
    });
    
    // Verify that the value was set
    EXPECT_DOUBLE_EQ(animationHelper->getAnimProcess(), testValue);
}

TEST_F(ViewAnimationHelperTest, CanCheckIfAnimationPlaying)
{
    bool result = animationHelper->isAnimationPlaying();
    
    // Should return false initially
    EXPECT_FALSE(result);
}

TEST_F(ViewAnimationHelperTest, CanCheckIfWaitingToPlaying)
{
    bool result = animationHelper->isWaitingToPlaying();
    
    // Should return false initially
    EXPECT_FALSE(result);
}

TEST_F(ViewAnimationHelperTest, CanCheckIfHasInitialized)
{
    bool result = animationHelper->hasInitialized();
    
    // Should return false initially
    EXPECT_FALSE(result);
}

TEST_F(ViewAnimationHelperTest, CanGetCurrentRectByIndex)
{
    QModelIndex index;
    
    QRect result = animationHelper->getCurrentRectByIndex(index);
    
    // Should return empty rect for invalid index
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(ViewAnimationHelperTest, CanInitAnimationHelper)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationHelper->initAnimationHelper();
    });
}

TEST_F(ViewAnimationHelperTest, CanReset)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationHelper->reset();
    });
}

TEST_F(ViewAnimationHelperTest, CanSyncVisibleRect)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationHelper->syncVisiableRect();
    });
}

TEST_F(ViewAnimationHelperTest, CanAboutToPlay)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationHelper->aboutToPlay();
    });
}

TEST_F(ViewAnimationHelperTest, CanPlayViewAnimation)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationHelper->playViewAnimation();
    });
}

TEST_F(ViewAnimationHelperTest, CanPlayAnimationWithWidthChange)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationHelper->playAnimationWithWidthChange(10);
    });
}

TEST_F(ViewAnimationHelperTest, CanPaintItems)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationHelper->paintItems();
    });
}

TEST_F(ViewAnimationHelperTest, CanOnDelayTimerFinish)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationHelper->onDelayTimerFinish();
    });
}

TEST_F(ViewAnimationHelperTest, CanOnAnimationValueChanged)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationHelper->onAnimationValueChanged();
    });
}

TEST_F(ViewAnimationHelperTest, CanOnAnimationTimerFinish)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        animationHelper->onAnimationTimerFinish();
    });
}
