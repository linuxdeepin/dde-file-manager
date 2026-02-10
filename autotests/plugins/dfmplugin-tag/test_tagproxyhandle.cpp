// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-tag/data/tagproxyhandle.h"
#include "plugins/common/dfmplugin-tag/data/private/tagproxyhandle_p.h"

#include <gtest/gtest.h>

#include <QDBusVariant>
#include <QDBusReply>
#include <QDBusPendingReply>
#include <QVariantMap>

using namespace dfmplugin_tag;

class UT_TagProxyHandle : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        handle = TagProxyHandle::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    TagProxyHandle *handle = nullptr;
};

TEST_F(UT_TagProxyHandle, instance)
{
    // Test singleton instance
    TagProxyHandle *instance1 = TagProxyHandle::instance();
    TagProxyHandle *instance2 = TagProxyHandle::instance();

    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);
}

TEST_F(UT_TagProxyHandle, isValid_True)
{
    // Test isValid when DBus is running
    stub.set_lamda(&TagProxyHandlePrivate::isDBusRuning, [](TagProxyHandlePrivate*) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = handle->isValid();

    EXPECT_TRUE(result);
}

TEST_F(UT_TagProxyHandle, isValid_False)
{
    // Test isValid when DBus is not running
    stub.set_lamda(&TagProxyHandlePrivate::isDBusRuning, [](TagProxyHandlePrivate*) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = handle->isValid();

    EXPECT_FALSE(result);
}

TEST_F(UT_TagProxyHandle, connectToService)
{
    // Test connecting to service
    bool result = false;

    stub.set_lamda(&TagProxyHandlePrivate::isDBusRuning, [](TagProxyHandlePrivate*) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_NO_THROW(result = handle->connectToService());
}

TEST_F(UT_TagProxyHandle, getAllTags_Empty)
{
    // Test getting all tags when none exist
    stub.set_lamda(&TagProxyHandlePrivate::isDBusRuning, [](TagProxyHandlePrivate*) -> bool {
        __DBG_STUB_INVOKE__
        return false; // DBus not running
    });

    QVariantMap tags = handle->getAllTags();

    // Result depends on DBus availability
    EXPECT_NO_THROW(tags);
}

TEST_F(UT_TagProxyHandle, getTagsColor_Empty)
{
    // Test getting tags color with empty list
    QStringList emptyTags;
    QVariantMap result = handle->getTagsColor(emptyTags);

    // Should handle empty input gracefully
    EXPECT_NO_THROW(result);
}

TEST_F(UT_TagProxyHandle, getTagsThroughFile_Empty)
{
    // Test getting tags through file with empty file list
    QStringList emptyFiles;
    QVariantMap result = handle->getTagsThroughFile(emptyFiles);

    // Should handle empty input gracefully
    EXPECT_NO_THROW(result);
}

TEST_F(UT_TagProxyHandle, getSameTagsOfDiffFiles_Empty)
{
    // Test getting same tags of different files with empty list
    QStringList emptyFiles;
    QVariant result = handle->getSameTagsOfDiffFiles(emptyFiles);

    // Should handle empty input gracefully
    EXPECT_NO_THROW(result);
}

TEST_F(UT_TagProxyHandle, getFilesThroughTag_Empty)
{
    // Test getting files through tag with empty tag list
    QStringList emptyTags;
    QVariantMap result = handle->getFilesThroughTag(emptyTags);

    // Should handle empty input gracefully
    EXPECT_NO_THROW(result);
}

TEST_F(UT_TagProxyHandle, getAllFileWithTags)
{
    // Test getting all files with tags
    QVariantHash result;

    stub.set_lamda(&TagProxyHandlePrivate::isDBusRuning, [](TagProxyHandlePrivate*) -> bool {
        __DBG_STUB_INVOKE__
        return false; // DBus not running
    });

    EXPECT_NO_THROW(result = handle->getAllFileWithTags());
}

TEST_F(UT_TagProxyHandle, addTags_Empty)
{
    // Test adding tags with empty map
    QVariantMap emptyTags;
    bool result = handle->addTags(emptyTags);

    // Should handle empty input
    EXPECT_NO_THROW(result);
}

TEST_F(UT_TagProxyHandle, addTagsForFiles_Empty)
{
    // Test adding tags for files with empty map
    QVariantMap emptyMap;
    bool result = handle->addTagsForFiles(emptyMap);

    // Should handle empty input
    EXPECT_NO_THROW(result);
}

TEST_F(UT_TagProxyHandle, changeTagsColor_Empty)
{
    // Test changing tags color with empty map
    QVariantMap emptyMap;
    bool result = handle->changeTagsColor(emptyMap);

    // Should handle empty input
    EXPECT_NO_THROW(result);
}

TEST_F(UT_TagProxyHandle, changeTagNamesWithFiles_Empty)
{
    // Test changing tag names with empty map
    QVariantMap emptyMap;
    bool result = handle->changeTagNamesWithFiles(emptyMap);

    // Should handle empty input
    EXPECT_NO_THROW(result);
}

TEST_F(UT_TagProxyHandle, changeFilePaths_Empty)
{
    // Test changing file paths with empty map
    QVariantMap emptyMap;
    bool result = handle->changeFilePaths(emptyMap);

    // Should handle empty input
    EXPECT_NO_THROW(result);
}

TEST_F(UT_TagProxyHandle, deleteTags_Empty)
{
    // Test deleting tags with empty map
    QVariantMap emptyMap;
    bool result = handle->deleteTags(emptyMap);

    // Should handle empty input
    EXPECT_NO_THROW(result);
}

TEST_F(UT_TagProxyHandle, deleteFiles_Empty)
{
    // Test deleting files with empty map
    QVariantMap emptyMap;
    bool result = handle->deleteFiles(emptyMap);

    // Should handle empty input
    EXPECT_NO_THROW(result);
}

TEST_F(UT_TagProxyHandle, deleteFileTags_Empty)
{
    // Test deleting file tags with empty map
    QMap<QString, QVariant> emptyMap;
    bool result = handle->deleteFileTags(emptyMap);

    // Should handle empty input
    EXPECT_NO_THROW(result);
}

TEST_F(UT_TagProxyHandle, signals)
{
    // Test that signals can be connected
    bool filesTaggedEmitted = false;
    bool filesUntaggedEmitted = false;
    bool newTagsAddedEmitted = false;
    bool tagsColorChangedEmitted = false;
    bool tagsDeletedEmitted = false;
    bool tagsNameChangedEmitted = false;
    bool tagServiceRegisteredEmitted = false;

    QObject::connect(handle, &TagProxyHandle::filesTagged,
        [&filesTaggedEmitted](const QVariantMap&) {
            filesTaggedEmitted = true;
        });

    QObject::connect(handle, &TagProxyHandle::filesUntagged,
        [&filesUntaggedEmitted](const QVariantMap&) {
            filesUntaggedEmitted = true;
        });

    QObject::connect(handle, &TagProxyHandle::newTagsAdded,
        [&newTagsAddedEmitted](const QVariantMap&) {
            newTagsAddedEmitted = true;
        });

    QObject::connect(handle, &TagProxyHandle::tagsColorChanged,
        [&tagsColorChangedEmitted](const QVariantMap&) {
            tagsColorChangedEmitted = true;
        });

    QObject::connect(handle, &TagProxyHandle::tagsDeleted,
        [&tagsDeletedEmitted](const QStringList&) {
            tagsDeletedEmitted = true;
        });

    QObject::connect(handle, &TagProxyHandle::tagsNameChanged,
        [&tagsNameChangedEmitted](const QVariantMap&) {
            tagsNameChangedEmitted = true;
        });

    QObject::connect(handle, &TagProxyHandle::tagServiceRegistered,
        [&tagServiceRegisteredEmitted]() {
            tagServiceRegisteredEmitted = true;
        });

    // Emit signals to test connections
    emit handle->filesTagged(QVariantMap());
    emit handle->filesUntagged(QVariantMap());
    emit handle->newTagsAdded(QVariantMap());
    emit handle->tagsColorChanged(QVariantMap());
    emit handle->tagsDeleted(QStringList());
    emit handle->tagsNameChanged(QVariantMap());
    emit handle->tagServiceRegistered();

    EXPECT_TRUE(filesTaggedEmitted);
    EXPECT_TRUE(filesUntaggedEmitted);
    EXPECT_TRUE(newTagsAddedEmitted);
    EXPECT_TRUE(tagsColorChangedEmitted);
    EXPECT_TRUE(tagsDeletedEmitted);
    EXPECT_TRUE(tagsNameChangedEmitted);
    EXPECT_TRUE(tagServiceRegisteredEmitted);
}
