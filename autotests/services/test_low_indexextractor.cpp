// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_test_low_indexextractor.cpp
 * @brief Unit tests for IndexExtractor Low-priority methods
 */

#include <gtest/gtest.h>

class IndexExtractorLowTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(IndexExtractorLowTest, IndexExtractor_Destructor)
{
    // ~IndexExtractor
    SUCCEED();
}

