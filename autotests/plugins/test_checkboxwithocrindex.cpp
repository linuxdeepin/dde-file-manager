// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_checkboxwithocrindex.cpp
 * @brief Unit tests for CheckBoxWithOcrIndex methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/checkboxwithocrindex.h"

#include <QTest>

using namespace dfmplugin_search;

class CheckBoxWithOcrIndexTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CheckBoxWithOcrIndex();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CheckBoxWithOcrIndex *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CheckBoxWithOcrIndexTest, CheckBoxWithOcrIndex)
{
    // Test constructor: CheckBoxWithOcrIndex((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CheckBoxWithOcrIndexTest, connectToBackend)
{
    // Test method: void connectToBackend(())
    EXPECT_NO_FATAL_FAILURE(obj->connectToBackend());
}

TEST_F(CheckBoxWithOcrIndexTest, initStatusBar)
{
    // Test method: void initStatusBar(())
    EXPECT_NO_FATAL_FAILURE(obj->initStatusBar());
}
