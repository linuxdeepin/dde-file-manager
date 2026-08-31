// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagdbhandler.cpp
 * @brief Unit tests for TagDbHandler methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tagdbhandler.h"

#include <QTest>

using namespace tag;

class TagDbHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagDbHandler();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagDbHandler *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagDbHandlerTest, clearAllTrashTags)
{
    // Test bool getter: clearAllTrashTags()
    bool result = obj->clearAllTrashTags();
    EXPECT_FALSE(result);

}

TEST_F(TagDbHandlerTest, deleteFiles)
{
    // Test method: bool deleteFiles((const QStringList &urls))
    QStringList _arg0{};
    auto result = obj->deleteFiles(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagDbHandlerTest, deleteTags)
{
    // Test method: bool deleteTags((const QStringList &tags))
    QStringList _arg0{};
    auto result = obj->deleteTags(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagDbHandlerTest, getAllFileWithTags)
{
    // Test getter: QVariantHash getAllFileWithTags()
    auto result = obj->getAllFileWithTags();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagDbHandlerTest, getFilesByTag)
{
    // Test method: QVariantMap getFilesByTag((const QStringList &tags))
    QStringList _arg0{};
    auto result = obj->getFilesByTag(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagDbHandlerTest, getSameTagsOfDiffUrls)
{
    // Test method: QVariant getSameTagsOfDiffUrls((const QStringList &urlList))
    QStringList _arg0{};
    auto result = obj->getSameTagsOfDiffUrls(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TagDbHandlerTest, getTagsByUrls)
{
    // Test method: QVariantMap getTagsByUrls((const QStringList &urlList))
    QStringList _arg0{};
    auto result = obj->getTagsByUrls(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagDbHandlerTest, getTrashFileTags)
{
    // Test method: QVariantMap getTrashFileTags((const QStringList &queryParams))
    QStringList _arg0{};
    auto result = obj->getTrashFileTags(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagDbHandlerTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(TagDbHandlerTest, removeSpecifiedTagOfFile)
{
    // Test method: bool removeSpecifiedTagOfFile((const QString &url, const QVariant &val))
    QString _arg0{};
    QVariant _arg1{};
    auto result = obj->removeSpecifiedTagOfFile(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TagDbHandlerTest, removeTagsOfFiles)
{
    // Test method: bool removeTagsOfFiles((const QVariantMap &data))
    QVariantMap _arg0{};
    auto result = obj->removeTagsOfFiles(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagDbHandlerTest, removeTrashFileTags)
{
    // Test method: bool removeTrashFileTags((const QString &originalPath, qint64 inode))
    QString _arg0{};
    auto result = obj->removeTrashFileTags(_arg0, 0);
    EXPECT_FALSE(result);

}

TEST_F(TagDbHandlerTest, saveTrashFileTags)
{
    // Test method: bool saveTrashFileTags((const QString &originalPath, qint64 inode, const QStringList &tags))
    QString _arg0{};
    QStringList _arg2{};
    auto result = obj->saveTrashFileTags(_arg0, 0, _arg2);
    EXPECT_FALSE(result);

}
