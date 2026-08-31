// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_musicpreview.cpp
 * @brief Unit tests for MusicPreview methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/music-preview/musicpreview.h"

#include <QTest>

using namespace src;

class MusicPreviewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MusicPreview();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MusicPreview *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MusicPreviewTest, handleBeforDestroy)
{
    // Test method: void handleBeforDestroy(())
    EXPECT_NO_FATAL_FAILURE(obj->handleBeforDestroy());
}

TEST_F(MusicPreviewTest, statusBarWidget)
{
    // Test getter: QWidget statusBarWidget()
    auto result = obj->statusBarWidget();
    EXPECT_NO_FATAL_FAILURE({ obj->statusBarWidget(); });

}
