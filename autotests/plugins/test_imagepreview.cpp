// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_imagepreview.cpp
 * @brief Unit tests for ImagePreview methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/image-preview/imagepreview.h"

#include <QTest>

using namespace src;

class ImagePreviewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ImagePreview();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ImagePreview *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ImagePreviewTest, setFileUrl)
{
    // Test method: bool setFileUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->setFileUrl(_arg0);
    EXPECT_FALSE(result);

}
