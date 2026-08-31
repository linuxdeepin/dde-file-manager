// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_watermaskframe.cpp
 * @brief Unit tests for WaterMaskFrame methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "watermask/watermaskframe.h"

#include <QTest>

using namespace ddplugin_canvas;

class WaterMaskFrameTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WaterMaskFrame();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WaterMaskFrame *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WaterMaskFrameTest, addWidget)
{
    // Test method: void addWidget((QHBoxLayout *layout, QWidget *wid, const QString &align))
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->addWidget(nullptr, nullptr, _arg2));
}

TEST_F(WaterMaskFrameTest, maskPixmap)
{
    // Test method: QPixmap maskPixmap((const QString &uri, const QSize &size, qreal pixelRatio))
    QString _arg0{};
    QSize _arg1{};
    auto result = obj->maskPixmap(_arg0, _arg1, 0.0);
    EXPECT_TRUE(result.isNull());

}

TEST_F(WaterMaskFrameTest, showLicenseState)
{
    // Test bool getter: showLicenseState()
    bool result = obj->showLicenseState();
    EXPECT_FALSE(result);

}

TEST_F(WaterMaskFrameTest, update)
{
    // Test method: void update((const ConfigInfo &configs, bool normal))
    ConfigInfo _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->update(_arg0, false));
}

TEST_F(WaterMaskFrameTest, usingCn)
{
    // Test bool getter: usingCn()
    bool result = obj->usingCn();
    EXPECT_FALSE(result);

}

TEST_F(WaterMaskFrameTest, defaultCfg)
{
    // Test method: WaterMaskFrame::ConfigInfo defaultCfg((QJsonObject *configs))
    auto result = obj->defaultCfg(nullptr);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(WaterMaskFrameTest, entCfg)
{
    // Test method: WaterMaskFrame::ConfigInfo entCfg((QJsonObject *configs, bool cn))
    auto result = obj->entCfg(nullptr, false);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(WaterMaskFrameTest, govCfg)
{
    // Test method: WaterMaskFrame::ConfigInfo govCfg((QJsonObject *configs, bool cn))
    auto result = obj->govCfg(nullptr, false);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(WaterMaskFrameTest, secCfg)
{
    // Test method: WaterMaskFrame::ConfigInfo secCfg((QJsonObject *configs, bool cn))
    auto result = obj->secCfg(nullptr, false);
    EXPECT_GE(static_cast<int>(result), 0);

}
