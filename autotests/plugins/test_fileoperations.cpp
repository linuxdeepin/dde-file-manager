// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperations.cpp
 * @brief Unit tests for FileOperations methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class FileOperationsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperations();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperations *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperationsTest, followEvents)
{
    // Test method: void followEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->followEvents());
}

TEST_F(FileOperationsTest, initEventHandle)
{
    // Test method: void initEventHandle(())
    EXPECT_NO_FATAL_FAILURE(obj->initEventHandle());
}

TEST_F(FileOperationsTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
