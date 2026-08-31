// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iconutils.cpp
 * @brief Unit tests for IconUtils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/utils/iconutils.h"

#include <QTest>

using namespace src;

class IconUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new IconUtils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    IconUtils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IconUtilsTest, addShadowToPixmap)
{
    // Test method: QPixmap addShadowToPixmap((const QPixmap &originalPixmap, int shadowOffsetY, qreal blurRadius, qreal shadowOpacity))
    QPixmap _arg0{};
    auto result = obj->addShadowToPixmap(_arg0, 0, 0.0, 0.0);
    EXPECT_TRUE(result.isNull());

}

TEST_F(IconUtilsTest, getIconStyle)
{
    // Test method: IconUtils::IconStyle getIconStyle((int size))
    auto result = obj->getIconStyle(0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(IconUtilsTest, hiDpiPixmap)
{
    // Test method: QPixmap hiDpiPixmap((const QIcon &icon, const QSize &size, const QWidget *widget))
    QIcon _arg0{};
    QSize _arg1{};
    auto result = obj->hiDpiPixmap(_arg0, _arg1, nullptr);
    EXPECT_TRUE(result.isNull());

}

TEST_F(IconUtilsTest, renderIconBackground)
{
    // Test method: QPixmap renderIconBackground((const QSizeF &size, const IconStyle &style))
    QSizeF _arg0{};
    IconStyle _arg1{};
    auto result = obj->renderIconBackground(_arg0, _arg1);
    EXPECT_TRUE(result.isNull());

}

TEST_F(IconUtilsTest, shouldSkipThumbnailFrame)
{
    // Test method: bool shouldSkipThumbnailFrame((const QString &mimeType))
    QString _arg0{};
    auto result = obj->shouldSkipThumbnailFrame(_arg0);
    EXPECT_FALSE(result);

}
