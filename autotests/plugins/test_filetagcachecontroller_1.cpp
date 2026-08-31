// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filetagcachecontroller_1.cpp
 * @brief Unit tests for FileTagCacheController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/filetagcache.h"

#include <QTest>

using namespace dfmplugin_tag;

class FileTagCacheControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileTagCacheController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileTagCacheController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileTagCacheControllerTest, FileTagCacheController)
{
    // Test constructor: FileTagCacheController((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileTagCacheControllerTest, getCacheTagsColor)
{
    // Test method: QMap<QString, QColor> getCacheTagsColor((const QStringList &tags))
    QStringList _arg0{};
    auto result = obj->getCacheTagsColor(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileTagCacheControllerTest, getTagsByFile)
{
    // Test method: QStringList getTagsByFile((const QString &path))
    QString _arg0{};
    auto result = obj->getTagsByFile(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileTagCacheControllerTest, getTrashFileTags)
{
    // Test method: QStringList getTrashFileTags((const QString &path, qint64 inode))
    QString _arg0{};
    auto result = obj->getTrashFileTags(_arg0, 0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileTagCacheControllerTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(FileTagCacheControllerTest, instance)
{
    // Test getter: FileTagCacheController instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(FileTagCacheControllerTest, FileTagCacheController_Destructor)
{
    // Test method:  ~FileTagCacheController(())
    EXPECT_NO_FATAL_FAILURE({ FileTagCacheController *tmp = new FileTagCacheController(); delete tmp; });
}
