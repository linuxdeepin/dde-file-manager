// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagproxyhandle_1.cpp
 * @brief Unit tests for TagProxyHandle methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "data/tagproxyhandle.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagProxyHandleTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagProxyHandle();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagProxyHandle *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagProxyHandleTest, TagProxyHandle)
{
    // Test constructor: TagProxyHandle((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagProxyHandleTest, addTags)
{
    // Test method: bool addTags((const QVariantMap &value))
    QVariantMap _arg0{};
    auto result = obj->addTags(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagProxyHandleTest, addTagsForFiles)
{
    // Test method: bool addTagsForFiles((const QVariantMap &value))
    QVariantMap _arg0{};
    auto result = obj->addTagsForFiles(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagProxyHandleTest, changeFilePaths)
{
    // Test method: bool changeFilePaths((const QVariantMap &value))
    QVariantMap _arg0{};
    auto result = obj->changeFilePaths(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagProxyHandleTest, changeTagNamesWithFiles)
{
    // Test method: bool changeTagNamesWithFiles((const QVariantMap &value))
    QVariantMap _arg0{};
    auto result = obj->changeTagNamesWithFiles(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagProxyHandleTest, changeTagsColor)
{
    // Test method: bool changeTagsColor((const QVariantMap &value))
    QVariantMap _arg0{};
    auto result = obj->changeTagsColor(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagProxyHandleTest, connectToService)
{
    // Test bool getter: connectToService()
    bool result = obj->connectToService();
    EXPECT_FALSE(result);

}

TEST_F(TagProxyHandleTest, getAllFileWithTags)
{
    // Test getter: QVariantHash getAllFileWithTags()
    auto result = obj->getAllFileWithTags();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagProxyHandleTest, getAllTags)
{
    // Test getter: QVariantMap getAllTags()
    auto result = obj->getAllTags();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagProxyHandleTest, getAllTrashFileTags)
{
    // Test getter: QVariantHash getAllTrashFileTags()
    auto result = obj->getAllTrashFileTags();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagProxyHandleTest, getFilesThroughTag)
{
    // Test method: QVariantMap getFilesThroughTag((const QStringList &value))
    QStringList _arg0{};
    auto result = obj->getFilesThroughTag(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagProxyHandleTest, getSameTagsOfDiffFiles)
{
    // Test method: QVariant getSameTagsOfDiffFiles((const QStringList &value))
    QStringList _arg0{};
    auto result = obj->getSameTagsOfDiffFiles(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TagProxyHandleTest, getTagsColor)
{
    // Test method: QVariantMap getTagsColor((const QStringList &value))
    QStringList _arg0{};
    auto result = obj->getTagsColor(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagProxyHandleTest, getTagsThroughFile)
{
    // Test method: QVariantMap getTagsThroughFile((const QStringList &value))
    QStringList _arg0{};
    auto result = obj->getTagsThroughFile(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagProxyHandleTest, getTrashFileTags)
{
    // Test method: QStringList getTrashFileTags((const QString &originalPath, qint64 inode))
    QString _arg0{};
    auto result = obj->getTrashFileTags(_arg0, 0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagProxyHandleTest, TagProxyHandle_Destructor)
{
    // Test method:  ~TagProxyHandle(())
    EXPECT_NO_FATAL_FAILURE({ TagProxyHandle *tmp = new TagProxyHandle(); delete tmp; });
}
