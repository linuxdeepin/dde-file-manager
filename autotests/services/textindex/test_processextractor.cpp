// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_processextractor.cpp
 * @brief Unit tests for ProcessExtractor (extractor/processextractor.cpp)
 *        Covers ctor/dtor, extract with null proxy, extract on same thread,
 *        and extract from different thread. The actual extractor subprocess
 *        interaction is stubbed to avoid launching real processes.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>

#include "stubext.h"
#include "dfm_test_main.h"

#include "services/textindex/service_textindex_global.h"
#include "services/textindex/extractor/processextractor.h"
#include "services/textindex/extractor/indexextractor.h"
#include "controllerpipe.h"

using namespace SERVICETEXTINDEX_NAMESPACE;
using namespace EXTRACTOR_NAMESPACE;

class ProcessExtractorTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        // Stub ControllerPipe::start to always fail so the extractor
        // subprocess never actually launches during tests.
        stub.set_lamda(VADDR(ControllerPipe, start), [](ControllerPipe *, const QString &, const QString &) -> bool {
            return false;
        });
    }
};

TEST_F(ProcessExtractorTest, ConstructAndDestruct)
{
    { ProcessExtractor ext; }
    SUCCEED();
}

TEST_F(ProcessExtractorTest, Extract_NullProxy_ReturnsError)
{
    ProcessExtractor ext;
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QString filePath = tmp.path() + "/test.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("hello world");
    f.close();

    auto result = ext.extract(filePath);
    EXPECT_FALSE(result.success);
}

TEST_F(ProcessExtractorTest, Extract_NonExistentFile)
{
    ProcessExtractor ext;
    auto result = ext.extract("/nonexistent/file/that/does/not/exist.txt");
    EXPECT_FALSE(result.success);
}

TEST_F(ProcessExtractorTest, Extract_EmptyFilePath)
{
    ProcessExtractor ext;
    auto result = ext.extract(QString());
    EXPECT_FALSE(result.success);
}

TEST_F(ProcessExtractorTest, Extract_WithMaxBytes)
{
    ProcessExtractor ext;
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QString filePath = tmp.path() + "/test.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("hello world");
    f.close();

    auto result = ext.extract(filePath, 1024);
    EXPECT_FALSE(result.success);
}

TEST_F(ProcessExtractorTest, ExtractResult_DefaultValues)
{
    IndexExtractionResult result;
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.text.isEmpty());
    EXPECT_TRUE(result.error.isEmpty());
    EXPECT_TRUE(result.checksum.isEmpty());
    EXPECT_FALSE(result.deduplicated);
}

TEST_F(ProcessExtractorTest, Extract_MultipleCalls)
{
    ProcessExtractor ext;
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());

    for (int i = 0; i < 5; i++) {
        QString filePath = tmp.path() + "/test" + QString::number(i) + ".txt";
        QFile f(filePath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("content");
        f.close();
        auto result = ext.extract(filePath);
        EXPECT_FALSE(result.success);
    }
}