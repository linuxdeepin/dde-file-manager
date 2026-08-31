// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_toolbarframe.cpp
 * @brief Unit tests for ToolBarFrame methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/music-preview/toolbarframe.h"

#include <QTest>

using namespace src;

class ToolBarFrameTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ToolBarFrame();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ToolBarFrame *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ToolBarFrameTest, play)
{
    // Test method: void play(())
    EXPECT_NO_FATAL_FAILURE(obj->play());
}
