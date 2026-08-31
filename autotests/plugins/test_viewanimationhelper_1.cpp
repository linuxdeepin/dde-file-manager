// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewanimationhelper_1.cpp
 * @brief Unit tests for ViewAnimationHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/viewanimationhelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class ViewAnimationHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewAnimationHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewAnimationHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewAnimationHelperTest, aboutToPlay)
{
    // Test method: void aboutToPlay(())
    EXPECT_NO_FATAL_FAILURE(obj->aboutToPlay());
}

TEST_F(ViewAnimationHelperTest, calcIndexRects)
{
    // Test method: QMap<QModelIndex, QRect> calcIndexRects((const QRect &rect))
    QRect _arg0{};
    auto result = obj->calcIndexRects(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ViewAnimationHelperTest, createPixmapsForVisiableRect)
{
    // Test method: void createPixmapsForVisiableRect(())
    EXPECT_NO_FATAL_FAILURE(obj->createPixmapsForVisiableRect());
}

TEST_F(ViewAnimationHelperTest, getCurrentRectByIndex)
{
    // Test method: QRect getCurrentRectByIndex((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->getCurrentRectByIndex(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ViewAnimationHelperTest, hasInitialized)
{
    // Test bool getter: hasInitialized()
    bool result = obj->hasInitialized();
    EXPECT_FALSE(result);

}

TEST_F(ViewAnimationHelperTest, initAnimationHelper)
{
    // Test method: void initAnimationHelper(())
    EXPECT_NO_FATAL_FAILURE(obj->initAnimationHelper());
}

TEST_F(ViewAnimationHelperTest, isAnimationPlaying)
{
    // Test bool getter: isAnimationPlaying()
    bool result = obj->isAnimationPlaying();
    EXPECT_FALSE(result);

}

TEST_F(ViewAnimationHelperTest, isWaitingToPlaying)
{
    // Test bool getter: isWaitingToPlaying()
    bool result = obj->isWaitingToPlaying();
    EXPECT_FALSE(result);

}

TEST_F(ViewAnimationHelperTest, onAnimationTimerFinish)
{
    // Test method: void onAnimationTimerFinish(())
    EXPECT_NO_FATAL_FAILURE(obj->onAnimationTimerFinish());
}

TEST_F(ViewAnimationHelperTest, onAnimationValueChanged)
{
    // Test method: void onAnimationValueChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onAnimationValueChanged());
}

TEST_F(ViewAnimationHelperTest, onDelayTimerFinish)
{
    // Test method: void onDelayTimerFinish(())
    EXPECT_NO_FATAL_FAILURE(obj->onDelayTimerFinish());
}

TEST_F(ViewAnimationHelperTest, playAnimationWithWidthChange)
{
    // Test method: void playAnimationWithWidthChange((int deltaWidth))
    EXPECT_NO_FATAL_FAILURE(obj->playAnimationWithWidthChange(0));
}

TEST_F(ViewAnimationHelperTest, setAnimProcess)
{
    // Test setter: void setAnimProcess((double value))
    EXPECT_NO_FATAL_FAILURE(obj->setAnimProcess(0.0));
}

TEST_F(ViewAnimationHelperTest, suppressNextResizeAnimation)
{
    // Test method: void suppressNextResizeAnimation(())
    EXPECT_NO_FATAL_FAILURE(obj->suppressNextResizeAnimation());
}

TEST_F(ViewAnimationHelperTest, syncVisiableRect)
{
    // Test method: void syncVisiableRect(())
    EXPECT_NO_FATAL_FAILURE(obj->syncVisiableRect());
}

TEST_F(ViewAnimationHelperTest, updateResizeAnimationBaseline)
{
    // Test method: bool updateResizeAnimationBaseline((const QSize &oldSize, const QSize &newSize))
    QSize _arg0{};
    QSize _arg1{};
    auto result = obj->updateResizeAnimationBaseline(_arg0, _arg1);
    EXPECT_FALSE(result);

}
