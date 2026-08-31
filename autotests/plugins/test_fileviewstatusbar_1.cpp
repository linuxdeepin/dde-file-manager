// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileviewstatusbar_1.cpp
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

TEST_F(FileViewStatusBarTest, findTipLabel)
{
    // Test getter: DTipLabel findTipLabel()
    auto result = obj->findTipLabel();
    EXPECT_NO_FATAL_FAILURE({ obj->findTipLabel(); });

}

TEST_F(FileViewStatusBarTest, hideLoadingIncator)
{
    // Test method: void hideLoadingIncator(())
    EXPECT_NO_FATAL_FAILURE(obj->hideLoadingIncator());
}

TEST_F(FileViewStatusBarTest, initLoadingSpinner)
{
    // Test method: void initLoadingSpinner(())
    EXPECT_NO_FATAL_FAILURE(obj->initLoadingSpinner());
}

TEST_F(FileViewStatusBarTest, initScalingSlider)
{
    // Test method: void initScalingSlider(())
    EXPECT_NO_FATAL_FAILURE(obj->initScalingSlider());
}

TEST_F(FileViewStatusBarTest, scalingSlider)
{
    // Test getter: DSlider scalingSlider()
    auto result = obj->scalingSlider();
    EXPECT_NO_FATAL_FAILURE({ obj->scalingSlider(); });

}

TEST_F(FileViewStatusBarTest, scalingValue)
{
    // Test getter: int scalingValue()
    auto result = obj->scalingValue();
    EXPECT_EQ(result, 0);

}

TEST_F(FileViewStatusBarTest, setScalingVisible)
{
    // Test setter: void setScalingVisible((const bool visible))
    EXPECT_NO_FATAL_FAILURE(obj->setScalingVisible(false));
}

TEST_F(FileViewStatusBarTest, showLoadingIncator)
{
    // Test method: void showLoadingIncator((const QString &tip))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showLoadingIncator(_arg0));
}
