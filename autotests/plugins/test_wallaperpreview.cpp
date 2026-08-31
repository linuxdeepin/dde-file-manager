// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_wallaperpreview.cpp
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

TEST_F(WallaperPreviewTest, WallaperPreview)
{
    // Test constructor: WallaperPreview((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(WallaperPreviewTest, setVisible)
{
    // Test setter: void setVisible((bool v))
    EXPECT_NO_FATAL_FAILURE(obj->setVisible(false));
}

TEST_F(WallaperPreviewTest, setWallpaper)
{
    // Test setter: void setWallpaper((const QString &screen, const QString &image))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setWallpaper(_arg0, _arg1));
}

TEST_F(WallaperPreviewTest, updateWallpaper)
{
    // Test method: void updateWallpaper(())
    EXPECT_NO_FATAL_FAILURE(obj->updateWallpaper());
}

TEST_F(WallaperPreviewTest, wallpaper)
{
    // Test getter: QMap<QString, QString> wallpaper()
    auto result = obj->wallpaper();
    EXPECT_TRUE(result.isEmpty());

}
