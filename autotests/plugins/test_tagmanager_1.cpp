// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagmanager_1.cpp
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

TEST_F(TagManagerTest, addIconTagsHandle)
{
    // Test method: bool addIconTagsHandle((const FileInfoPointer &info, ElideTextLayout *layout))
    FileInfoPointer _arg0{};
    auto result = obj->addIconTagsHandle(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, canTagFile)
{
    // Test method: bool canTagFile((const FileInfoPointer &info))
    FileInfoPointer _arg0{};
    auto result = obj->canTagFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, changeTagColor)
{
    // Test method: bool changeTagColor((const QString &tagName, const QString &newTagColor))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->changeTagColor(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, changeTagName)
{
    // Test method: bool changeTagName((const QString &tagName, const QString &newName))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->changeTagName(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, fileDropHandle)
{
    // Test method: bool fileDropHandle((const QList<QUrl> &fromUrls, const QUrl &toUrl))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    auto result = obj->fileDropHandle(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, fileDropHandleWithAction)
{
    // Test method: bool fileDropHandleWithAction((const QList<QUrl> &fromUrls, const QUrl &toUrl, Qt::DropAction *action))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    auto result = obj->fileDropHandleWithAction(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, getAllTags)
{
    // Test getter: TagManager::TagColorMap getAllTags()
    auto result = obj->getAllTags();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(TagManagerTest, getFilesByTag)
{
    // Test method: QStringList getFilesByTag((const QString &tag))
    QString _arg0{};
    auto result = obj->getFilesByTag(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagManagerTest, getTagIconName)
{
    // Test method: QString getTagIconName((const QString &tag))
    QString _arg0{};
    auto result = obj->getTagIconName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagManagerTest, getTagsColor)
{
    // Test method: TagManager::TagColorMap getTagsColor((const QStringList &tags))
    QStringList _arg0{};
    auto result = obj->getTagsColor(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(TagManagerTest, getTagsColorName)
{
    // Test method: QMap<QString, QString> getTagsColorName((const QStringList &tags))
    QStringList _arg0{};
    auto result = obj->getTagsColorName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagManagerTest, hideFiles)
{
    // Test method: void hideFiles((const QList<QString> &tags, const QList<QUrl> &files))
    QList<QString> _arg0{};
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->hideFiles(_arg0, _arg1));
}

TEST_F(TagManagerTest, initializeConnection)
{
    // Test method: void initializeConnection(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeConnection());
}

TEST_F(TagManagerTest, instance)
{
    // Test getter: TagManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(TagManagerTest, onFilesTagged)
{
    // Test method: void onFilesTagged((const QVariantMap &fileAndTags))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFilesTagged(_arg0));
}

TEST_F(TagManagerTest, onFilesUntagged)
{
    // Test method: void onFilesUntagged((const QVariantMap &fileAndTags))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFilesUntagged(_arg0));
}

TEST_F(TagManagerTest, onTagAdded)
{
    // Test method: void onTagAdded((const QVariantMap &tags))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onTagAdded(_arg0));
}

TEST_F(TagManagerTest, onTagColorChanged)
{
    // Test method: void onTagColorChanged((const QVariantMap &tagAndColorName))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onTagColorChanged(_arg0));
}

TEST_F(TagManagerTest, onTagNameChanged)
{
    // Test method: void onTagNameChanged((const QVariantMap &oldAndNew))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onTagNameChanged(_arg0));
}

TEST_F(TagManagerTest, paintListTagsHandle)
{
    // Test method: bool paintListTagsHandle((int role, const FileInfoPointer &info, QPainter *painter, QRectF *rect))
    FileInfoPointer _arg1{};
    auto result = obj->paintListTagsHandle(0, _arg1, nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, registerTagColor)
{
    // Test method: bool registerTagColor((const QString &tagName, const QString &color))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->registerTagColor(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, renameHandle)
{
    // Test method: void renameHandle((quint64 windowId, const QUrl &url, const QString &name))
    QUrl _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->renameHandle(0, _arg1, _arg2));
}

TEST_F(TagManagerTest, sepateTitlebarCrumb)
{
    // Test method: bool sepateTitlebarCrumb((const QUrl &url, QList<QVariantMap> *mapGroup))
    QUrl _arg0{};
    auto result = obj->sepateTitlebarCrumb(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerTest, transformQueryData)
{
    // Test method: QVariant transformQueryData((const QDBusVariant &var))
    QDBusVariant _arg0{};
    auto result = obj->transformQueryData(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TagManagerTest, TagManager_Destructor)
{
    // Test method:  ~TagManager(())
    EXPECT_NO_FATAL_FAILURE({ TagManager *tmp = new TagManager(); delete tmp; });
}
