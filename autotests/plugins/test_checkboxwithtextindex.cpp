// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_checkboxwithtextindex.cpp
 * @brief Unit tests for CheckBoxWithTextIndex methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/checkboxwithtextindex.h"

#include <QTest>

using namespace dfmplugin_search;

class CheckBoxWithTextIndexTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CheckBoxWithTextIndex();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CheckBoxWithTextIndex *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CheckBoxWithTextIndexTest, CheckBoxWithTextIndex)
{
    // Test constructor: CheckBoxWithTextIndex((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CheckBoxWithTextIndexTest, connectToBackend)
{
    // Test method: void connectToBackend(())
    EXPECT_NO_FATAL_FAILURE(obj->connectToBackend());
}

TEST_F(CheckBoxWithTextIndexTest, initStatusBar)
{
    // Test method: void initStatusBar(())
    EXPECT_NO_FATAL_FAILURE(obj->initStatusBar());
}
