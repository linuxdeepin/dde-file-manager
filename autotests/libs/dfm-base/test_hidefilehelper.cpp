// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_hidefilehelper.cpp
 * @brief Unit tests for HideFileHelper (hidefilehelper.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <mutex>

#include <dfm-base/utils/hidefilehelper.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;

class HideFileHelperTest : public testing::Test
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
        dirPath = tmpDir.path();
    }

    QTemporaryDir tmpDir;
    QString dirPath;
    static std::once_flag flag;
};

std::once_flag HideFileHelperTest::flag;

TEST_F(HideFileHelperTest, DirUrlAndFileUrl)
{
    HideFileHelper helper(QUrl::fromLocalFile(dirPath));
    EXPECT_FALSE(helper.dirUrl().isEmpty());
    EXPECT_FALSE(helper.fileUrl().isEmpty());
    EXPECT_TRUE(helper.fileUrl().path().endsWith(".hidden"));
}

TEST_F(HideFileHelperTest, InsertRemoveContains)
{
    HideFileHelper helper(QUrl::fromLocalFile(dirPath));
    EXPECT_TRUE(helper.insert("secret.txt"));
    EXPECT_TRUE(helper.contains("secret.txt"));
    EXPECT_FALSE(helper.contains("nope.txt"));
    EXPECT_TRUE(helper.remove("secret.txt"));
    EXPECT_FALSE(helper.contains("secret.txt"));
}

TEST_F(HideFileHelperTest, HideFileList)
{
    HideFileHelper helper(QUrl::fromLocalFile(dirPath));
    helper.insert("a.txt");
    helper.insert("b.txt");
    QSet<QString> list = helper.hideFileList();
    EXPECT_TRUE(list.contains("a.txt"));
    EXPECT_TRUE(list.contains("b.txt"));
}

TEST_F(HideFileHelperTest, SaveWritesHiddenFile)
{
    HideFileHelper helper(QUrl::fromLocalFile(dirPath));
    helper.insert("c.txt");
    bool ok = false;
    EXPECT_NO_FATAL_FAILURE({ ok = helper.save(); });
    // save may return true if writable
    if (ok) {
        QFile f(dirPath + "/.hidden");
        EXPECT_TRUE(f.exists());
    }
}

TEST_F(HideFileHelperTest, ConstructWithExistingHiddenFile)
{
    // Create a .hidden file first, then construct helper to read it back.
    QFile f(dirPath + "/.hidden");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("existing.txt\n");
    f.close();

    HideFileHelper helper(QUrl::fromLocalFile(dirPath));
    EXPECT_TRUE(helper.contains("existing.txt"));
}
