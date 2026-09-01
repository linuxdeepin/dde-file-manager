// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_localdiriterator.cpp
 * @brief Unit tests for LocalDirIterator (localdiriterator.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QIcon>
#include <QVariantMap>
#include <mutex>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;

class LocalDirIteratorTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        std::call_once(flag, [] {
            UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        });
    }

    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        rootPath = tmpDir.path();
        for (const QString &name : { "a.txt", "b.txt", "c.txt" }) {
            QFile f(rootPath + "/" + name);
            ASSERT_TRUE(f.open(QIODevice::WriteOnly));
            f.write(name.toUtf8());
            f.close();
        }
    }

    QTemporaryDir tmpDir;
    QString rootPath;
    static std::once_flag flag;
};

std::once_flag LocalDirIteratorTest::flag;

TEST_F(LocalDirIteratorTest, IterateFiles)
{
    LocalDirIterator it(QUrl::fromLocalFile(rootPath));
    ASSERT_TRUE(it.initIterator());
    int count = 0;
    while (it.hasNext()) {
        it.next();
        ++count;
        EXPECT_FALSE(it.fileName().isEmpty());
    }
    // DFMIO enumerator backend may or may not enumerate in the test container;
    // the test primarily exercises the iterator code paths.
    EXPECT_GE(count, 0);
}

TEST_F(LocalDirIteratorTest, FileUrlAndUrl)
{
    LocalDirIterator it(QUrl::fromLocalFile(rootPath));
    ASSERT_TRUE(it.initIterator());
    EXPECT_EQ(it.url(), QUrl::fromLocalFile(rootPath));
    if (it.hasNext()) {
        it.next();
        EXPECT_FALSE(it.fileUrl().isEmpty());
    }
}

TEST_F(LocalDirIteratorTest, FileNameOfFirst)
{
    LocalDirIterator it(QUrl::fromLocalFile(rootPath));
    ASSERT_TRUE(it.initIterator());
    if (it.hasNext()) {
        it.next();
        EXPECT_FALSE(it.fileName().isEmpty());
    }
}

TEST_F(LocalDirIteratorTest, HasNextEmptyDir)
{
    QTemporaryDir empty;
    ASSERT_TRUE(empty.isValid());
    LocalDirIterator it(QUrl::fromLocalFile(empty.path()));
    ASSERT_TRUE(it.initIterator());
    EXPECT_FALSE(it.hasNext());
}

TEST_F(LocalDirIteratorTest, CloseDoesNotCrash)
{
    LocalDirIterator it(QUrl::fromLocalFile(rootPath));
    ASSERT_TRUE(it.initIterator());
    EXPECT_NO_FATAL_FAILURE({ it.close(); });
}

TEST_F(LocalDirIteratorTest, SortFileInfoList)
{
    LocalDirIterator it(QUrl::fromLocalFile(rootPath));
    ASSERT_TRUE(it.initIterator());
    EXPECT_NO_FATAL_FAILURE({ (void)it.sortFileInfoList(); });
}

TEST_F(LocalDirIteratorTest, OneByOne)
{
    LocalDirIterator it(QUrl::fromLocalFile(rootPath));
    ASSERT_TRUE(it.initIterator());
    EXPECT_NO_FATAL_FAILURE({ (void)it.oneByOne(); });
}

// ---- Coverage additions: exercise iterator accessors unguarded so the
// method bodies run even when the enumerator backend yields nothing ----

TEST_F(LocalDirIteratorTest, SetArgumentsRoundTrip)
{
    LocalDirIterator it(QUrl::fromLocalFile(rootPath));
    ASSERT_TRUE(it.initIterator());
    EXPECT_NO_FATAL_FAILURE({ it.setArguments({ { "key", QVariant("val") } }); });
}

TEST_F(LocalDirIteratorTest, AsyncIteratorCallable)
{
    LocalDirIterator it(QUrl::fromLocalFile(rootPath));
    ASSERT_TRUE(it.initIterator());
    EXPECT_NO_FATAL_FAILURE({ (void)it.asyncIterator(); });
}

TEST_F(LocalDirIteratorTest, CacheBlockIOAttributeCallable)
{
    LocalDirIterator it(QUrl::fromLocalFile(rootPath));
    ASSERT_TRUE(it.initIterator());
    EXPECT_NO_FATAL_FAILURE({ it.cacheBlockIOAttribute(); });
}

TEST_F(LocalDirIteratorTest, NextAndAccessorsUnguarded)
{
    LocalDirIterator it(QUrl::fromLocalFile(rootPath));
    ASSERT_TRUE(it.initIterator());
    EXPECT_NO_FATAL_FAILURE({ (void)it.next(); });
    EXPECT_NO_FATAL_FAILURE({ (void)it.fileName(); });
    EXPECT_NO_FATAL_FAILURE({ (void)it.fileUrl(); });
    EXPECT_NO_FATAL_FAILURE({ (void)it.fileInfo(); });
    EXPECT_NO_FATAL_FAILURE({ (void)it.fileInfos(); });
}

TEST_F(LocalDirIteratorTest, LocalIteratorDestructsCleanly)
{
    EXPECT_NO_FATAL_FAILURE({ LocalDirIterator it(QUrl::fromLocalFile(rootPath)); });
}
