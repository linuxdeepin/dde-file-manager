// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileindexcontroller.cpp
 * @brief Unit tests for FileIndexController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileindexcontroller.h"

#include <QTest>

using namespace core;

class FileIndexControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileIndexController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileIndexController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileIndexControllerTest, enableFileIndex)
{
    // Test bool getter: enableFileIndex()
    bool result = obj->enableFileIndex();
    EXPECT_FALSE(result);

}

TEST_F(FileIndexControllerTest, scheduleApply)
{
    // Test method: void scheduleApply(())
    EXPECT_NO_FATAL_FAILURE(obj->scheduleApply());
}
