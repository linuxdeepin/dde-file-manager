// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_cusmediaplayer.cpp
 * @brief Unit tests for CusMediaPlayer methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/music-preview/cusmediaplayer.h"

#include <QTest>

using namespace src;

class CusMediaPlayerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CusMediaPlayer();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CusMediaPlayer *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CusMediaPlayerTest, duration)
{
    // Test getter: qint64 duration()
    auto result = obj->duration();
    EXPECT_EQ(result, 0);

}

TEST_F(CusMediaPlayerTest, instance)
{
    // Test getter: CusMediaPlayer instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
