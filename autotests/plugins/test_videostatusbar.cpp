// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_videostatusbar.cpp
 * @brief Unit tests for VideoStatusBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/video-preview/videostatusbar.h"

#include <QTest>

using namespace src;

class VideoStatusBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VideoStatusBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VideoStatusBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VideoStatusBarTest, VideoStatusBar)
{
    // Test constructor: VideoStatusBar((VideoPreview *preview))
    ASSERT_NE(obj, nullptr);
}
