// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_ocrdeduplication.cpp
 * @brief Unit tests for OcrDeduplication (extractor/ocrdeduplication.cpp)
 *        Tests the lookupByTextChecksum function with various inputs.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/extractor/ocrdeduplication.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

class OcrDeduplicationTest : public testing::Test
{
protected:
    QTemporaryDir tmp;

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
    }
};

TEST_F(OcrDeduplicationTest, Lookup_EmptyChecksum_ReturnsEmpty)
{
    QString result = OcrDeduplication::lookupByTextChecksum(QString(), tmp.path());
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(OcrDeduplicationTest, Lookup_EmptyIndexDir_ReturnsEmpty)
{
    QString result = OcrDeduplication::lookupByTextChecksum("abc123", QString());
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(OcrDeduplicationTest, Lookup_BothEmpty_ReturnsEmpty)
{
    QString result = OcrDeduplication::lookupByTextChecksum(QString(), QString());
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(OcrDeduplicationTest, Lookup_NonExistentIndexDir_ReturnsEmpty)
{
    // The function catches all exceptions and returns empty
    QString result = OcrDeduplication::lookupByTextChecksum(
            "d41d8cd98f00b204e9800998ecf8427e", "/nonexistent/path/to/index");
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(OcrDeduplicationTest, Lookup_EmptyDirAsIndexDir_ReturnsEmpty)
{
    // Directory exists but is empty (not a valid Lucene index)
    QString result = OcrDeduplication::lookupByTextChecksum(
            "d41d8cd98f00b204e9800998ecf8427e", tmp.path());
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(OcrDeduplicationTest, Lookup_ChecksumWithSpecialChars)
{
    QString result = OcrDeduplication::lookupByTextChecksum(
            "!@#$%^&*()_+{}|:\"<>?", tmp.path());
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(OcrDeduplicationTest, Lookup_LongChecksum)
{
    QString longChecksum(1000, 'a');
    QString result = OcrDeduplication::lookupByTextChecksum(longChecksum, tmp.path());
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(OcrDeduplicationTest, Lookup_ValidMd5Checksum_NonIndexDir)
{
    // Valid MD5 format but directory is not a Lucene index
    QString result = OcrDeduplication::lookupByTextChecksum(
            "5d41402abc4b2a76b9719d911017c592", tmp.path());
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(OcrDeduplicationTest, Lookup_UnicodeChecksum)
{
    QString result = OcrDeduplication::lookupByTextChecksum(
            QString::fromUtf8("中文md5哈希值测试"), tmp.path());
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(OcrDeduplicationTest, Lookup_RegularFileAsIndexDir)
{
    // Create a regular file where an index dir is expected
    QString filePath = tmp.path() + "/not_a_dir.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("not an index");
    f.close();

    QString result = OcrDeduplication::lookupByTextChecksum(
            "abc123", filePath);
    // Will fail (not a directory), caught by try/catch -> empty
    EXPECT_TRUE(result.isEmpty());
}