// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_wallaperpreview_1.cpp
 * @brief Unit tests for WallaperPreview methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "wallaperpreview.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class WallaperPreviewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WallaperPreview();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WallaperPreview *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WallaperPreviewTest, createWidget)
{
    // Test method: PreviewWidgetPtr createWidget((ScreenPointer sc))
    auto result = obj->createWidget(ScreenPointer());
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(WallaperPreviewTest, getBackground)
{
    // Test method: QString getBackground((const QString &screen))
    QString _arg0{};
    auto result = obj->getBackground(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WallaperPreviewTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(WallaperPreviewTest, isVisible)
{
    // Test bool getter: isVisible()
    bool result = obj->isVisible();
    EXPECT_FALSE(result);

}

TEST_F(WallaperPreviewTest, pullImageSettings)
{
    // Test method: void pullImageSettings(())
    EXPECT_NO_FATAL_FAILURE(obj->pullImageSettings());
}

TEST_F(WallaperPreviewTest, updateGeometry)
{
    // Test method: void updateGeometry(())
    EXPECT_NO_FATAL_FAILURE(obj->updateGeometry());
}

TEST_F(WallaperPreviewTest, widget)
{
    // Test getter: PreviewWidgetPtr widget()
    auto result = obj->widget();
    EXPECT_EQ(result.get(), nullptr);

}
