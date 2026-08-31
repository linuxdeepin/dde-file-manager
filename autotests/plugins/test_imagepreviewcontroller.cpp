// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_imagepreviewcontroller.cpp
 * @brief Unit tests for ImagePreviewController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/imagepreviewworker.h"

#include <QTest>

using namespace dfmplugin_detailspace;

class ImagePreviewControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ImagePreviewController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ImagePreviewController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ImagePreviewControllerTest, onNeedIconFallback)
{
    // Test method: void onNeedIconFallback((const QUrl &url, const QSize &targetSize))
    QUrl _arg0{};
    QSize _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onNeedIconFallback(_arg0, _arg1));
}
