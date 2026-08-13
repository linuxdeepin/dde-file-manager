// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_processextractor_extra.cpp
 * @brief Additional tests for ProcessExtractor covering more internal paths.
 *        Tests various extract scenarios on same thread.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QThread>
#include <QEventLoop>
#include <QTimer>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/extractor/processextractor.h"
#include "services/textindex/extractor/indexextractor.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

class ProcessExtractorExtraTest : public testing::Test
{
protected:
    QTemporaryDir tmp;

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        createFile("normal.txt", "normal content");
        createFile("empty.txt", QString(""));
    }

    void createFile(const QString &name, const QString &content)
    {
        QFile f(tmp.path() + "/" + name);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write(content.toUtf8());
        f.close();
    }
};

TEST_F(ProcessExtractorExtraTest, Extract_SameThread_MultipleSequential)
{
    ProcessExtractor ext;
    for (int i = 0; i < 3; i++) {
        auto result = ext.extract(tmp.path() + "/normal.txt");
        EXPECT_FALSE(result.success);
    }
}

TEST_F(ProcessExtractorExtraTest, Extract_LargeFilePath)
{
    ProcessExtractor ext;
    QString longDir = tmp.path();
    for (int i = 0; i < 10; i++) {
        longDir += "/subdir_" + QString::number(i);
        QDir().mkpath(longDir);
    }
    QString longPath = longDir + "/file.txt";
    QFile f(longPath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();

    auto result = ext.extract(longPath);
    EXPECT_FALSE(result.success);
}

TEST_F(ProcessExtractorExtraTest, Extract_ZeroMaxBytes)
{
    ProcessExtractor ext;
    auto result = ext.extract(tmp.path() + "/normal.txt", 0);
    EXPECT_FALSE(result.success);
}

TEST_F(ProcessExtractorExtraTest, Extract_VeryLargeMaxBytes)
{
    ProcessExtractor ext;
    auto result = ext.extract(tmp.path() + "/normal.txt", SIZE_MAX);
    EXPECT_FALSE(result.success);
}

TEST_F(ProcessExtractorExtraTest, Extract_PathWithSpaces)
{
    QString dirWithSpaces = tmp.path() + "/dir with spaces";
    QDir().mkpath(dirWithSpaces);
    QString filePath = dirWithSpaces + "/file with spaces.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("content");
    f.close();

    ProcessExtractor ext;
    auto result = ext.extract(filePath);
    EXPECT_FALSE(result.success);
}

TEST_F(ProcessExtractorExtraTest, Extract_PathWithSpecialChars)
{
    QString specialDir = tmp.path() + "/dir_特殊_🚀";
    QDir().mkpath(specialDir);
    QString filePath = specialDir + "/文件.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("unicode");
    f.close();

    ProcessExtractor ext;
    auto result = ext.extract(filePath);
    EXPECT_FALSE(result.success);
}

TEST_F(ProcessExtractorExtraTest, MultipleExtractors_InScope)
{
    ProcessExtractor ext1;
    ProcessExtractor ext2;
    ProcessExtractor ext3;

    auto r1 = ext1.extract(tmp.path() + "/normal.txt");
    auto r2 = ext2.extract(tmp.path() + "/normal.txt");
    auto r3 = ext3.extract(tmp.path() + "/normal.txt");

    EXPECT_FALSE(r1.success);
    EXPECT_FALSE(r2.success);
    EXPECT_FALSE(r3.success);
}

TEST_F(ProcessExtractorExtraTest, Extract_EmptyFile)
{
    ProcessExtractor ext;
    auto result = ext.extract(tmp.path() + "/empty.txt");
    EXPECT_FALSE(result.success);
}

TEST_F(ProcessExtractorExtraTest, Extract_NonExistentFile)
{
    ProcessExtractor ext;
    auto result = ext.extract("/nonexistent/file/that/does/not/exist.txt");
    EXPECT_FALSE(result.success);
}

TEST_F(ProcessExtractorExtraTest, Extract_WithMaxBytes_1024)
{
    ProcessExtractor ext;
    auto result = ext.extract(tmp.path() + "/normal.txt", 1024);
    EXPECT_FALSE(result.success);
}

TEST_F(ProcessExtractorExtraTest, Extract_WithMaxBytes_1)
{
    ProcessExtractor ext;
    auto result = ext.extract(tmp.path() + "/normal.txt", 1);
    EXPECT_FALSE(result.success);
}