// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-tag/dfmplugin_tag_global.h"
#include "plugins/common/dfmplugin-tag/utils/tagmanager.h"
#include "plugins/common/dfmplugin-tag/utils/taghelper.h"
#include "plugins/common/dfmplugin-tag/utils/filetagcache.h"
#include "plugins/common/dfmplugin-tag/utils/anythingmonitorfilter.h"
#include "plugins/common/dfmplugin-tag/events/tageventcaller.h"
#include "plugins/common/dfmplugin-tag/files/tagfileinfo.h"
#include "plugins/common/dfmplugin-tag/data/tagproxyhandle.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/file/local/desktopfileinfo.h>

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>
#include <QSignalSpy>

#include <QUrl>
#include <QSet>
#include <QColor>

using namespace dfmplugin_tag;
DFMBASE_USE_NAMESPACE

class TagManagerImpl : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // The controller singleton is pulled in by TagManager::instance().
        // Stub the update thread before it gets created.
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

        // Harmless stubs so that FileTagCache signals routed to TagManager slots
        // do not need a fully running framework / sidebar.
        stub.set_lamda(&TagHelper::makeTagUrlByTagName, [](const TagHelper *, const QString &tag) -> QUrl {
            __DBG_STUB_INVOKE__
            return QUrl(QString("tag:///%1").arg(tag));
        });
        stub.set_lamda(&TagHelper::createSidebarItemInfo, [](const TagHelper *, const QString &tag) -> QVariantMap {
            __DBG_STUB_INVOKE__
            QVariantMap map;
            map["tag"] = tag;
            return map;
        });
        stub.set_lamda(&TagHelper::queryIconNameByColor, [](const TagHelper *, const QColor &) -> QString {
            __DBG_STUB_INVOKE__
            return QString("dfm_tag_icon");
        });
        stub.set_lamda(&TagEventCaller::sendFileUpdate, [](const QString &) {
            __DBG_STUB_INVOKE__
        });

        // UrlRoute::urlToPath() requires the scheme to be registered,
        // otherwise it returns an empty string.
        UrlRoute::regScheme(Global::Scheme::kFile, "/");

        manager = TagManager::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

    void resetFileTagCache()
    {
        stub.set_lamda(&TagProxyHandle::getAllFileWithTags, [](TagProxyHandle *) -> QVariantHash {
            __DBG_STUB_INVOKE__
            return QVariantHash();
        });
        stub.set_lamda(&TagProxyHandle::getAllTags, [](TagProxyHandle *) -> QVariantMap {
            __DBG_STUB_INVOKE__
            return QVariantMap();
        });
        stub.set_lamda(&TagProxyHandle::getAllTrashFileTags, [](TagProxyHandle *) -> QVariantHash {
            __DBG_STUB_INVOKE__
            return QVariantHash();
        });
        FileTagCacheWorker worker;
        worker.loadFileTagsFromDatabase();
    }

public:
    stub_ext::StubExt stub;
    TagManager *manager = nullptr;
};

TEST_F(TagManagerImpl, instance_and_scheme_and_root_url)
{
    EXPECT_NE(manager, nullptr);
    EXPECT_EQ(TagManager::instance(), manager);
    EXPECT_EQ(TagManager::scheme(), QString("tag"));

    QUrl root = TagManager::rootUrl();
    EXPECT_EQ(root.scheme(), QString("tag"));
    EXPECT_EQ(root.path(), QString("/"));
}

TEST_F(TagManagerImpl, can_tag_file_by_url_real_logic)
{
    stub.set_lamda(static_cast<FileInfoPointer(*)(const QUrl &, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>),
                   [](const QUrl &, Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return FileInfoPointer(new FileInfo(QUrl("file:///tmp/tm_impl/can_tag.txt")));
                   });
    stub.set_lamda(&AnythingMonitorFilter::instance, []() -> AnythingMonitorFilter & {
        __DBG_STUB_INVOKE__
        static AnythingMonitorFilter filter;
        return filter;
    });
    stub.set_lamda(&AnythingMonitorFilter::whetherFilterCurrentPath, [](AnythingMonitorFilter *, const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&SystemPathUtil::instance, []() -> SystemPathUtil * {
        __DBG_STUB_INVOKE__
        static SystemPathUtil util;
        return &util;
    });
    stub.set_lamda(&SystemPathUtil::isSystemPath, [](SystemPathUtil *, QString) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&FileUtils::isDesktopFileSuffix, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&ProtocolUtils::isSMBFile, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_TRUE(manager->canTagFile(QUrl("file:///tmp/tm_impl/can_tag.txt")));
}

TEST_F(TagManagerImpl, can_tag_file_by_url_transforms_to_local)
{
    // Transform the remote url into a local file:// url, as the real
    // UniversalUtils does for reachable remote shares.
    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [](const QList<QUrl> &urls, QList<QUrl> *realUrls) -> bool {
        __DBG_STUB_INVOKE__
        for (const QUrl &url : urls)
            realUrls->append(QUrl(QString("file://%1").arg(url.path())));
        return true;
    });
    stub.set_lamda(static_cast<FileInfoPointer(*)(const QUrl &, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>),
                   [](const QUrl &, Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return FileInfoPointer(new FileInfo(QUrl("file:///tmp/tm_impl/can_tag2.txt")));
                   });
    stub.set_lamda(&AnythingMonitorFilter::instance, []() -> AnythingMonitorFilter & {
        static AnythingMonitorFilter filter;
        return filter;
    });
    stub.set_lamda(&AnythingMonitorFilter::whetherFilterCurrentPath, [](AnythingMonitorFilter *, const QString &) -> bool {
        return true;
    });
    stub.set_lamda(&SystemPathUtil::instance, []() -> SystemPathUtil * {
        static SystemPathUtil util;
        return &util;
    });
    stub.set_lamda(&SystemPathUtil::isSystemPath, [](SystemPathUtil *, QString) -> bool {
        return false;
    });
    stub.set_lamda(&FileUtils::isDesktopFileSuffix, [](const QUrl &) -> bool { return false; });
    stub.set_lamda(&ProtocolUtils::isSMBFile, [](const QUrl &) -> bool { return false; });

    EXPECT_TRUE(manager->canTagFile(QUrl("smb://share/tmp/tm_impl/can_tag2.txt")));
}

TEST_F(TagManagerImpl, can_tag_file_by_info_real_logic)
{
    FileInfoPointer info(new FileInfo(QUrl("file:///tmp/tm_impl/can_tag3.txt")));

    stub.set_lamda(&AnythingMonitorFilter::instance, []() -> AnythingMonitorFilter & {
        static AnythingMonitorFilter filter;
        return filter;
    });
    stub.set_lamda(&AnythingMonitorFilter::whetherFilterCurrentPath, [](AnythingMonitorFilter *, const QString &) -> bool {
        return true;
    });
    stub.set_lamda(&SystemPathUtil::instance, []() -> SystemPathUtil * {
        static SystemPathUtil util;
        return &util;
    });
    stub.set_lamda(&SystemPathUtil::isSystemPath, [](SystemPathUtil *, QString) -> bool { return false; });
    stub.set_lamda(&FileUtils::isDesktopFileSuffix, [](const QUrl &) -> bool { return false; });
    stub.set_lamda(&ProtocolUtils::isSMBFile, [](const QUrl &) -> bool { return false; });

    EXPECT_TRUE(manager->canTagFile(info));
}

TEST_F(TagManagerImpl, get_tags_by_urls_uses_cache)
{
    resetFileTagCache();

    FileTagCacheWorker worker;
    QVariantMap tags;
    tags.insert("TmUrlTag", "#ff0000");
    worker.onTagAdded(tags);

    QVariantMap fileTags;
    fileTags.insert("/tmp/tm_impl/url_file.txt", QStringList({ "TmUrlTag" }));
    worker.onFilesTagged(fileTags);

    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [](const QList<QUrl> &urls, QList<QUrl> *realUrls) -> bool {
        __DBG_STUB_INVOKE__
        *realUrls = urls;
        return true;
    });
    stub.set_lamda(&TagHelper::commonUrls, [](const QList<QUrl> &urls) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return urls;
    });

    auto result = manager->getTagsByUrls({ QUrl("file:///tmp/tm_impl/url_file.txt") });
    EXPECT_EQ(result, QStringList({ "TmUrlTag" }));
}

TEST_F(TagManagerImpl, find_children_real_logic)
{
    resetFileTagCache();

    FileTagCacheWorker worker;
    QVariantMap tags;
    tags.insert("TmChildTag", "#00ff00");
    worker.onTagAdded(tags);

    QVariantMap fileTags;
    fileTags.insert("/tmp/tm_impl/dir/file1.txt", QStringList({ "TmChildTag" }));
    fileTags.insert("/tmp/tm_impl/dir/sub/file2.txt", QStringList({ "TmChildTag" }));
    fileTags.insert("/tmp/tm_impl/other/file3.txt", QStringList({ "TmChildTag" }));
    worker.onFilesTagged(fileTags);

    stub.set_lamda(&FileUtils::bindUrlTransform, [](const QUrl &url) -> QUrl {
        __DBG_STUB_INVOKE__
        return url;
    });

    auto children = manager->findChildren("/tmp/tm_impl/dir");
    EXPECT_EQ(children.size(), 2);
    EXPECT_TRUE(children.contains("/tmp/tm_impl/dir/file1.txt"));
    EXPECT_TRUE(children.contains("/tmp/tm_impl/dir/sub/file2.txt"));
}

TEST_F(TagManagerImpl, set_tags_for_files_orchestrates_add_and_remove)
{
    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [](const QList<QUrl> &urls, QList<QUrl> *realUrls) -> bool {
        __DBG_STUB_INVOKE__
        *realUrls = urls;
        return true;
    });
    stub.set_lamda(&TagHelper::commonUrls, [](const QList<QUrl> &urls) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return urls;
    });
    // existing tags on the file -> setTagsForFiles must remove old tags not in the new list
    stub.set_lamda(&FileTagCacheController::getTagsByFiles, [](FileTagCacheController *, const QStringList &) -> QStringList {
        __DBG_STUB_INVOKE__
        return QStringList({ "OldTag" });
    });
    stub.set_lamda(&TagHelper::queryColorByDisplayName, [](const TagHelper *, const QString &) -> QColor {
        __DBG_STUB_INVOKE__
        return QColor("#ff0000");
    });
    stub.set_lamda(&TagProxyHandle::addTags, [](TagProxyHandle *, const QVariantMap &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&TagProxyHandle::addTagsForFiles, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("/tmp/tm_impl/set_tag.txt"));
        return true;
    });
    stub.set_lamda(&TagProxyHandle::deleteFileTags, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("/tmp/tm_impl/set_tag.txt"));
        return true;
    });

    QList<QUrl> files = { QUrl("file:///tmp/tm_impl/set_tag.txt") };
    bool result = manager->setTagsForFiles({ "NewTag" }, files);
    EXPECT_TRUE(result);
}

TEST_F(TagManagerImpl, add_tags_for_files_builds_request)
{
    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [](const QList<QUrl> &urls, QList<QUrl> *realUrls) -> bool {
        __DBG_STUB_INVOKE__
        *realUrls = urls;
        return true;
    });
    stub.set_lamda(&TagHelper::commonUrls, [](const QList<QUrl> &urls) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return urls;
    });
    stub.set_lamda(&TagHelper::queryColorByDisplayName, [](const TagHelper *, const QString &) -> QColor {
        __DBG_STUB_INVOKE__
        return QColor("#ff0000");
    });
    stub.set_lamda(&TagProxyHandle::addTags, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("TmNewTag"));
        return true;
    });
    stub.set_lamda(&TagProxyHandle::addTagsForFiles, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("/tmp/tm_impl/add_tag.txt"));
        return true;
    });

    bool result = manager->addTagsForFiles({ "TmNewTag" }, { QUrl("file:///tmp/tm_impl/add_tag.txt") });
    EXPECT_TRUE(result);
}

TEST_F(TagManagerImpl, remove_tags_of_files_builds_request)
{
    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [](const QList<QUrl> &urls, QList<QUrl> *realUrls) -> bool {
        __DBG_STUB_INVOKE__
        *realUrls = urls;
        return true;
    });
    stub.set_lamda(&TagHelper::commonUrls, [](const QList<QUrl> &urls) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return urls;
    });
    stub.set_lamda(&TagProxyHandle::deleteFileTags, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("/tmp/tm_impl/remove_tag.txt"));
        auto tags = map.value("/tmp/tm_impl/remove_tag.txt").toStringList();
        EXPECT_TRUE(tags.contains("TmRemoveTag"));
        return true;
    });

    bool result = manager->removeTagsOfFiles({ "TmRemoveTag" }, { QUrl("file:///tmp/tm_impl/remove_tag.txt") });
    EXPECT_TRUE(result);
}

TEST_F(TagManagerImpl, remove_children_real_logic)
{
    resetFileTagCache();

    FileTagCacheWorker worker;
    QVariantMap tags;
    tags.insert("TmChildRemoveTag", "#ff0000");
    worker.onTagAdded(tags);

    QVariantMap fileTags;
    fileTags.insert("/tmp/tm_impl/children/file1.txt", QStringList({ "TmChildRemoveTag" }));
    fileTags.insert("/tmp/tm_impl/children/file2.txt", QStringList({ "TmChildRemoveTag" }));
    worker.onFilesTagged(fileTags);

    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [](const QList<QUrl> &urls, QList<QUrl> *realUrls) -> bool {
        __DBG_STUB_INVOKE__
        *realUrls = urls;
        return true;
    });
    stub.set_lamda(&TagHelper::commonUrls, [](const QList<QUrl> &urls) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return urls;
    });
    stub.set_lamda(&FileUtils::bindUrlTransform, [](const QUrl &url) -> QUrl {
        __DBG_STUB_INVOKE__
        return url;
    });
    QSet<QString> removedPaths;
    stub.set_lamda(&TagProxyHandle::deleteFileTags, [&](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        // removeChildren() removes tags file by file, one entry per call.
        EXPECT_EQ(map.size(), 1);
        for (auto it = map.begin(); it != map.end(); ++it)
            removedPaths.insert(it.key());
        return true;
    });

    bool result = manager->removeChildren("/tmp/tm_impl/children");
    EXPECT_TRUE(result);
    EXPECT_EQ(removedPaths.size(), 2);
    EXPECT_TRUE(removedPaths.contains("/tmp/tm_impl/children/file1.txt"));
    EXPECT_TRUE(removedPaths.contains("/tmp/tm_impl/children/file2.txt"));
}

TEST_F(TagManagerImpl, get_all_tags_real_logic)
{
    stub.set_lamda(&TagProxyHandle::getAllTags, [](TagProxyHandle *) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map.insert("TmAllTag", QVariant(QColor("#aabbcc")));
        return map;
    });

    auto result = manager->getAllTags();
    EXPECT_EQ(result.size(), 1);
    EXPECT_TRUE(result.contains("TmAllTag"));
    EXPECT_EQ(result.value("TmAllTag").name().toLower(), QString("#aabbcc"));
}

TEST_F(TagManagerImpl, get_tags_color_real_logic)
{
    stub.set_lamda(&TagProxyHandle::getTagsColor, [](TagProxyHandle *, const QStringList &) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map.insert("TmColorTag1", QVariant("#112233"));
        return map;
    });

    auto result = manager->getTagsColor({ "TmColorTag1", "TmUnknown" });
    EXPECT_EQ(result.size(), 1);
    EXPECT_TRUE(result.contains("TmColorTag1"));
    EXPECT_EQ(result.value("TmColorTag1").name().toLower(), QString("#112233"));
}

TEST_F(TagManagerImpl, get_files_by_tag)
{
    stub.set_lamda(&TagProxyHandle::getFilesThroughTag, [](TagProxyHandle *, const QStringList &) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map.insert("TmSearchTag", QVariant(QStringList({ "/tmp/tm_impl/file1.txt", "/tmp/tm_impl/file2.txt" })));
        return map;
    });

    auto result = manager->getFilesByTag("TmSearchTag");
    EXPECT_EQ(result.size(), 2);
}

TEST_F(TagManagerImpl, get_tag_icon_name)
{
    stub.set_lamda(&TagProxyHandle::getTagsColor, [](TagProxyHandle *, const QStringList &) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map.insert("TmIconTag", QVariant("#ff0000"));
        return map;
    });
    stub.set_lamda(&TagHelper::queryIconNameByColor, [](const TagHelper *, const QColor &) -> QString {
        __DBG_STUB_INVOKE__
        return QString("dfm_tag_red");
    });

    auto iconName = manager->getTagIconName("TmIconTag");
    EXPECT_EQ(iconName, QString("dfm_tag_red"));
}

TEST_F(TagManagerImpl, register_tag_color)
{
    EXPECT_TRUE(manager->registerTagColor("TmRegTag", "#ff0000"));
    EXPECT_FALSE(manager->registerTagColor("TmRegTag", "#00ff00"));
}

TEST_F(TagManagerImpl, change_tag_color_default_sync)
{
    stub.set_lamda(&TagHelper::queryColorByColorName, [](const TagHelper *, const QString &) -> QColor {
        __DBG_STUB_INVOKE__
        return QColor("#ff0000");
    });
    stub.set_lamda(&TagHelper::queryDisplayNameByColor, [](const TagHelper *, const QColor &) -> QString {
        __DBG_STUB_INVOKE__
        return QString("Red");
    });
    stub.set_lamda(&TagHelper::isDefaultTag, [](const TagHelper *, const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(static_cast<TagManager::TagColorMap (TagManager::*)()>(&TagManager::getAllTags), []() -> TagManager::TagColorMap {
        __DBG_STUB_INVOKE__
        return {};
    });
    stub.set_lamda(&TagProxyHandle::changeTagsColor, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("OldDefaultTag"));
        return true;
    });
    stub.set_lamda(&TagProxyHandle::changeTagNamesWithFiles, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("OldDefaultTag"));
        EXPECT_EQ(map.value("OldDefaultTag").toString(), QString("Red"));
        return true;
    });

    QSignalSpy deletedEmittedSpy(manager, &TagManager::tagDeleted);

    bool result = manager->changeTagColor("OldDefaultTag", "Red");
    EXPECT_TRUE(result);
    EXPECT_EQ(deletedEmittedSpy.count(), 1);
}

TEST_F(TagManagerImpl, change_tag_color_normal_path)
{
    stub.set_lamda(&TagHelper::queryColorByColorName, [](const TagHelper *, const QString &) -> QColor {
        __DBG_STUB_INVOKE__
        return QColor("#00ff00");
    });
    stub.set_lamda(&TagHelper::queryDisplayNameByColor, [](const TagHelper *, const QColor &) -> QString {
        __DBG_STUB_INVOKE__
        return QString("Green");
    });
    stub.set_lamda(&TagHelper::isDefaultTag, [](const TagHelper *, const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&TagProxyHandle::changeTagsColor, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("TmColorTag"));
        return true;
    });

    QSignalSpy deletedEmittedSpy(manager, &TagManager::tagDeleted);

    bool result = manager->changeTagColor("TmColorTag", "Green");
    EXPECT_TRUE(result);
    EXPECT_EQ(deletedEmittedSpy.count(), 1);
}

TEST_F(TagManagerImpl, change_tag_name_default_color_sync)
{
    stub.set_lamda(static_cast<TagManager::TagColorMap (TagManager::*)()>(&TagManager::getAllTags), []() -> TagManager::TagColorMap {
        __DBG_STUB_INVOKE__
        return {};
    });
    stub.set_lamda(&TagHelper::queryColorByDisplayName, [](const TagHelper *, const QString &) -> QColor {
        __DBG_STUB_INVOKE__
        return QColor("#0000ff");
    });
    stub.set_lamda(&TagProxyHandle::changeTagsColor, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("TmNameOld"));
        return true;
    });
    stub.set_lamda(&TagProxyHandle::changeTagNamesWithFiles, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("TmNameOld"));
        EXPECT_EQ(map.value("TmNameOld").toString(), QString("Blue"));
        return true;
    });

    QSignalSpy deletedEmittedSpy(manager, &TagManager::tagDeleted);

    bool result = manager->changeTagName("TmNameOld", "Blue");
    EXPECT_TRUE(result);
    EXPECT_EQ(deletedEmittedSpy.count(), 1);
}

TEST_F(TagManagerImpl, assign_color_to_tags)
{
    stub.set_lamda(static_cast<TagManager::TagColorMap (TagManager::*)()>(&TagManager::getAllTags), []() -> TagManager::TagColorMap {
        __DBG_STUB_INVOKE__
        return {};
    });
    // Avoid touching the daemon DBus interface (null in the test environment).
    stub.set_lamda(&TagProxyHandle::getTagsColor, [](TagProxyHandle *, const QStringList &) -> QVariantMap {
        __DBG_STUB_INVOKE__
        return {};
    });
    stub.set_lamda(&TagHelper::getColorNameByTag, [](const TagHelper *, const QString &) -> QString {
        __DBG_STUB_INVOKE__
        return QString("TmColorName");
    });
    stub.set_lamda(&TagHelper::queryColorByColorName, [](const TagHelper *, const QString &) -> QColor {
        __DBG_STUB_INVOKE__
        return QColor("#123456");
    });

    auto result = manager->assignColorToTags({ "TmAssignTag" });
    EXPECT_TRUE(result.contains("TmAssignTag"));
    EXPECT_EQ(result.value("TmAssignTag").name().toLower(), QString("#123456"));
}

TEST_F(TagManagerImpl, sepate_titlebar_crumb)
{
    stub.set_lamda(&TagHelper::getTagNameFromUrl, [](const TagHelper *, const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return QString("TmCrumbTag");
    });
    stub.set_lamda(static_cast<QString (TagManager::*)(const QString &) const>(&TagManager::getTagIconName), [](const TagManager *, const QString &) -> QString {
        __DBG_STUB_INVOKE__
        return QString("dfm_tag_icon");
    });

    QList<QVariantMap> group;
    bool result = manager->sepateTitlebarCrumb(QUrl("tag:///TmCrumbTag"), &group);
    EXPECT_TRUE(result);
    EXPECT_EQ(group.size(), 1);
    EXPECT_EQ(group[0].value("CrumbData_Key_IconName"), QString("dfm_tag_icon"));
}

TEST_F(TagManagerImpl, delete_tags_emits_signal)
{
    stub.set_lamda(static_cast<bool (TagManager::*)(const QStringList &, const DeleteOpts &)>(&TagManager::deleteTagData), [](TagManager *, const QStringList &, const DeleteOpts &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QSignalSpy deletedEmittedSpy(manager, &TagManager::tagDeleted);

    manager->deleteTags({ "TmDeleteTag" });
    EXPECT_EQ(deletedEmittedSpy.count(), 1);
}

TEST_F(TagManagerImpl, hide_files_emits_signal)
{
    QSignalSpy emittedSpy(manager, &TagManager::filesHidden);

    manager->hideFiles({ "TmHideTag" }, { QUrl("file:///tmp/tm_impl/hide.txt") });
    EXPECT_EQ(emittedSpy.count(), 1);
}

TEST_F(TagManagerImpl, paste_handle_copy_action)
{
    stub.set_lamda(&ClipBoard::clipboardAction, [](ClipBoard *) -> ClipBoard::ClipboardAction {
        __DBG_STUB_INVOKE__
        return ClipBoard::kCopyAction;
    });
    stub.set_lamda(&ClipBoard::clipboardFileUrlList, [](ClipBoard *) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return { QUrl("file:///tmp/tm_impl/source.txt") };
    });
    stub.set_lamda(static_cast<bool (TagManager::*)(const QUrl &) const>(&TagManager::canTagFile), [](const TagManager *, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [](const QList<QUrl> &urls, QList<QUrl> *realUrls) -> bool {
        __DBG_STUB_INVOKE__
        *realUrls = urls;
        return true;
    });
    stub.set_lamda(&TagHelper::commonUrls, [](const QList<QUrl> &urls) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return urls;
    });
    stub.set_lamda(&TagHelper::queryColorByDisplayName, [](const TagHelper *, const QString &) -> QColor {
        __DBG_STUB_INVOKE__
        return QColor("#ff0000");
    });
    stub.set_lamda(&TagProxyHandle::addTags, [](TagProxyHandle *, const QVariantMap &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&TagProxyHandle::addTagsForFiles, [](TagProxyHandle *, const QVariantMap &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    TagFileInfoPointer tagInfo(new TagFileInfo(QUrl("tag:///TmPasteTag")));
    stub.set_lamda(static_cast<TagFileInfoPointer(*)(const QUrl &, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<TagFileInfo>),
                   [tagInfo](const QUrl &, Global::CreateFileInfoType, QString *) -> TagFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return tagInfo;
                   });

    bool result = manager->pasteHandle(0, {}, QUrl("tag:///TmPasteTag"));
    EXPECT_TRUE(result);
}

TEST_F(TagManagerImpl, file_drop_handle_real_logic)
{
    stub.set_lamda(static_cast<bool (TagManager::*)(const QUrl &) const>(&TagManager::canTagFile), [](const TagManager *, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(static_cast<bool (TagManager::*)(const QStringList &, const QList<QUrl> &)>(&TagManager::setTagsForFiles), [](TagManager *, const QStringList &, const QList<QUrl> &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    TagFileInfoPointer tagInfo(new TagFileInfo(QUrl("tag:///TmDropTag")));
    stub.set_lamda(static_cast<TagFileInfoPointer(*)(const QUrl &, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<TagFileInfo>),
                   [tagInfo](const QUrl &, Global::CreateFileInfoType, QString *) -> TagFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return tagInfo;
                   });

    bool result = manager->fileDropHandle({ QUrl("file:///tmp/tm_impl/drop.txt") }, QUrl("tag:///TmDropTag"));
    EXPECT_TRUE(result);
}

TEST_F(TagManagerImpl, file_drop_handle_with_action)
{
    Qt::DropAction action = Qt::CopyAction;
    stub.set_lamda(static_cast<bool (TagManager::*)(const QList<QUrl> &, const QUrl &)>(&TagManager::fileDropHandle), [](TagManager *, const QList<QUrl> &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = manager->fileDropHandleWithAction({}, QUrl("tag:///TmDropTag"), &action);
    EXPECT_TRUE(result);
    EXPECT_EQ(action, Qt::IgnoreAction);
}

TEST_F(TagManagerImpl, trash_file_tags)
{
    stub.set_lamda(&TagProxyHandle::saveTrashFileTags, [](TagProxyHandle *, const QString &, qint64, const QStringList &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&TagProxyHandle::getTrashFileTags, [](TagProxyHandle *, const QString &, qint64) -> QStringList {
        __DBG_STUB_INVOKE__
        return QStringList({ "TmTrashTag" });
    });
    stub.set_lamda(&TagProxyHandle::removeTrashFileTags, [](TagProxyHandle *, const QString &, qint64) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&TagProxyHandle::clearAllTrashTags, [](TagProxyHandle *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(manager->saveTrashFileTags("/tmp/tm_impl/original.txt", 123, { "TmTrashTag" }));

    auto tags = manager->getTrashFileTags("/tmp/tm_impl/original.txt", 123);
    EXPECT_EQ(tags, QStringList({ "TmTrashTag" }));

    EXPECT_TRUE(manager->removeTrashFileTags("/tmp/tm_impl/original.txt", 123));
    EXPECT_TRUE(manager->clearAllTrashTags());
}

TEST_F(TagManagerImpl, get_trash_tags_uses_cache_when_present)
{
    resetFileTagCache();

    stub.set_lamda(&TagProxyHandle::getAllTrashFileTags, [](TagProxyHandle *) -> QVariantHash {
        __DBG_STUB_INVOKE__
        QVariantHash hash;
        hash.insert("/tmp/tm_impl/trash:777", QStringList({ "TmTrashCached" }));
        return hash;
    });

    FileTagCacheWorker worker;
    worker.onTrashFileTagsChanged();

    auto tags = manager->getTrashFileTags("/tmp/tm_impl/trash", 777);
    EXPECT_EQ(tags, QStringList({ "TmTrashCached" }));
}

TEST_F(TagManagerImpl, on_tag_added_does_not_crash)
{
    EXPECT_NO_THROW(manager->onTagAdded({ { "TmOnAddedTag", "#ff0000" } }));
}

TEST_F(TagManagerImpl, on_tag_deleted_emits_signal)
{
    QSignalSpy emittedSpy(manager, &TagManager::tagDeleted);

    manager->onTagDeleted({ "TmOnDeletedTag" });
    EXPECT_EQ(emittedSpy.count(), 1);
}

TEST_F(TagManagerImpl, on_tag_color_changed_emits_signal)
{
    QSignalSpy emittedSpy(manager, &TagManager::tagColorChanged);

    manager->onTagColorChanged({ { "TmOnColorTag", "#ff0000" } });
    EXPECT_EQ(emittedSpy.count(), 1);
}

TEST_F(TagManagerImpl, on_tag_name_changed_does_not_crash)
{
    EXPECT_NO_THROW(manager->onTagNameChanged({ { "TmOnOldName", "TmOnNewName" } }));
}

TEST_F(TagManagerImpl, on_files_tagged_emits_signal)
{
    QSignalSpy emittedSpy(manager, &TagManager::filesTagged);

    manager->onFilesTagged({ { "/tmp/tm_impl/tagged.txt", QStringList({ "Tag" }) } });
    EXPECT_EQ(emittedSpy.count(), 1);
}

TEST_F(TagManagerImpl, on_files_untagged_emits_signal)
{
    QSignalSpy emittedSpy(manager, &TagManager::filesUntagged);

    manager->onFilesUntagged({ { "/tmp/tm_impl/untagged.txt", QStringList({ "Tag" }) } });
    EXPECT_EQ(emittedSpy.count(), 1);
}
