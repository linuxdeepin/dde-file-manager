// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_contentdeduplication.cpp
 * @brief Unit tests for ContentDeduplication (extractor/contentdeduplication.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/extractor/contentdeduplication.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

class ContentDeduplicationTest : public testing::Test
{
protected:
    QTemporaryDir tmp;

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
    }
};

TEST_F(ContentDeduplicationTest, Lookup_EmptyChecksum)
{
    QString result = ContentDeduplication::lookupByTextChecksum(QString(), tmp.path());
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(ContentDeduplicationTest, Lookup_EmptyIndexDir)
{
    QString result = ContentDeduplication::lookupByTextChecksum("abc123", QString());
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(ContentDeduplicationTest, Lookup_BothEmpty)
{
    QString result = ContentDeduplication::lookupByTextChecksum(QString(), QString());
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(ContentDeduplicationTest, Lookup_NonExistentIndexDir)
{
    QString result = ContentDeduplication::lookupByTextChecksum(
            "d41d8cd98f00b204e9800998ecf8427e", "/nonexistent/path/to/index");
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(ContentDeduplicationTest, Lookup_EmptyDirAsIndex)
{
    QString result = ContentDeduplication::lookupByTextChecksum(
            "d41d8cd98f00b204e9800998ecf8427e", tmp.path());
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(ContentDeduplicationTest, Lookup_LongChecksum)
{
    QString longChecksum(1000, 'a');
    QString result = ContentDeduplication::lookupByTextChecksum(longChecksum, tmp.path());
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(ContentDeduplicationTest, Lookup_FileAsIndexDir)
{
    QString filePath = tmp.path() + "/not_a_dir.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("not an index");
    f.close();

    QString result = ContentDeduplication::lookupByTextChecksum("abc123", filePath);
    EXPECT_TRUE(result.isEmpty());
}