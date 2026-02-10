// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-tag/utils/filetagcache.h"
#include "plugins/common/dfmplugin-tag/utils/private/filetagcache_p.h"

#include <gtest/gtest.h>

#include <QColor>
#include <QVariantMap>

using namespace dfmplugin_tag;

class UT_FileTagCacheController : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Stub QThread operations to avoid thread issues in tests
        stub.set_lamda(ADDR(QThread, start), [](QThread*, QThread::Priority) {
            __DBG_STUB_INVOKE__
            // Prevent thread from starting
        });

        stub.set_lamda(ADDR(QThread, quit), [](QThread*) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(static_cast<bool (QThread::*)(QDeadlineTimer)>(&QThread::wait), [] {
            __DBG_STUB_INVOKE__
            return true;
        });

        controller = &FileTagCacheController::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    FileTagCacheController *controller = nullptr;
};

TEST_F(UT_FileTagCacheController, instance)
{
    // Test singleton instance
    FileTagCacheController *instance1 = &FileTagCacheController::instance();
    FileTagCacheController *instance2 = &FileTagCacheController::instance();

    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);
}

TEST_F(UT_FileTagCacheController, getTagsByFile_Empty)
{
    // Test getting tags for empty file path
    stub.set_lamda(&FileTagCache::getTagsByFiles, [](const FileTagCache*, const QStringList&) -> QStringList {
        __DBG_STUB_INVOKE__
        return QStringList();
    });

    QStringList tags = controller->getTagsByFile("");

    EXPECT_TRUE(tags.isEmpty());
}

TEST_F(UT_FileTagCacheController, getTagsByFile_ValidFile)
{
    // Test getting tags for valid file
    stub.set_lamda(&FileTagCache::getTagsByFiles, [](const FileTagCache*, const QStringList&) -> QStringList {
        __DBG_STUB_INVOKE__
        QStringList tags;
        tags << "Tag1" << "Tag2";
        return tags;
    });

    QStringList tags = controller->getTagsByFile("/home/user/test.txt");

    EXPECT_EQ(tags.size(), 2);
}

TEST_F(UT_FileTagCacheController, getTagsByFiles_Empty)
{
    // Test getting tags for empty file list
    QStringList paths;

    stub.set_lamda(&FileTagCache::getTagsByFiles, [](const FileTagCache*, const QStringList&) -> QStringList {
        __DBG_STUB_INVOKE__
        return QStringList();
    });

    QStringList tags = controller->getTagsByFiles(paths);

    EXPECT_TRUE(tags.isEmpty());
}

TEST_F(UT_FileTagCacheController, getTagsByFiles_MultipleFiles)
{
    // Test getting tags for multiple files
    QStringList paths;
    paths << "/home/user/file1.txt" << "/home/user/file2.txt";

    stub.set_lamda(&FileTagCache::getTagsByFiles, [](const FileTagCache*, const QStringList&) -> QStringList {
        __DBG_STUB_INVOKE__
        QStringList tags;
        tags << "Tag1" << "Tag2";
        return tags;
    });

    QStringList tags = controller->getTagsByFiles(paths);

    EXPECT_EQ(tags.size(), 2);
}

TEST_F(UT_FileTagCacheController, getCacheTagsColor_Empty)
{
    // Test getting colors for empty tag list
    QStringList tags;

    stub.set_lamda(&FileTagCache::getTagsColor, [](const FileTagCache*, const QStringList&) -> QMap<QString, QColor> {
        __DBG_STUB_INVOKE__
        return QMap<QString, QColor>();
    });

    QMap<QString, QColor> colors = controller->getCacheTagsColor(tags);

    EXPECT_TRUE(colors.isEmpty());
}

TEST_F(UT_FileTagCacheController, getCacheTagsColor_ValidTags)
{
    // Test getting colors for valid tags
    QStringList tags;
    tags << "Tag1" << "Tag2";

    stub.set_lamda(&FileTagCache::getTagsColor, [](const FileTagCache*, const QStringList&) -> QMap<QString, QColor> {
        __DBG_STUB_INVOKE__
        QMap<QString, QColor> colors;
        colors["Tag1"] = QColor("#ffa503");
        colors["Tag2"] = QColor("#ff1c49");
        return colors;
    });

    QMap<QString, QColor> colors = controller->getCacheTagsColor(tags);

    EXPECT_EQ(colors.size(), 2);
    EXPECT_TRUE(colors.contains("Tag1"));
    EXPECT_TRUE(colors.contains("Tag2"));
}

TEST_F(UT_FileTagCacheController, findChildren_Empty)
{
    // Test finding children for empty path
    stub.set_lamda(&FileTagCache::findChildren, [](const FileTagCache*, const QString&) -> QHash<QString, QStringList> {
        __DBG_STUB_INVOKE__
        return QHash<QString, QStringList>();
    });

    QHash<QString, QStringList> children = controller->findChildren("");

    EXPECT_TRUE(children.isEmpty());
}

TEST_F(UT_FileTagCacheController, findChildren_ValidPath)
{
    // Test finding children for valid path
    stub.set_lamda(&FileTagCache::findChildren, [](const FileTagCache*, const QString&) -> QHash<QString, QStringList> {
        __DBG_STUB_INVOKE__
        QHash<QString, QStringList> children;
        QStringList tags;
        tags << "Tag1";
        children["/home/user/subdir/file.txt"] = tags;
        return children;
    });

    QHash<QString, QStringList> children = controller->findChildren("/home/user");

    EXPECT_FALSE(children.isEmpty());
}

TEST_F(UT_FileTagCacheController, getTagsByFiles_SingleFile)
{
    // Test getting tags for a single file (common tags scenario)
    QStringList paths;
    paths << "/home/user/test.txt";

    stub.set_lamda(&FileTagCache::getTagsByFiles, [](const FileTagCache*, const QStringList&) -> QStringList {
        __DBG_STUB_INVOKE__
        QStringList tags;
        tags << "Important" << "Work";
        return tags;
    });

    QStringList tags = controller->getTagsByFiles(paths);

    EXPECT_FALSE(tags.isEmpty());
}

TEST_F(UT_FileTagCacheController, getCacheTagsColor_MixedColors)
{
    // Test getting colors with different color formats
    QStringList tags;
    tags << "Orange" << "Red" << "Blue";

    stub.set_lamda(&FileTagCache::getTagsColor, [](const FileTagCache*, const QStringList&) -> QMap<QString, QColor> {
        __DBG_STUB_INVOKE__
        QMap<QString, QColor> colors;
        colors["Orange"] = QColor("#ffa503");
        colors["Red"] = QColor("#ff1c49");
        colors["Blue"] = QColor("#0000ff");
        return colors;
    });

    QMap<QString, QColor> colors = controller->getCacheTagsColor(tags);

    EXPECT_EQ(colors.size(), 3);
    EXPECT_TRUE(colors["Orange"].isValid());
    EXPECT_TRUE(colors["Red"].isValid());
    EXPECT_TRUE(colors["Blue"].isValid());
}

TEST_F(UT_FileTagCacheController, findChildren_NestedPaths)
{
    // Test finding children in nested directory structure
    stub.set_lamda(&FileTagCache::findChildren, [](const FileTagCache*, const QString&) -> QHash<QString, QStringList> {
        __DBG_STUB_INVOKE__
        QHash<QString, QStringList> children;
        children["/home/user/dir1/file1.txt"] = QStringList() << "Tag1";
        children["/home/user/dir2/file2.txt"] = QStringList() << "Tag2";
        children["/home/user/dir1/subdir/file3.txt"] = QStringList() << "Tag1" << "Tag2";
        return children;
    });

    QHash<QString, QStringList> children = controller->findChildren("/home/user");

    EXPECT_EQ(children.size(), 3);
}
