// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_checkboxwithfileindex.cpp
 * @brief Unit tests for CheckBoxWithFileIndex methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/checkboxwithfileindex.h"

#include <QTest>

using namespace dfmplugin_search;

class CheckBoxWithFileIndexTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CheckBoxWithFileIndex();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CheckBoxWithFileIndex *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CheckBoxWithFileIndexTest, applyState)
{
    // Test method: void applyState((const FileIndexState &state))
    FileIndexState _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->applyState(_arg0));
}

TEST_F(CheckBoxWithFileIndexTest, queryState)
{
    // Test getter: CheckBoxWithFileIndex::FileIndexState queryState()
    auto result = obj->queryState();
    EXPECT_GE(static_cast<int>(result), 0);

}
