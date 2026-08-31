// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagdbhandler_1.cpp
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

TEST_F(TagDbHandlerTest, TagDbHandler)
{
    // Test constructor: TagDbHandler((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagDbHandlerTest, addTagProperty)
{
    // Test method: bool addTagProperty((const QVariantMap &data))
    QVariantMap _arg0{};
    auto result = obj->addTagProperty(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagDbHandlerTest, changeFilePath)
{
    // Test method: bool changeFilePath((const QString &oldPath, const QString &newPath))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->changeFilePath(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TagDbHandlerTest, changeFilePaths)
{
    // Test method: bool changeFilePaths((const QVariantMap &data))
    QVariantMap _arg0{};
    auto result = obj->changeFilePaths(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagDbHandlerTest, changeTagColor)
{
    // Test method: bool changeTagColor((const QString &tagName, const QString &newTagColor))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->changeTagColor(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TagDbHandlerTest, changeTagColors)
{
    // Test method: bool changeTagColors((const QVariantMap &data))
    QVariantMap _arg0{};
    auto result = obj->changeTagColors(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagDbHandlerTest, changeTagNameWithFile)
{
    // Test method: bool changeTagNameWithFile((const QString &tagName, const QString &newName))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->changeTagNameWithFile(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TagDbHandlerTest, changeTagNamesWithFiles)
{
    // Test method: bool changeTagNamesWithFiles((const QVariantMap &data))
    QVariantMap _arg0{};
    auto result = obj->changeTagNamesWithFiles(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagDbHandlerTest, checkTag)
{
    // Test method: bool checkTag((const QString &tag))
    QString _arg0{};
    auto result = obj->checkTag(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagDbHandlerTest, createTable)
{
    // Test method: bool createTable((const QString &tableName))
    QString _arg0{};
    auto result = obj->createTable(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagDbHandlerTest, getAllTags)
{
    // Test getter: QVariantMap getAllTags()
    auto result = obj->getAllTags();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagDbHandlerTest, getAllTrashFileTags)
{
    // Test getter: QVariantHash getAllTrashFileTags()
    auto result = obj->getAllTrashFileTags();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagDbHandlerTest, getTagsColor)
{
    // Test method: QVariantMap getTagsColor((const QStringList &tags))
    QStringList _arg0{};
    auto result = obj->getTagsColor(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagDbHandlerTest, hasTrashFileTags)
{
    // Test method: bool hasTrashFileTags((const QString &originalPath, qint64 inode))
    QString _arg0{};
    auto result = obj->hasTrashFileTags(_arg0, 0);
    EXPECT_FALSE(result);

}

TEST_F(TagDbHandlerTest, insertTagProperty)
{
    // Test method: bool insertTagProperty((const QString &name, const QVariant &value))
    QString _arg0{};
    QVariant _arg1{};
    auto result = obj->insertTagProperty(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TagDbHandlerTest, instance)
{
    // Test getter: TagDbHandler instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(TagDbHandlerTest, lastError)
{
    // Test getter: QString lastError()
    auto result = obj->lastError();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagDbHandlerTest, tagFile)
{
    // Test method: bool tagFile((const QString &file, const QVariant &tags))
    QString _arg0{};
    QVariant _arg1{};
    auto result = obj->tagFile(_arg0, _arg1);
    EXPECT_FALSE(result);

}
