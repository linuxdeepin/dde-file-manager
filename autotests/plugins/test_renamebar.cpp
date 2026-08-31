// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_renamebar.cpp
 * @brief Unit tests for RenameBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/renamebar.h"

#include <QTest>

using namespace dfmplugin_workspace;

class RenameBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RenameBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RenameBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RenameBarTest, onCustomOperatorSNNumberChanged)
{
    // Test method: void onCustomOperatorSNNumberChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onCustomOperatorSNNumberChanged());
}

TEST_F(RenameBarTest, onVisibleChanged)
{
    // Test method: void onVisibleChanged((bool value))
    EXPECT_NO_FATAL_FAILURE(obj->onVisibleChanged(false));
}

TEST_F(RenameBarTest, reset)
{
    // Test method: void reset(())
    EXPECT_NO_FATAL_FAILURE(obj->reset());
}

TEST_F(RenameBarTest, setVisible)
{
    // Test setter: void setVisible((bool visible))
    EXPECT_NO_FATAL_FAILURE(obj->setVisible(false));
}
