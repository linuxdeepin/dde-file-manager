// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_customwatermasklabel_1.cpp
 * @brief Unit tests for CustomWaterMaskLabel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "watermask/customwatermasklabel.h"

#include <QTest>

using namespace ddplugin_canvas;

class CustomWaterMaskLabelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CustomWaterMaskLabel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CustomWaterMaskLabel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CustomWaterMaskLabelTest, CustomWaterMaskLabel)
{
    // Test constructor: CustomWaterMaskLabel((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CustomWaterMaskLabelTest, maskPixmap)
{
    // Test method: QPixmap maskPixmap((const QString &uri, const QSize &size, qreal pixelRatio))
    QString _arg0{};
    QSize _arg1{};
    auto result = obj->maskPixmap(_arg0, _arg1, 0.0);
    EXPECT_TRUE(result.isNull());

}

TEST_F(CustomWaterMaskLabelTest, onConfigChanged)
{
    // Test method: void onConfigChanged((const QString &cfg, const QString &key))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onConfigChanged(_arg0, _arg1));
}

TEST_F(CustomWaterMaskLabelTest, onSystemMaskShow)
{
    // Test method: void onSystemMaskShow((const QPoint &pos))
    QPoint _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onSystemMaskShow(_arg0));
}

TEST_F(CustomWaterMaskLabelTest, refresh)
{
    // Test method: void refresh(())
    EXPECT_NO_FATAL_FAILURE(obj->refresh());
}

TEST_F(CustomWaterMaskLabelTest, CustomWaterMaskLabel_Destructor)
{
    // Test method:  ~CustomWaterMaskLabel(())
    EXPECT_NO_FATAL_FAILURE({ CustomWaterMaskLabel *tmp = new CustomWaterMaskLabel(); delete tmp; });
}
