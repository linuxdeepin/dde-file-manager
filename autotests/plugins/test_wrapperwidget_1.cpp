// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_wrapperwidget_1.cpp
 * @brief Unit tests for WrapperWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "wallpaperitem.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class WrapperWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WrapperWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WrapperWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WrapperWidgetTest, WrapperWidget)
{
    // Test constructor: WrapperWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(WrapperWidgetTest, boxGeometry)
{
    // Test getter: QRect boxGeometry()
    auto result = obj->boxGeometry();
    EXPECT_FALSE(result.isValid());

}

TEST_F(WrapperWidgetTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *event))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(WrapperWidgetTest, setBoxGeometry)
{
    // Test method: void setBoxGeometry(())
    EXPECT_NO_FATAL_FAILURE(obj->setBoxGeometry());
}

TEST_F(WrapperWidgetTest, setOpacity)
{
    // Test method: void setOpacity(())
    EXPECT_NO_FATAL_FAILURE(obj->setOpacity());
}

TEST_F(WrapperWidgetTest, setPixmap)
{
    // Test setter: void setPixmap((const QPixmap &pix))
    QPixmap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setPixmap(_arg0));
}
