// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagmanager.cpp
 * @brief Unit tests for TagManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/tagmanager.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagManagerTest, addTagsForFiles)
{
    // Test method: bool addTagsForFiles((const QList<QString> &tags, const QList<QUrl> &files))
    QList<QString> _arg0{};
    QList<QUrl> _arg1{};
    auto result = obj->addTagsForFiles(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, assignColorToTags)
{
    // Test method: QMap<QString, QColor> assignColorToTags((const QStringList &tagList))
    QStringList _arg0{};
    auto result = obj->assignColorToTags(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagManagerTest, clearAllTrashTags)
{
    // Test bool getter: clearAllTrashTags()
    bool result = obj->clearAllTrashTags();
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, contenxtMenuHandle)
{
    // Test method: void contenxtMenuHandle((quint64 windowId, const QUrl &url, const QPoint &globalPos))
    QUrl _arg1{};
    QPoint _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->contenxtMenuHandle(0, _arg1, _arg2));
}

TEST_F(TagManagerTest, deleteTagData)
{
    // Test method: bool deleteTagData((const QStringList &data, const DeleteOpts &type))
    QStringList _arg0{};
    DeleteOpts _arg1{};
    auto result = obj->deleteTagData(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, deleteTags)
{
    // Test method: void deleteTags((const QStringList &tags))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->deleteTags(_arg0));
}

TEST_F(TagManagerTest, findChildren)
{
    // Test method: QHash<QString, QStringList> findChildren((const QString &parentPath))
    QString _arg0{};
    auto result = obj->findChildren(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagManagerTest, getTagsByUrls)
{
    // Test method: QStringList getTagsByUrls((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->getTagsByUrls(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagManagerTest, getTrashFileTags)
{
    // Test method: QStringList getTrashFileTags((const QString &originalPath, qint64 fileInode))
    QString _arg0{};
    auto result = obj->getTrashFileTags(_arg0, 0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagManagerTest, localFileCanTagFilter)
{
    // Test method: bool localFileCanTagFilter((const FileInfoPointer &info))
    FileInfoPointer _arg0{};
    auto result = obj->localFileCanTagFilter(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, onTagDeleted)
{
    // Test method: void onTagDeleted((const QStringList &tags))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onTagDeleted(_arg0));
}

TEST_F(TagManagerTest, pasteHandle)
{
    // Test method: bool pasteHandle((quint64 winId, const QList<QUrl> &fromUrls, const QUrl &to))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    auto result = obj->pasteHandle(0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, removeChildren)
{
    // Test method: bool removeChildren((const QString &parentPath))
    QString _arg0{};
    auto result = obj->removeChildren(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, removeTagsOfFiles)
{
    // Test method: bool removeTagsOfFiles((const QList<QString> &tags, const QList<QUrl> &files))
    QList<QString> _arg0{};
    QList<QUrl> _arg1{};
    auto result = obj->removeTagsOfFiles(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, removeTrashFileTags)
{
    // Test method: bool removeTrashFileTags((const QString &originalPath, qint64 fileInode))
    QString _arg0{};
    auto result = obj->removeTrashFileTags(_arg0, 0);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(TagManagerTest, saveTrashFileTags)
{
    // Test method: bool saveTrashFileTags((const QString &originalPath, qint64 fileInode, const QStringList &tagNames))
    QString _arg0{};
    QStringList _arg2{};
    auto result = obj->saveTrashFileTags(_arg0, 0, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, scheme)
{
    // Test getter: QString scheme()
    auto result = obj->scheme();
    EXPECT_TRUE(result.isEmpty());

}
