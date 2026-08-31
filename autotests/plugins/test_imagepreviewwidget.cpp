// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_imagepreviewwidget.cpp
 * @brief Unit tests for ImagePreviewWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/imagepreviewwidget.h"

#include <QTest>

using namespace dfmplugin_detailspace;

class ImagePreviewWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ImagePreviewWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ImagePreviewWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ImagePreviewWidgetTest, pixmap)
{
    // Test getter: QPixmap pixmap()
    auto result = obj->pixmap();
    EXPECT_TRUE(result.isNull());

}
