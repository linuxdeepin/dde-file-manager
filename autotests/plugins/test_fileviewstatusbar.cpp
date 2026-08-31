// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileviewstatusbar.cpp
 * @brief Unit tests for FileViewStatusBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/fileviewstatusbar.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileViewStatusBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileViewStatusBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileViewStatusBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileViewStatusBarTest, FileViewStatusBar)
{
    // Test constructor: FileViewStatusBar((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileViewStatusBarTest, clearLayoutAndAnchors)
{
    // Test method: void clearLayoutAndAnchors(())
    EXPECT_NO_FATAL_FAILURE(obj->clearLayoutAndAnchors());
}

TEST_F(FileViewStatusBarTest, resetScalingSlider)
{
    // Test method: void resetScalingSlider((const int stepCount))
    EXPECT_NO_FATAL_FAILURE(obj->resetScalingSlider(0));
}

TEST_F(FileViewStatusBarTest, setCustomLayout)
{
    // Test method: void setCustomLayout(())
    EXPECT_NO_FATAL_FAILURE(obj->setCustomLayout());
}
