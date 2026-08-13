// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_indexextractor.cpp
 * @brief Unit tests for IndexExtractor interface and IndexExtractionResult
 */

#include <gtest/gtest.h>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/extractor/indexextractor.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

TEST(IndexExtractionResultTest, DefaultValues)
{
    IndexExtractionResult result;
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.text.isEmpty());
    EXPECT_TRUE(result.error.isEmpty());
    EXPECT_TRUE(result.checksum.isEmpty());
    EXPECT_FALSE(result.deduplicated);
}

TEST(IndexExtractionResultTest, SuccessResult)
{
    IndexExtractionResult result;
    result.success = true;
    result.text = "hello world";
    result.checksum = "abc123";
    result.deduplicated = true;

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.text, "hello world");
    EXPECT_EQ(result.checksum, "abc123");
    EXPECT_TRUE(result.deduplicated);
}

TEST(IndexExtractionResultTest, ErrorResult)
{
    IndexExtractionResult result;
    result.success = false;
    result.error = "File not found";

    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.text.isEmpty());
    EXPECT_EQ(result.error, "File not found");
}