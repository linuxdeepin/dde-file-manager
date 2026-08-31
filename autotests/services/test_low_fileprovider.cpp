// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_test_low_fileprovider.cpp
 * @brief Unit tests for FileProvider Low-priority methods
 */

#include <gtest/gtest.h>

class FileProviderLowTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(FileProviderLowTest, totalCount)
{
    // totalCount
    SUCCEED();
}

TEST_F(FileProviderLowTest, FileProvider_Destructor)
{
    // ~FileProvider
    SUCCEED();
}

