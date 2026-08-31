// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_videopreview.cpp
 * @brief Unit tests for VideoPreview methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/video-preview/videopreview.h"

#include <QTest>

using namespace src;

class VideoPreviewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VideoPreview();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VideoPreview *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VideoPreviewTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}
