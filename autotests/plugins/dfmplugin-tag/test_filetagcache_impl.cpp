// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-tag/utils/filetagcache.h"
#include "plugins/common/dfmplugin-tag/utils/private/filetagcache_p.h"
#include "plugins/common/dfmplugin-tag/data/tagproxyhandle.h"

#include <gtest/gtest.h>

#include <QColor>
#include <QVariantMap>
#include <QThread>

using namespace dfmplugin_tag;

class FileTagCacheImpl : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Prevent the FileTagCacheController update thread from actually running;
        // otherwise the unit-test process may hang or crash because the worker
        // lives in a thread without an event loop.
        stub.set_lamda(ADDR(QThread, start), [](QThread *, QThread::Priority) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(ADDR(QThread, quit), [](QThread *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(static_cast<bool (QThread::*)(QDeadlineTimer)>(&QThread::wait), [] {
            __DBG_STUB_INVOKE__
            return true;
        });

        // Make sure the controller singleton is materialized under the stubs above.
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

TEST_F(FileTagCacheImpl, controller_singleton)
{
    EXPECT_NE(controller, nullptr);
    EXPECT_EQ(controller, &FileTagCacheController::instance());
}

TEST_F(FileTagCacheImpl, load_file_tags_from_database)
{
    stub.set_lamda(&TagProxyHandle::getAllFileWithTags, [](TagProxyHandle *) -> QVariantHash {
        __DBG_STUB_INVOKE__
        QVariantHash hash;
        hash.insert("/tmp/ftc_impl/file1.txt", QStringList() << "ImplTag1");
        return hash;
    });
    stub.set_lamda(&TagProxyHandle::getAllTags, [](TagProxyHandle *) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map.insert("ImplTag1", "#ff0000");
        return map;
    });
    stub.set_lamda(&TagProxyHandle::getAllTrashFileTags, [](TagProxyHandle *) -> QVariantHash {
        __DBG_STUB_INVOKE__
        QVariantHash hash;
        hash.insert("/tmp/ftc_impl/trash:42", QStringList() << "ImplTrashTag");
        return hash;
    });

    FileTagCacheWorker worker;
    worker.loadFileTagsFromDatabase();

    EXPECT_EQ(controller->getTagsByFile("/tmp/ftc_impl/file1.txt"), QStringList({ "ImplTag1" }));

    auto colors = controller->getCacheTagsColor({ "ImplTag1" });
    EXPECT_EQ(colors.size(), 1);
    EXPECT_EQ(colors.value("ImplTag1").name().toLower(), QString("#ff0000"));

    auto trashTags = controller->getTrashFileTags("/tmp/ftc_impl/trash", 42);
    EXPECT_EQ(trashTags, QStringList({ "ImplTrashTag" }));
}

TEST_F(FileTagCacheImpl, add_and_delete_tags)
{
    FileTagCacheWorker worker;

    QVariantMap tags;
    tags.insert("ImplTagAdd", "#00ff00");
    worker.onTagAdded(tags);

    auto colors = controller->getCacheTagsColor({ "ImplTagAdd" });
    EXPECT_EQ(colors.size(), 1);
    EXPECT_EQ(colors.value("ImplTagAdd").name().toLower(), QString("#00ff00"));

    worker.onTagDeleted(QVariant(QStringList({ "ImplTagAdd" })));
    colors = controller->getCacheTagsColor({ "ImplTagAdd" });
    EXPECT_TRUE(colors.isEmpty());
}

TEST_F(FileTagCacheImpl, tag_and_untag_files)
{
    FileTagCacheWorker worker;

    QVariantMap tags;
    tags.insert("ImplTagFile", "#0000ff");
    worker.onTagAdded(tags);

    QVariantMap fileTags;
    fileTags.insert("/tmp/ftc_impl/file2.txt", QStringList({ "ImplTagFile" }));
    worker.onFilesTagged(fileTags);

    EXPECT_EQ(controller->getTagsByFile("/tmp/ftc_impl/file2.txt"), QStringList({ "ImplTagFile" }));

    worker.onFilesUntagged(fileTags);
    EXPECT_TRUE(controller->getTagsByFile("/tmp/ftc_impl/file2.txt").isEmpty());
}

TEST_F(FileTagCacheImpl, change_tag_color)
{
    FileTagCacheWorker worker;

    QVariantMap tags;
    tags.insert("ImplTagColor", "#123456");
    worker.onTagAdded(tags);

    QVariantMap colorMap;
    colorMap.insert("ImplTagColor", "#654321");
    worker.onTagsColorChanged(colorMap);

    auto colors = controller->getCacheTagsColor({ "ImplTagColor" });
    EXPECT_EQ(colors.size(), 1);
    EXPECT_EQ(colors.value("ImplTagColor").name().toLower(), QString("#654321"));
}

TEST_F(FileTagCacheImpl, change_tag_name)
{
    FileTagCacheWorker worker;

    QVariantMap tags;
    tags.insert("ImplTagOldName", "#abcdef");
    worker.onTagAdded(tags);

    QVariantMap fileTags;
    fileTags.insert("/tmp/ftc_impl/file3.txt", QStringList({ "ImplTagOldName" }));
    worker.onFilesTagged(fileTags);

    QVariantMap nameMap;
    nameMap.insert("ImplTagOldName", "ImplTagNewName");
    worker.onTagsNameChanged(nameMap);

    auto fileTagsResult = controller->getTagsByFile("/tmp/ftc_impl/file3.txt");
    EXPECT_TRUE(fileTagsResult.contains("ImplTagNewName"));
    EXPECT_FALSE(fileTagsResult.contains("ImplTagOldName"));

    auto colors = controller->getCacheTagsColor({ "ImplTagNewName" });
    EXPECT_EQ(colors.size(), 1);
    EXPECT_EQ(colors.value("ImplTagNewName").name().toLower(), QString("#abcdef"));
}

TEST_F(FileTagCacheImpl, get_tags_by_files_intersection)
{
    FileTagCacheWorker worker;

    QVariantMap tags;
    tags.insert("ImplCommon", "#ff0000");
    tags.insert("ImplOnlyA", "#00ff00");
    tags.insert("ImplOnlyB", "#0000ff");
    worker.onTagAdded(tags);

    QVariantMap fileTags;
    fileTags.insert("/tmp/ftc_impl/fileA.txt", QStringList({ "ImplCommon", "ImplOnlyA" }));
    fileTags.insert("/tmp/ftc_impl/fileB.txt", QStringList({ "ImplCommon", "ImplOnlyB" }));
    worker.onFilesTagged(fileTags);

    auto result = controller->getTagsByFiles({ "/tmp/ftc_impl/fileA.txt", "/tmp/ftc_impl/fileB.txt" });
    EXPECT_EQ(result.size(), 1);
    EXPECT_TRUE(result.contains("ImplCommon"));
}

TEST_F(FileTagCacheImpl, get_tags_by_files_empty_paths)
{
    auto result = controller->getTagsByFiles(QStringList());
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(FileTagCacheImpl, find_children)
{
    FileTagCacheWorker worker;

    QVariantMap tags;
    tags.insert("ImplChildTag", "#ffaa00");
    worker.onTagAdded(tags);

    QVariantMap fileTags;
    fileTags.insert("/tmp/ftc_impl/dir/file1.txt", QStringList({ "ImplChildTag" }));
    fileTags.insert("/tmp/ftc_impl/dir/sub/file2.txt", QStringList({ "ImplChildTag" }));
    fileTags.insert("/tmp/ftc_impl/other/file3.txt", QStringList({ "ImplChildTag" }));
    worker.onFilesTagged(fileTags);

    auto children = controller->findChildren("/tmp/ftc_impl/dir");
    EXPECT_EQ(children.size(), 2);
    EXPECT_TRUE(children.contains("/tmp/ftc_impl/dir/file1.txt"));
    EXPECT_TRUE(children.contains("/tmp/ftc_impl/dir/sub/file2.txt"));
    EXPECT_FALSE(children.contains("/tmp/ftc_impl/other/file3.txt"));
}

TEST_F(FileTagCacheImpl, get_tags_color_ignores_unknown_tags)
{
    FileTagCacheWorker worker;

    QVariantMap tags;
    tags.insert("ImplKnownTag", "#112233");
    worker.onTagAdded(tags);

    auto colors = controller->getCacheTagsColor({ "ImplKnownTag", "ImplUnknownTag" });
    EXPECT_EQ(colors.size(), 1);
    EXPECT_TRUE(colors.contains("ImplKnownTag"));
    EXPECT_FALSE(colors.contains("ImplUnknownTag"));
}

TEST_F(FileTagCacheImpl, reload_trash_file_tags)
{
    stub.set_lamda(&TagProxyHandle::getAllTrashFileTags, [](TagProxyHandle *) -> QVariantHash {
        __DBG_STUB_INVOKE__
        QVariantHash hash;
        hash.insert("/tmp/ftc_impl/trash2:99", QStringList() << "ImplTrashTag2");
        return hash;
    });

    FileTagCacheWorker worker;
    worker.onTrashFileTagsChanged();

    auto trashTags = controller->getTrashFileTags("/tmp/ftc_impl/trash2", 99);
    EXPECT_EQ(trashTags, QStringList({ "ImplTrashTag2" }));
}

TEST_F(FileTagCacheImpl, delete_tag_removes_it_from_files)
{
    FileTagCacheWorker worker;

    QVariantMap tags;
    tags.insert("ImplTagToDelete", "#deadbeef");
    worker.onTagAdded(tags);

    QVariantMap fileTags;
    fileTags.insert("/tmp/ftc_impl/delete_me.txt", QStringList({ "ImplTagToDelete" }));
    worker.onFilesTagged(fileTags);

    EXPECT_FALSE(controller->getTagsByFile("/tmp/ftc_impl/delete_me.txt").isEmpty());

    worker.onTagDeleted(QVariant(QStringList({ "ImplTagToDelete" })));

    EXPECT_TRUE(controller->getTagsByFile("/tmp/ftc_impl/delete_me.txt").isEmpty());
    EXPECT_TRUE(controller->getCacheTagsColor({ "ImplTagToDelete" }).isEmpty());
}
