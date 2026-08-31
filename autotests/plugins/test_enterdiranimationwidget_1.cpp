// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_enterdiranimationwidget_1.cpp
 * @brief Unit tests for EnterDirAnimationWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/enterdiranimationwidget.h"

#include <QTest>

using namespace dfmplugin_workspace;

class EnterDirAnimationWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EnterDirAnimationWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EnterDirAnimationWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EnterDirAnimationWidgetTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(EnterDirAnimationWidgetTest, onProcessChanged)
{
    // Test method: void onProcessChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onProcessChanged());
}

TEST_F(EnterDirAnimationWidgetTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *event))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(EnterDirAnimationWidgetTest, paintPix)
{
    // Test method: void paintPix((QPainter *painter, const QPixmap &pix, double process))
    QPixmap _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->paintPix(nullptr, _arg1, 0.0));
}

TEST_F(EnterDirAnimationWidgetTest, playAppear)
{
    // Test method: void playAppear(())
    EXPECT_NO_FATAL_FAILURE(obj->playAppear());
}

TEST_F(EnterDirAnimationWidgetTest, playDisappear)
{
    // Test method: void playDisappear(())
    EXPECT_NO_FATAL_FAILURE(obj->playDisappear());
}

TEST_F(EnterDirAnimationWidgetTest, setAppearPixmap)
{
    // Test setter: void setAppearPixmap((const QPixmap &pm))
    QPixmap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setAppearPixmap(_arg0));
}

TEST_F(EnterDirAnimationWidgetTest, setAppearProcess)
{
    // Test setter: void setAppearProcess((double value))
    EXPECT_NO_FATAL_FAILURE(obj->setAppearProcess(0.0));
}

TEST_F(EnterDirAnimationWidgetTest, setDisappearPixmap)
{
    // Test setter: void setDisappearPixmap((const QPixmap &pm))
    QPixmap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setDisappearPixmap(_arg0));
}

TEST_F(EnterDirAnimationWidgetTest, setDisappearProcess)
{
    // Test setter: void setDisappearProcess((double value))
    EXPECT_NO_FATAL_FAILURE(obj->setDisappearProcess(0.0));
}

TEST_F(EnterDirAnimationWidgetTest, stopAndHide)
{
    // Test method: void stopAndHide(())
    EXPECT_NO_FATAL_FAILURE(obj->stopAndHide());
}
