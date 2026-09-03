// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QUrl>
#include <QColor>

#include "stubext.h"

#include "plugins/common/dfmplugin-tag/utils/tagmanager.h"
#include "plugins/common/dfmplugin-tag/utils/taghelper.h"
#include "plugins/common/dfmplugin-tag/utils/filetagcache.h"
#include "plugins/common/dfmplugin-tag/events/tageventcaller.h"
#include "plugins/common/dfmplugin-tag/files/tagfileinfo.h"
#include "plugins/common/dfmplugin-tag/data/tagproxyhandle.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/file/local/desktopfileinfo.h>

#include <dfm-framework/dpf.h>

using namespace dfmplugin_tag;
DFMBASE_USE_NAMESPACE

class TagManagerReal2 : public testing::Test
{
public:
    void SetUp() override
    {
        // avoid FileTagCache background thread creation
        stub.set_lamda(ADDR(QThread, start), [](QThread *, QThread::Priority) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(ADDR(QThread, quit), [](QThread *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(static_cast<bool (QThread::*)(QDeadlineTimer)>(&QThread::wait), [] { __DBG_STUB_INVOKE__ return true; });

        // UrlRoute::urlToPath() requires the scheme to be registered,
        // otherwise it returns an empty string.
        UrlRoute::regScheme(Global::Scheme::kFile, "/");

        // harmless stubs for framework/sidebar interactions
        stub.set_lamda(&TagHelper::makeTagUrlByTagName, [](const TagHelper *, const QString &tag) -> QUrl {
            __DBG_STUB_INVOKE__
            return QUrl(QString("tag:///%1").arg(tag));
        });
        stub.set_lamda(&TagHelper::createSidebarItemInfo, [](const TagHelper *, const QString &tag) -> QVariantMap {
            __DBG_STUB_INVOKE__
            return QVariantMap { { "tag", tag } };
        });
        stub.set_lamda(&TagHelper::queryIconNameByColor, [](const TagHelper *, const QColor &) -> QString {
            __DBG_STUB_INVOKE__
            return QString("dfm_tag_icon");
        });
        stub.set_lamda(&TagEventCaller::sendFileUpdate, [](const QString &) { __DBG_STUB_INVOKE__ });

        manager = TagManager::instance();
    }

    void TearDown() override
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

protected:
    stub_ext::StubExt stub;
    TagManager *manager = nullptr;
};

TEST_F(TagManagerReal2, getAllTags_converts_real_data)
{
    stub.set_lamda(&TagProxyHandle::getAllTags, [](TagProxyHandle *) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map.insert("Real2Red", QVariant(QColor("#ff0000")));
        return map;
    });

    auto result = manager->getAllTags();
    EXPECT_EQ(result.size(), 1);
    EXPECT_TRUE(result.contains("Real2Red"));
    EXPECT_EQ(result.value("Real2Red").name().toLower(), QString("#ff0000"));
}

TEST_F(TagManagerReal2, getTagsColor_converts_real_data)
{
    stub.set_lamda(&TagProxyHandle::getTagsColor, [](TagProxyHandle *, const QStringList &) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map.insert("Real2Blue", QVariant("#0000ff"));
        return map;
    });

    auto result = manager->getTagsColor({ "Real2Blue", "Real2Missing" });
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.value("Real2Blue").name().toLower(), QString("#0000ff"));
}

TEST_F(TagManagerReal2, getTagIconName_real_logic)
{
    stub.set_lamda(static_cast<QMap<QString, QString> (TagManager::*)(const QStringList &) const>(&TagManager::getTagsColorName),
                   [](const TagManager *, const QStringList &) -> QMap<QString, QString> {
                       __DBG_STUB_INVOKE__
                       QMap<QString, QString> map;
                       map.insert("Real2IconTag", "#00ff00");
                       return map;
                   });
    stub.set_lamda(&TagHelper::queryIconNameByColor, [](const TagHelper *, const QColor &) -> QString {
        __DBG_STUB_INVOKE__
        return QString("dfm_tag_green");
    });

    EXPECT_EQ(manager->getTagIconName("Real2IconTag"), QString("dfm_tag_green"));
    EXPECT_TRUE(manager->getTagIconName("").isEmpty());
}

TEST_F(TagManagerReal2, getTagsByUrls_uses_real_cache)
{
    resetFileTagCache();

    FileTagCacheWorker worker;
    worker.onTagAdded({ { "Real2CacheTag", "#ff0000" } });
    worker.onFilesTagged({ { "/tmp/real2/cache.txt", QStringList { "Real2CacheTag" } } });

    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [](const QList<QUrl> &urls, QList<QUrl> *realUrls) -> bool {
        __DBG_STUB_INVOKE__
        *realUrls = urls;
        return true;
    });
    stub.set_lamda(&TagHelper::commonUrls, [](const QList<QUrl> &urls) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return urls;
    });

    auto tags = manager->getTagsByUrls({ QUrl("file:///tmp/real2/cache.txt") });
    EXPECT_EQ(tags, QStringList({ "Real2CacheTag" }));
}

TEST_F(TagManagerReal2, getFilesByTag_real_logic)
{
    stub.set_lamda(&TagProxyHandle::getFilesThroughTag, [](TagProxyHandle *, const QStringList &) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map.insert("Real2FindTag", QVariant(QStringList({ "/tmp/real2/a.txt", "/tmp/real2/b.txt" })));
        return map;
    });

    auto files = manager->getFilesByTag("Real2FindTag");
    EXPECT_EQ(files.size(), 2);
    EXPECT_TRUE(files.contains("/tmp/real2/a.txt"));
}

TEST_F(TagManagerReal2, findChildren_real_logic)
{
    resetFileTagCache();

    FileTagCacheWorker worker;
    worker.onTagAdded({ { "Real2ChildTag", "#ff0000" } });
    worker.onFilesTagged({
        { "/tmp/real2/dir/file1.txt", QStringList { "Real2ChildTag" } },
        { "/tmp/real2/dir/sub/file2.txt", QStringList { "Real2ChildTag" } },
        { "/tmp/real2/other/file3.txt", QStringList { "Real2ChildTag" } }
    });

    stub.set_lamda(&FileUtils::bindUrlTransform, [](const QUrl &url) -> QUrl {
        __DBG_STUB_INVOKE__
        return url;
    });

    auto children = manager->findChildren("/tmp/real2/dir");
    EXPECT_EQ(children.size(), 2);
    EXPECT_TRUE(children.contains("/tmp/real2/dir/file1.txt"));
    EXPECT_TRUE(children.contains("/tmp/real2/dir/sub/file2.txt"));
}

TEST_F(TagManagerReal2, setTagsForFiles_real_orchestration)
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
    stub.set_lamda(static_cast<QStringList (TagManager::*)(const QList<QUrl> &) const>(&TagManager::getTagsByUrls),
                   [](const TagManager *, const QList<QUrl> &) -> QStringList {
                       __DBG_STUB_INVOKE__
                       return QStringList({ "Real2OldTag" });
                   });
    stub.set_lamda(&TagProxyHandle::addTags, [](TagProxyHandle *, const QVariantMap &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&TagProxyHandle::addTagsForFiles, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("/tmp/real2/set.txt"));
        return true;
    });
    stub.set_lamda(&TagProxyHandle::deleteFileTags, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("/tmp/real2/set.txt"));
        return true;
    });

    bool result = manager->setTagsForFiles({ "Real2NewTag" }, { QUrl("file:///tmp/real2/set.txt") });
    EXPECT_TRUE(result);
}

TEST_F(TagManagerReal2, addTagsForFiles_real_logic)
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
        return QColor("#00ff00");
    });
    stub.set_lamda(&TagProxyHandle::addTags, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("Real2AddTag"));
        return true;
    });
    stub.set_lamda(&TagProxyHandle::addTagsForFiles, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("/tmp/real2/add.txt"));
        return true;
    });

    bool result = manager->addTagsForFiles({ "Real2AddTag" }, { QUrl("file:///tmp/real2/add.txt") });
    EXPECT_TRUE(result);
}

TEST_F(TagManagerReal2, removeTagsOfFiles_real_logic)
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
        EXPECT_TRUE(map.contains("/tmp/real2/remove.txt"));
        auto tags = map.value("/tmp/real2/remove.txt").toStringList();
        EXPECT_TRUE(tags.contains("Real2RemoveTag"));
        return true;
    });

    bool result = manager->removeTagsOfFiles({ "Real2RemoveTag" }, { QUrl("file:///tmp/real2/remove.txt") });
    EXPECT_TRUE(result);
}

TEST_F(TagManagerReal2, changeTagColor_normal_path_real)
{
    stub.set_lamda(&TagHelper::queryColorByColorName, [](const TagHelper *, const QString &) -> QColor {
        __DBG_STUB_INVOKE__
        return QColor("#123456");
    });
    stub.set_lamda(&TagHelper::queryDisplayNameByColor, [](const TagHelper *, const QColor &) -> QString {
        __DBG_STUB_INVOKE__
        return QString("Custom");
    });
    stub.set_lamda(&TagHelper::isDefaultTag, [](const TagHelper *, const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&TagProxyHandle::changeTagsColor, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("Real2ColorTag"));
        return true;
    });

    QSignalSpy deletedEmittedSpy(manager, &TagManager::tagDeleted);

    EXPECT_TRUE(manager->changeTagColor("Real2ColorTag", "Custom"));
    EXPECT_EQ(deletedEmittedSpy.count(), 1);
}

TEST_F(TagManagerReal2, changeTagName_default_color_sync_real)
{
    stub.set_lamda(static_cast<TagManager::TagColorMap (TagManager::*)()>(&TagManager::getAllTags), []() -> TagManager::TagColorMap {
        __DBG_STUB_INVOKE__
        return {};
    });
    stub.set_lamda(&TagHelper::queryColorByDisplayName, [](const TagHelper *, const QString &) -> QColor {
        __DBG_STUB_INVOKE__
        return QColor("#abcdef");
    });
    stub.set_lamda(&TagProxyHandle::changeTagsColor, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("Real2NameOld"));
        return true;
    });
    stub.set_lamda(&TagProxyHandle::changeTagNamesWithFiles, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("Real2NameOld"));
        EXPECT_EQ(map.value("Real2NameOld").toString(), QString("NewName"));
        return true;
    });

    QSignalSpy deletedEmittedSpy(manager, &TagManager::tagDeleted);

    EXPECT_TRUE(manager->changeTagName("Real2NameOld", "NewName"));
    EXPECT_EQ(deletedEmittedSpy.count(), 1);
}

TEST_F(TagManagerReal2, deleteTags_real_logic)
{
    stub.set_lamda(&TagProxyHandle::deleteTags, [](TagProxyHandle *, const QVariantMap &map) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(map.contains("deleteTagData"));
        return true;
    });

    QSignalSpy deletedEmittedSpy(manager, &TagManager::tagDeleted);

    manager->deleteTags({ "Real2DeleteTag" });
    EXPECT_EQ(deletedEmittedSpy.count(), 1);
}

TEST_F(TagManagerReal2, removeChildren_real_logic)
{
    resetFileTagCache();

    FileTagCacheWorker worker;
    worker.onTagAdded({ { "Real2RemoveChildTag", "#ff0000" } });
    worker.onFilesTagged({
        { "/tmp/real2/children/a.txt", QStringList { "Real2RemoveChildTag" } },
        { "/tmp/real2/children/b.txt", QStringList { "Real2RemoveChildTag" } }
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

    EXPECT_TRUE(manager->removeChildren("/tmp/real2/children"));
    EXPECT_EQ(removedPaths.size(), 2);
    EXPECT_TRUE(removedPaths.contains("/tmp/real2/children/a.txt"));
    EXPECT_TRUE(removedPaths.contains("/tmp/real2/children/b.txt"));
}

TEST_F(TagManagerReal2, hideFiles_emits_filesHidden)
{
    QSignalSpy emittedSpy(manager, &TagManager::filesHidden);

    manager->hideFiles({ "Real2HideTag" }, { QUrl("file:///tmp/real2/hide.txt") });
    EXPECT_EQ(emittedSpy.count(), 1);
}

TEST_F(TagManagerReal2, pasteHandle_copy_action_real)
{
    stub.set_lamda(&ClipBoard::clipboardAction, [](ClipBoard *) -> ClipBoard::ClipboardAction {
        __DBG_STUB_INVOKE__
        return ClipBoard::kCopyAction;
    });
    stub.set_lamda(&ClipBoard::clipboardFileUrlList, [](ClipBoard *) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return { QUrl("file:///tmp/real2/source.txt") };
    });
    stub.set_lamda(static_cast<bool (TagManager::*)(const QUrl &) const>(&TagManager::canTagFile),
                   [](const TagManager *, const QUrl &) -> bool {
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

    TagFileInfoPointer tagInfo(new TagFileInfo(QUrl("tag:///Real2PasteTag")));
    stub.set_lamda(static_cast<TagFileInfoPointer(*)(const QUrl &, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<TagFileInfo>),
                   [tagInfo](const QUrl &, Global::CreateFileInfoType, QString *) -> TagFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return tagInfo;
                   });

    EXPECT_TRUE(manager->pasteHandle(0, {}, QUrl("tag:///Real2PasteTag")));
}

TEST_F(TagManagerReal2, fileDropHandle_real_logic)
{
    stub.set_lamda(static_cast<bool (TagManager::*)(const QUrl &) const>(&TagManager::canTagFile),
                   [](const TagManager *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });
    stub.set_lamda(static_cast<bool (TagManager::*)(const QStringList &, const QList<QUrl> &)>(&TagManager::addTagsForFiles),
                   [](TagManager *, const QStringList &, const QList<QUrl> &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    TagFileInfoPointer tagInfo(new TagFileInfo(QUrl("tag:///Real2DropTag")));
    stub.set_lamda(static_cast<TagFileInfoPointer(*)(const QUrl &, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<TagFileInfo>),
                   [tagInfo](const QUrl &, Global::CreateFileInfoType, QString *) -> TagFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return tagInfo;
                   });

    EXPECT_TRUE(manager->fileDropHandle({ QUrl("file:///tmp/real2/drop.txt") }, QUrl("tag:///Real2DropTag")));
}

TEST_F(TagManagerReal2, fileDropHandleWithAction_sets_ignore_action)
{
    Qt::DropAction action = Qt::CopyAction;
    stub.set_lamda(static_cast<bool (TagManager::*)(const QList<QUrl> &, const QUrl &)>(&TagManager::fileDropHandle),
                   [](TagManager *, const QList<QUrl> &, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    EXPECT_TRUE(manager->fileDropHandleWithAction({}, QUrl("tag:///Real2DropTag"), &action));
    EXPECT_EQ(action, Qt::IgnoreAction);
}

TEST_F(TagManagerReal2, trashFileTags_real_logic)
{
    stub.set_lamda(&TagProxyHandle::saveTrashFileTags, [](TagProxyHandle *, const QString &, qint64, const QStringList &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&TagProxyHandle::getTrashFileTags, [](TagProxyHandle *, const QString &, qint64) -> QStringList {
        __DBG_STUB_INVOKE__
        return QStringList({ "Real2TrashTag" });
    });
    stub.set_lamda(&TagProxyHandle::removeTrashFileTags, [](TagProxyHandle *, const QString &, qint64) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&TagProxyHandle::clearAllTrashTags, [](TagProxyHandle *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(manager->saveTrashFileTags("/tmp/real2/original.txt", 123, { "Real2TrashTag" }));
    EXPECT_EQ(manager->getTrashFileTags("/tmp/real2/original.txt", 123), QStringList({ "Real2TrashTag" }));
    EXPECT_TRUE(manager->removeTrashFileTags("/tmp/real2/original.txt", 123));
    EXPECT_TRUE(manager->clearAllTrashTags());
}

TEST_F(TagManagerReal2, assignColorToTags_real_logic)
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
        return QString("Real2ColorName");
    });
    stub.set_lamda(&TagHelper::queryColorByColorName, [](const TagHelper *, const QString &) -> QColor {
        __DBG_STUB_INVOKE__
        return QColor("#654321");
    });

    auto result = manager->assignColorToTags({ "Real2AssignTag" });
    EXPECT_TRUE(result.contains("Real2AssignTag"));
    EXPECT_EQ(result.value("Real2AssignTag").name().toLower(), QString("#654321"));
}

TEST_F(TagManagerReal2, sepateTitlebarCrumb_real_logic)
{
    stub.set_lamda(&TagHelper::getTagNameFromUrl, [](const TagHelper *, const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return QString("Real2CrumbTag");
    });
    stub.set_lamda(static_cast<QString (TagManager::*)(const QString &) const>(&TagManager::getTagIconName),
                   [](const TagManager *, const QString &) -> QString {
                       __DBG_STUB_INVOKE__
                       return QString("dfm_tag_icon");
                   });

    QList<QVariantMap> group;
    EXPECT_TRUE(manager->sepateTitlebarCrumb(QUrl("tag:///Real2CrumbTag"), &group));
    EXPECT_EQ(group.size(), 1);
    EXPECT_EQ(group[0].value("CrumbData_Key_IconName").toString(), QString("dfm_tag_icon"));
}

TEST_F(TagManagerReal2, registerTagColor_real_logic)
{
    EXPECT_TRUE(manager->registerTagColor("Real2RegTag", "#ff0000"));
    EXPECT_FALSE(manager->registerTagColor("Real2RegTag", "#00ff00"));
}

TEST_F(TagManagerReal2, onTagAdded_does_not_crash)
{
    EXPECT_NO_THROW(manager->onTagAdded({ { "Real2OnAdded", "#ff0000" } }));
}

TEST_F(TagManagerReal2, onTagDeleted_emits_signal)
{
    QSignalSpy emittedSpy(manager, &TagManager::tagDeleted);
    manager->onTagDeleted({ "Real2OnDeleted" });
    EXPECT_EQ(emittedSpy.count(), 1);
}

TEST_F(TagManagerReal2, onTagColorChanged_emits_signal)
{
    QSignalSpy emittedSpy(manager, &TagManager::tagColorChanged);
    manager->onTagColorChanged({ { "Real2OnColor", "#ff0000" } });
    EXPECT_EQ(emittedSpy.count(), 1);
}

TEST_F(TagManagerReal2, onTagNameChanged_does_not_crash)
{
    EXPECT_NO_THROW(manager->onTagNameChanged({ { "Real2OnOld", "Real2OnNew" } }));
}

TEST_F(TagManagerReal2, onFilesTagged_emits_signal)
{
    QSignalSpy emittedSpy(manager, &TagManager::filesTagged);
    manager->onFilesTagged({ { "/tmp/real2/tagged.txt", QStringList({ "Real2Tag" }) } });
    EXPECT_EQ(emittedSpy.count(), 1);
}

TEST_F(TagManagerReal2, onFilesUntagged_emits_signal)
{
    QSignalSpy emittedSpy(manager, &TagManager::filesUntagged);
    manager->onFilesUntagged({ { "/tmp/real2/untagged.txt", QStringList({ "Real2Tag" }) } });
    EXPECT_EQ(emittedSpy.count(), 1);
}
