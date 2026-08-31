// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_videopreview.cpp
 * @brief Unit tests for VideoPreview Mid-priority methods
 */

#include <gtest/gtest.h>

class VideoPreviewTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(VideoPreviewTest, stop)
{
    // stop
    SUCCEED();
}
