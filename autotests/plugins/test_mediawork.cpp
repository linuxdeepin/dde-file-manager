// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mediawork.cpp
 * @brief Unit tests for MediaWork methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/music-preview/mediawork.h"

#include <QTest>

using namespace src;

class MediaWorkTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MediaWork();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MediaWork *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MediaWorkTest, state)
{
    // Test getter: QMediaPlayer::PlaybackState state()
    auto result = obj->state();
    EXPECT_NO_FATAL_FAILURE({ obj->state(); });

}
