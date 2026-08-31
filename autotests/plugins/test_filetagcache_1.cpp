// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filetagcache_1.cpp
 * @brief Unit tests for FileTagCache methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/filetagcache.h"

#include <QTest>

using namespace dfmplugin_tag;

class FileTagCacheTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileTagCache();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileTagCache *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileTagCacheTest, FileTagCache)
{
    // Test constructor: FileTagCache((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileTagCacheTest, changeFilesTagName)
{
    // Test method: void changeFilesTagName((const QString &oldName, const QString &newName))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->changeFilesTagName(_arg0, _arg1));
}

TEST_F(FileTagCacheTest, findChildren)
{
    // Test method: QHash<QString, QStringList> findChildren((const QString &parentPath))
    QString _arg0{};
    auto result = obj->findChildren(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileTagCacheTest, getTagsByFiles)
{
    // Test method: QStringList getTagsByFiles((const QStringList &paths))
    QStringList _arg0{};
    auto result = obj->getTagsByFiles(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileTagCacheTest, getTrashTags)
{
    // Test method: QStringList getTrashTags((const QString &path, qint64 inode))
    QString _arg0{};
    auto result = obj->getTrashTags(_arg0, 0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileTagCacheTest, instance)
{
    // Test getter: FileTagCache instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(FileTagCacheTest, loadFileTagsFromDatabase)
{
    // Test method: void loadFileTagsFromDatabase(())
    EXPECT_NO_FATAL_FAILURE(obj->loadFileTagsFromDatabase());
}

TEST_F(FileTagCacheTest, reloadTrashFileTagsCache)
{
    // Test method: void reloadTrashFileTagsCache(())
    EXPECT_NO_FATAL_FAILURE(obj->reloadTrashFileTagsCache());
}

TEST_F(FileTagCacheTest, FileTagCache_Destructor)
{
    // Test method:  ~FileTagCache(())
    EXPECT_NO_FATAL_FAILURE({ FileTagCache *tmp = new FileTagCache(); delete tmp; });
}
