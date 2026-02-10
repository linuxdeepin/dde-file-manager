// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-tag/files/tagfilewatcher.h"
#include "plugins/common/dfmplugin-tag/files/private/tagfilewatcher_p.h"
#include "plugins/common/dfmplugin-tag/utils/taghelper.h"

#include <dfm-base/utils/universalutils.h>

#include <gtest/gtest.h>

#include <QUrl>
#include <QSignalSpy>

using namespace dfmplugin_tag;
DFMBASE_USE_NAMESPACE

class UT_TagFileWatcher : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        testUrl.setScheme("tag");
        testUrl.setPath("/TestTag");

        // Stub initialization methods
        stub.set_lamda(&TagFileWatcherPrivate::initFileWatcher, [](TagFileWatcherPrivate*) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&TagFileWatcherPrivate::initConnect, [](TagFileWatcherPrivate*) {
            __DBG_STUB_INVOKE__
        });

        watcher = new TagFileWatcher(testUrl);
    }

    virtual void TearDown() override
    {
        delete watcher;
        watcher = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    TagFileWatcher *watcher = nullptr;
    QUrl testUrl;
};

TEST_F(UT_TagFileWatcher, constructor)
{
    // Test constructor
    EXPECT_NE(watcher, nullptr);
}

TEST_F(UT_TagFileWatcher, setEnabledSubfileWatcher)
{
    // Test setEnabledSubfileWatcher (currently does nothing)
    QUrl subfileUrl = QUrl::fromLocalFile("/home/user/test.txt");

    EXPECT_NO_THROW(watcher->setEnabledSubfileWatcher(subfileUrl, true));
    EXPECT_NO_THROW(watcher->setEnabledSubfileWatcher(subfileUrl, false));
}

TEST_F(UT_TagFileWatcher, onTagRemoved_MatchingTag)
{
    // Test when the removed tag matches the watcher's tag
    QSignalSpy spy(watcher, &AbstractFileWatcher::fileDeleted);

    stub.set_lamda(&TagHelper::instance, []() -> TagHelper* {
        __DBG_STUB_INVOKE__
        static TagHelper helper;
        return &helper;
    });

    stub.set_lamda(&TagHelper::makeTagUrlByTagName, [](const TagHelper*, const QString&) -> QUrl {
        __DBG_STUB_INVOKE__
        QUrl url;
        url.setScheme("tag");
        url.setPath("/TestTag");
        return url;
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl&, const QUrl&) -> bool {
        __DBG_STUB_INVOKE__
        return true; // URLs match
    });

    watcher->onTagRemoved("TestTag");

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_TagFileWatcher, onTagRemoved_DifferentTag)
{
    // Test when the removed tag doesn't match
    QSignalSpy spy(watcher, &AbstractFileWatcher::fileDeleted);

    stub.set_lamda(&TagHelper::instance, []() -> TagHelper* {
        __DBG_STUB_INVOKE__
        static TagHelper helper;
        return &helper;
    });

    stub.set_lamda(&TagHelper::makeTagUrlByTagName, [](const TagHelper*, const QString&) -> QUrl {
        __DBG_STUB_INVOKE__
        QUrl url;
        url.setScheme("tag");
        url.setPath("/OtherTag");
        return url;
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl&, const QUrl&) -> bool {
        __DBG_STUB_INVOKE__
        return false; // URLs don't match
    });

    watcher->onTagRemoved("OtherTag");

    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UT_TagFileWatcher, onFilesTagged_SingleFile)
{
    // Test when a single file is tagged
    QSignalSpy spy(watcher, &AbstractFileWatcher::subfileCreated);

    stub.set_lamda(&TagHelper::instance, []() -> TagHelper* {
        __DBG_STUB_INVOKE__
        static TagHelper helper;
        return &helper;
    });

    stub.set_lamda(&TagHelper::getTagNameFromUrl, [](const TagHelper*, const QUrl&) -> QString {
        __DBG_STUB_INVOKE__
        return "TestTag";
    });

    QVariantMap fileAndTags;
    QStringList tags;
    tags << "TestTag";
    fileAndTags["/home/user/test.txt"] = tags;

    watcher->onFilesTagged(fileAndTags);

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_TagFileWatcher, onFilesTagged_MultipleFiles)
{
    // Test when multiple files are tagged
    QSignalSpy spy(watcher, &AbstractFileWatcher::subfileCreated);

    stub.set_lamda(&TagHelper::instance, []() -> TagHelper* {
        __DBG_STUB_INVOKE__
        static TagHelper helper;
        return &helper;
    });

    stub.set_lamda(&TagHelper::getTagNameFromUrl, [](const TagHelper*, const QUrl&) -> QString {
        __DBG_STUB_INVOKE__
        return "TestTag";
    });

    QVariantMap fileAndTags;
    QStringList tags;
    tags << "TestTag";
    fileAndTags["/home/user/file1.txt"] = tags;
    fileAndTags["/home/user/file2.txt"] = tags;
    fileAndTags["/home/user/file3.txt"] = tags;

    watcher->onFilesTagged(fileAndTags);

    EXPECT_EQ(spy.count(), 3);
}

TEST_F(UT_TagFileWatcher, onFilesTagged_DifferentTag)
{
    // Test when files are tagged with different tag
    QSignalSpy spy(watcher, &AbstractFileWatcher::subfileCreated);

    stub.set_lamda(&TagHelper::instance, []() -> TagHelper* {
        __DBG_STUB_INVOKE__
        static TagHelper helper;
        return &helper;
    });

    stub.set_lamda(&TagHelper::getTagNameFromUrl, [](const TagHelper*, const QUrl&) -> QString {
        __DBG_STUB_INVOKE__
        return "TestTag";
    });

    QVariantMap fileAndTags;
    QStringList tags;
    tags << "OtherTag"; // Different tag
    fileAndTags["/home/user/test.txt"] = tags;

    watcher->onFilesTagged(fileAndTags);

    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UT_TagFileWatcher, onFilesUntagged_SingleFile)
{
    // Test when a single file is untagged
    QSignalSpy spy(watcher, &AbstractFileWatcher::fileDeleted);

    stub.set_lamda(&TagHelper::instance, []() -> TagHelper* {
        __DBG_STUB_INVOKE__
        static TagHelper helper;
        return &helper;
    });

    stub.set_lamda(&TagHelper::getTagNameFromUrl, [](const TagHelper*, const QUrl&) -> QString {
        __DBG_STUB_INVOKE__
        return "TestTag";
    });

    QVariantMap fileAndTags;
    QStringList tags;
    tags << "TestTag";
    fileAndTags["/home/user/test.txt"] = tags;

    watcher->onFilesUntagged(fileAndTags);

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_TagFileWatcher, onFilesUntagged_MultipleFiles)
{
    // Test when multiple files are untagged
    QSignalSpy spy(watcher, &AbstractFileWatcher::fileDeleted);

    stub.set_lamda(&TagHelper::instance, []() -> TagHelper* {
        __DBG_STUB_INVOKE__
        static TagHelper helper;
        return &helper;
    });

    stub.set_lamda(&TagHelper::getTagNameFromUrl, [](const TagHelper*, const QUrl&) -> QString {
        __DBG_STUB_INVOKE__
        return "TestTag";
    });

    QVariantMap fileAndTags;
    QStringList tags;
    tags << "TestTag";
    fileAndTags["/home/user/file1.txt"] = tags;
    fileAndTags["/home/user/file2.txt"] = tags;

    watcher->onFilesUntagged(fileAndTags);

    EXPECT_EQ(spy.count(), 2);
}

TEST_F(UT_TagFileWatcher, onFilesHidden_SingleFile)
{
    // Test when a single file is hidden
    QSignalSpy spy(watcher, &AbstractFileWatcher::fileAttributeChanged);

    stub.set_lamda(&TagHelper::instance, []() -> TagHelper* {
        __DBG_STUB_INVOKE__
        static TagHelper helper;
        return &helper;
    });

    stub.set_lamda(&TagHelper::getTagNameFromUrl, [](const TagHelper*, const QUrl&) -> QString {
        __DBG_STUB_INVOKE__
        return "TestTag";
    });

    QVariantMap fileAndTags;
    QStringList tags;
    tags << "TestTag";
    fileAndTags["/home/user/test.txt"] = tags;

    watcher->onFilesHidden(fileAndTags);

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_TagFileWatcher, onFilesHidden_DifferentTag)
{
    // Test when files with different tags are hidden
    QSignalSpy spy(watcher, &AbstractFileWatcher::fileAttributeChanged);

    stub.set_lamda(&TagHelper::instance, []() -> TagHelper* {
        __DBG_STUB_INVOKE__
        static TagHelper helper;
        return &helper;
    });

    stub.set_lamda(&TagHelper::getTagNameFromUrl, [](const TagHelper*, const QUrl&) -> QString {
        __DBG_STUB_INVOKE__
        return "TestTag";
    });

    QVariantMap fileAndTags;
    QStringList tags;
    tags << "OtherTag";
    fileAndTags["/home/user/test.txt"] = tags;

    watcher->onFilesHidden(fileAndTags);

    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UT_TagFileWatcher, onFilesTagged_EmptyMap)
{
    // Test with empty file and tags map
    QSignalSpy spy(watcher, &AbstractFileWatcher::subfileCreated);

    stub.set_lamda(&TagHelper::instance, []() -> TagHelper* {
        __DBG_STUB_INVOKE__
        static TagHelper helper;
        return &helper;
    });

    stub.set_lamda(&TagHelper::getTagNameFromUrl, [](const TagHelper*, const QUrl&) -> QString {
        __DBG_STUB_INVOKE__
        return "TestTag";
    });

    QVariantMap fileAndTags; // Empty map

    watcher->onFilesTagged(fileAndTags);

    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UT_TagFileWatcher, signals)
{
    // Test that all signals can be emitted without crashing
    QSignalSpy subfileCreatedSpy(watcher, &AbstractFileWatcher::subfileCreated);
    QSignalSpy fileDeletedSpy(watcher, &AbstractFileWatcher::fileDeleted);
    QSignalSpy fileAttributeChangedSpy(watcher, &AbstractFileWatcher::fileAttributeChanged);

    stub.set_lamda(&TagHelper::instance, []() -> TagHelper* {
        __DBG_STUB_INVOKE__
        static TagHelper helper;
        return &helper;
    });

    stub.set_lamda(&TagHelper::getTagNameFromUrl, [](const TagHelper*, const QUrl&) -> QString {
        __DBG_STUB_INVOKE__
        return "TestTag";
    });

    stub.set_lamda(&TagHelper::makeTagUrlByTagName, [](const TagHelper*, const QString&) -> QUrl {
        __DBG_STUB_INVOKE__
        QUrl url;
        url.setScheme("tag");
        url.setPath("/TestTag");
        return url;
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl&, const QUrl&) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QVariantMap fileAndTags;
    QStringList tags;
    tags << "TestTag";
    fileAndTags["/home/user/test.txt"] = tags;

    // Trigger different signals
    watcher->onFilesTagged(fileAndTags);
    watcher->onFilesUntagged(fileAndTags);
    watcher->onFilesHidden(fileAndTags);
    watcher->onTagRemoved("TestTag");

    EXPECT_GT(subfileCreatedSpy.count() + fileDeletedSpy.count() + fileAttributeChangedSpy.count(), 0);
}
