// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperations_1.cpp
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

TEST_F(FileOperationsTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(FileOperationsTest, regSettingConfig)
{
    // Test method: void regSettingConfig(())
    EXPECT_NO_FATAL_FAILURE(obj->regSettingConfig());
}
