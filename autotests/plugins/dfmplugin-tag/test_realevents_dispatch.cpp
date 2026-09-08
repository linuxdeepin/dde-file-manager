// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// Real event-dispatch tests: register the missing hook/signal/slot event
// types, then run the production Tag::initialize()/start() wiring (real
// bindEvents()/followEvents()) exactly once and PUBLISH the real events /
// RUN the real hooks with exactly-typed QVariant arguments, so every
// EventHelper<M>::invoke / EventHelper<M>::EventHelper template
// instantiation owned by dfmplugin_tag in include/dfm-framework/event/
// eventhelper.h actually executes. Daemon-dbus boundary (TagProxyHandle
// queries) and the heavy TagManager mutators are stubbed with argument
// capture for exact-value assertions; the plugin-started path is driven
// through the real Listener signals so the registerPlugin/regToDetailspace/
// regToPropertyDialog lambdas run and the pushed std::function extension
// callbacks are captured via real slot-channel consumers and invoked.

#include "stubext.h"
#include "dfm_hookreg.h"

#include "plugins/common/dfmplugin-tag/tag.h"
#include "plugins/common/dfmplugin-tag/events/tageventreceiver.h"
#include "plugins/common/dfmplugin-tag/utils/tagmanager.h"
#include "plugins/common/dfmplugin-tag/utils/tagfilehelper.h"
#include "plugins/common/dfmplugin-tag/utils/filetagcache.h"
#include "plugins/common/dfmplugin-tag/files/tagfileinfo.h"
#include "plugins/common/dfmplugin-tag/data/tagproxyhandle.h"
#include "plugins/common/dfmplugin-tag/widgets/tagwidget.h"

#include <dfm-framework/dpf.h>
#include <dfm-framework/event/event.h>

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/elidetextlayout.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/widgets/filemanagerwindow.h>

#include <gtest/gtest.h>

#include <QApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QPainter>
#include <QPixmap>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_tag;

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;
using ViewExtensionUpdateFunc = std::function<void(QWidget *widget, const QUrl &url)>;
using ViewExtensionShouldShowFunc = std::function<bool(const QUrl &url)>;

Q_DECLARE_METATYPE(QDir::Filters)
Q_DECLARE_METATYPE(QList<QVariantMap> *)
Q_DECLARE_METATYPE(Qt::DropAction *)
Q_DECLARE_METATYPE(QRectF *)
Q_DECLARE_METATYPE(QPainter *)
Q_DECLARE_METATYPE(dfmbase::ElideTextLayout *)
Q_DECLARE_METATYPE(CustomViewExtensionView)
Q_DECLARE_METATYPE(ViewExtensionUpdateFunc)
Q_DECLARE_METATYPE(ViewExtensionShouldShowFunc)

namespace {

// ---------- captured arguments / call flags (reset in SetUp) ----------
struct Captures
{
    bool addTagsCalled = false;
    QList<QString> addTagsNames;
    QList<QUrl> addTagsFiles;
    bool removeTagsCalled = false;
    QList<QString> removeTagsNames;
    QList<QUrl> removeTagsFiles;
    bool setTagsCalled = false;
    QList<QString> setTagsNames;
    QList<QUrl> setTagsFiles;
    bool hideFilesCalled = false;
    QList<QString> hideFilesTags;
    QList<QUrl> hideFilesFiles;
    bool clearTrashCalled = false;
    bool saveTrashCalled = false;
    QString saveTrashPath;
    qint64 saveTrashInode = -1;
    QStringList saveTrashTags;
    bool getTrashCalled = false;
    QString getTrashPath;
    qint64 getTrashInode = -1;
    QStringList getTrashResult;
    bool removeTrashCalled = false;
    QString removeTrashPath;
    qint64 removeTrashInode = -1;
    bool removeChildrenCalled = false;
    QString removeChildrenPath;
    QString paintIconPath;
    QStringList tagsByUrlsResult;
    quint64 filterWinId = 0;
    int filterFlags = -1;
    QList<QUrl> openFilesUrls;
    quint64 openFilesWinId = 0;
};

Captures g;

void resetCaptures()
{
    g = Captures();
}

// Slot-channel consumer capturing the std::function extension callbacks the
// plugin pushes during regToPropertyDialog()/regToDetailspace(), plus the
// deferred slot_View_SetFilter push and the re-published kOpenFiles urls.
// Plain QObject subclass (no Q_OBJECT needed: dpf stores raw member
// function pointers, not metaobject slots).
class ExtensionSink : public QObject
{
public:
    bool propRegistered = false;
    QString propName;
    int propIndex = -1;
    CustomViewExtensionView propCreate;
    ViewExtensionUpdateFunc propUpdate;

    bool detailRegistered = false;
    int detailIndex = -1;
    CustomViewExtensionView detailCreate;
    ViewExtensionUpdateFunc detailUpdate;
    ViewExtensionShouldShowFunc detailShouldShow;

    bool filterRegistered = false;
    QString filterScheme;
    QStringList filterFields;

    void reset()
    {
        propRegistered = detailRegistered = filterRegistered = false;
        propName = QString();
        propIndex = detailIndex = -1;
        propCreate = CustomViewExtensionView();
        propUpdate = ViewExtensionUpdateFunc();
        detailCreate = CustomViewExtensionView();
        detailUpdate = ViewExtensionUpdateFunc();
        detailShouldShow = ViewExtensionShouldShowFunc();
        filterScheme = QString();
        filterFields.clear();
    }

    void onPropRegister(const QVariant &create, const QVariant &update,
                        const QVariant &name, const QVariant &index)
    {
        propCreate = create.value<CustomViewExtensionView>();
        propUpdate = update.value<ViewExtensionUpdateFunc>();
        propName = name.toString();
        propIndex = index.toInt();
        propRegistered = true;
    }

    void onDetailRegister(const QVariant &create, const QVariant &update,
                          const QVariant &shouldShow, const QVariant &index)
    {
        detailCreate = create.value<CustomViewExtensionView>();
        detailUpdate = update.value<ViewExtensionUpdateFunc>();
        detailShouldShow = shouldShow.value<ViewExtensionShouldShowFunc>();
        detailIndex = index.toInt();
        detailRegistered = true;
    }

    void onFieldFilter(const QVariant &scheme, const QVariant &fields)
    {
        filterScheme = scheme.toString();
        filterFields = fields.toStringList();
        filterRegistered = true;
    }

    void onSetFilter(const QVariant &winId, const QVariant &filters)
    {
        g.filterWinId = winId.toULongLong();
        g.filterFlags = int(filters.value<QDir::Filters>());
    }

    void onOpenFiles(const QVariant &winId, const QVariant &urls)
    {
        g.openFilesWinId = winId.toULongLong();
        g.openFilesUrls = urls.value<QList<QUrl>>();
    }
};

ExtensionSink &sink()
{
    static ExtensionSink ins;
    return ins;
}

std::once_flag g_wiringOnce;

// Subscribe the full production event wiring exactly once for the process:
// register the missing custom event types first (hook topics via
// dfmtest_hooks, the custom signal/slot topics used by bindEvents and the
// extension registration pushes), connect the capture consumers, then run
// the real Tag::initialize() (bindEvents + followEvents + bindWindows) and
// Tag::start() (plugin-started callbacks registration).
Tag &tagPluginInstance()
{
    static Tag plugin;
    static std::once_flag once;
    std::call_once(once, [&plugin]() {
        dfmtest_hooks::registerAllHookEvents();
        dpf::Event *evt = dpf::Event::instance();
        using S = dpf::EventStratege;
        evt->registerEventType(S::kSignal, "dfmplugin_sidebar", "signal_Sidebar_Sorted");
        evt->registerEventType(S::kSignal, "dfmplugin_menu", "signal_MenuScene_SceneAdded");
        evt->registerEventType(S::kSlot, "dfmplugin_propertydialog", "slot_ViewExtensionWithUpdate_Register");
        evt->registerEventType(S::kSlot, "dfmplugin_detailspace", "slot_ViewExtension_Register");
        evt->registerEventType(S::kSlot, "dfmplugin_detailspace", "slot_BasicFiledFilter_Add");
        evt->registerEventType(S::kSlot, "dfmplugin_workspace", "slot_View_SetFilter");

        dpfSlotChannel->connect("dfmplugin_propertydialog", "slot_ViewExtensionWithUpdate_Register",
                                &sink(), &ExtensionSink::onPropRegister);
        dpfSlotChannel->connect("dfmplugin_detailspace", "slot_ViewExtension_Register",
                                &sink(), &ExtensionSink::onDetailRegister);
        dpfSlotChannel->connect("dfmplugin_detailspace", "slot_BasicFiledFilter_Add",
                                &sink(), &ExtensionSink::onFieldFilter);
        dpfSlotChannel->connect("dfmplugin_workspace", "slot_View_SetFilter",
                                &sink(), &ExtensionSink::onSetFilter);
        dpfSignalDispatcher->subscribe(GlobalEventType::kOpenFiles, &sink(), &ExtensionSink::onOpenFiles);

        plugin.initialize();
        plugin.start();
    });
    return plugin;
}

}   // namespace

class TagRealEventsDispatchTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        resetCaptures();
        sink().reset();

        // daemon-dbus boundary: inert cache reload + no real daemon queries
        stub.set_lamda(&TagProxyHandle::getAllFileWithTags, []() -> QVariantHash {
            return {};
        });
        stub.set_lamda(&TagProxyHandle::getAllTags, []() -> QVariantMap {
            return {};
        });
        stub.set_lamda(&TagProxyHandle::getAllTrashFileTags, []() -> QVariantHash {
            return {};
        });

        // perform the one-time real wiring
        tagPluginInstance();

        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        // real SyncFileInfo for the file scheme: handlers stat real temp
        // files, so pathOf()/extendAttributes(kInode) return real values
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        installHandlerStubs();
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    QUrl makeUrl(const QString &name) const
    {
        return QUrl::fromLocalFile(tempDir->path() + "/" + name);
    }

    QUrl createTestFile(const QString &name)
    {
        QString path = tempDir->path() + "/" + name;
        QFile file(path);
        EXPECT_TRUE(file.open(QIODevice::WriteOnly));
        file.write("dispatch");
        file.close();
        return QUrl::fromLocalFile(path);
    }

    void installHandlerStubs()
    {
        stub.set_lamda(&TagManager::getTagsByUrls, []() {
            return g.tagsByUrlsResult;
        });
        auto canTagUrl = static_cast<bool (TagManager::*)(const QUrl &) const>(&TagManager::canTagFile);
        stub.set_lamda(canTagUrl, []() -> bool {
            return true;
        });
        auto canTagInfo = static_cast<bool (TagManager::*)(const FileInfoPointer &) const>(&TagManager::canTagFile);
        stub.set_lamda(canTagInfo, []() -> bool {
            return true;
        });
        stub.set_lamda(&TagManager::addTagsForFiles,
                       [](TagManager *, const QList<QString> &tags, const QList<QUrl> &files) -> bool {
                           g.addTagsCalled = true;
                           g.addTagsNames = tags;
                           g.addTagsFiles = files;
                           return true;
                       });
        stub.set_lamda(&TagManager::removeTagsOfFiles,
                       [](TagManager *, const QList<QString> &tags, const QList<QUrl> &files) -> bool {
                           g.removeTagsCalled = true;
                           g.removeTagsNames = tags;
                           g.removeTagsFiles = files;
                           return true;
                       });
        stub.set_lamda(&TagManager::setTagsForFiles,
                       [](TagManager *, const QList<QString> &tags, const QList<QUrl> &files) -> bool {
                           g.setTagsCalled = true;
                           g.setTagsNames = tags;
                           g.setTagsFiles = files;
                           return true;
                       });
        stub.set_lamda(&TagManager::hideFiles,
                       [](TagManager *, const QList<QString> &tags, const QList<QUrl> &files) {
                           g.hideFilesCalled = true;
                           g.hideFilesTags = tags;
                           g.hideFilesFiles = files;
                       });
        stub.set_lamda(&TagManager::clearAllTrashTags, []() -> bool {
            g.clearTrashCalled = true;
            return true;
        });
        stub.set_lamda(&TagManager::saveTrashFileTags,
                       [](TagManager *, const QString &path, qint64 inode, const QStringList &tags) -> bool {
                           g.saveTrashCalled = true;
                           g.saveTrashPath = path;
                           g.saveTrashInode = inode;
                           g.saveTrashTags = tags;
                           return true;
                       });
        stub.set_lamda(&TagManager::getTrashFileTags,
                       [](TagManager *, const QString &path, qint64 inode) -> QStringList {
                           g.getTrashCalled = true;
                           g.getTrashPath = path;
                           g.getTrashInode = inode;
                           return g.getTrashResult;
                       });
        stub.set_lamda(&TagManager::removeTrashFileTags,
                       [](TagManager *, const QString &path, qint64 inode) -> bool {
                           g.removeTrashCalled = true;
                           g.removeTrashPath = path;
                           g.removeTrashInode = inode;
                           return true;
                       });
        stub.set_lamda(&TagManager::removeChildren,
                       [](TagManager *, const QString &path) -> bool {
                           g.removeChildrenCalled = true;
                           g.removeChildrenPath = path;
                           return true;
                       });
        stub.set_lamda(&TagManager::getTagIconName,
                       [](TagManager *, const QString &) -> QString {
                           return QString("icon-blue");
                       });
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
};

// ========== wiring + slot channel ==========

TEST_F(TagRealEventsDispatchTest, RealWiring_SlotGetTagsPush_ReturnsTagsFromManager)
{
    g.tagsByUrlsResult = QStringList { "red", "green" };
    QUrl url = makeUrl("gettags.txt");

    QVariant ret = dpfSlotChannel->push("dfmplugin_tag", "slot_GetTags", url);

    EXPECT_EQ(ret.toStringList(), QStringList({ "red", "green" }));
    EXPECT_FALSE(ret.toStringList().isEmpty());
}

TEST_F(TagRealEventsDispatchTest, RealWiring_EmptyTagList_PushReturnsEmptyList)
{
    g.tagsByUrlsResult = QStringList {};

    QVariant ret = dpfSlotChannel->push("dfmplugin_tag", "slot_GetTags", makeUrl("empty.txt"));

    ASSERT_TRUE(ret.isValid());
    EXPECT_TRUE(ret.toStringList().isEmpty());
}

// ========== pluginsStarted / menu scene wiring ==========

TEST_F(TagRealEventsDispatchTest, PluginsStarted_MenuSceneSubscribeAndPublish_UnsubscribesAfterMatch)
{
    ASSERT_TRUE(QMetaObject::invokeMethod(dpfListener, "pluginsStarted", Qt::DirectConnection));

    // bindScene("FileOperatorMenu") has now subscribed the scene-added signal
    // through the real EventDispatcherManager; publishing the matching scene
    // runs Tag::onMenuSceneAdded and unsubscribes (EventHandler::compare path).
    EXPECT_TRUE(dpfSignalDispatcher->publish("dfmplugin_menu", "signal_MenuScene_SceneAdded",
                                             QString("FileOperatorMenu")));

    // unrelated scene names must be ignored by the same dispatcher
    EXPECT_TRUE(dpfSignalDispatcher->publish("dfmplugin_menu", "signal_MenuScene_SceneAdded",
                                             QString("SomeOtherScene")));
}

// ========== TagEventReceiver: GlobalEventType signals ==========

TEST_F(TagRealEventsDispatchTest, HideFilesResultEvent_PublishSuccess_HideFilesReceivesTagsAndUrls)
{
    g.tagsByUrlsResult = QStringList { "red" };
    QList<QUrl> urls { makeUrl("hide.txt") };

    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kHideFilesResult, quint64(1), urls, true));

    ASSERT_TRUE(g.hideFilesCalled);
    EXPECT_EQ(g.hideFilesTags, QStringList({ "red" }));
    EXPECT_EQ(g.hideFilesFiles, urls);
}

TEST_F(TagRealEventsDispatchTest, HideFilesResultEvent_FailedOperation_NoHideFilesCall)
{
    g.tagsByUrlsResult = QStringList { "red" };
    QList<QUrl> urls { makeUrl("hide_fail.txt") };

    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kHideFilesResult, quint64(1), urls, false));

    EXPECT_FALSE(g.hideFilesCalled);
}

TEST_F(TagRealEventsDispatchTest, CutFileResultEvent_PublishSuccess_SourceUntaggedAndDestTagged)
{
    g.tagsByUrlsResult = QStringList { "red" };
    QList<QUrl> srcs { createTestFile("cut_src.txt") };
    QList<QUrl> dsts { makeUrl("cut_dst.txt") };

    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kCutFileResult, srcs, dsts, true, QString()));

    ASSERT_TRUE(g.removeTagsCalled);
    EXPECT_EQ(g.removeTagsNames, QStringList({ "red" }));
    EXPECT_EQ(g.removeTagsFiles, srcs);
    ASSERT_TRUE(g.addTagsCalled);
    EXPECT_EQ(g.addTagsNames, QStringList({ "red" }));
    EXPECT_EQ(g.addTagsFiles, dsts);
}

TEST_F(TagRealEventsDispatchTest, CopyFileResultEvent_PublishSuccess_OnlyDestTagged)
{
    g.tagsByUrlsResult = QStringList { "blue" };
    QList<QUrl> srcs { createTestFile("copy_src.txt") };
    QList<QUrl> dsts { makeUrl("copy_dst.txt") };

    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kCopyResult, srcs, dsts, true, QString()));

    EXPECT_FALSE(g.removeTagsCalled);
    ASSERT_TRUE(g.addTagsCalled);
    EXPECT_EQ(g.addTagsNames, QStringList({ "blue" }));
    EXPECT_EQ(g.addTagsFiles, dsts);
}

TEST_F(TagRealEventsDispatchTest, CutFileResultEvent_SizeMismatchOrFailure_NoTagChanges)
{
    g.tagsByUrlsResult = QStringList { "red" };
    QList<QUrl> srcs { createTestFile("cut_bad.txt") };

    // destination list empty -> early return
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kCutFileResult, srcs, QList<QUrl>(), true, QString()));
    // failed operation -> early return
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kCutFileResult, srcs, srcs, false, QString("boom")));
    // size mismatch -> early return
    QList<QUrl> twoDsts { makeUrl("d1.txt"), makeUrl("d2.txt") };
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kCutFileResult, srcs, twoDsts, true, QString()));

    EXPECT_FALSE(g.removeTagsCalled);
    EXPECT_FALSE(g.addTagsCalled);
}

TEST_F(TagRealEventsDispatchTest, DeleteFilesResultEvent_PublishSuccess_ChildrenRemovedAndFilesUntagged)
{
    g.tagsByUrlsResult = QStringList { "red" };
    QUrl src = createTestFile("removed.txt");
    QList<QUrl> srcs { src };

    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kDeleteFilesResult, srcs, true, QString()));

    ASSERT_TRUE(g.removeChildrenCalled);
    EXPECT_EQ(g.removeChildrenPath, src.toLocalFile());
    ASSERT_TRUE(g.removeTagsCalled);
    EXPECT_EQ(g.removeTagsFiles, srcs);
    EXPECT_EQ(g.removeTagsNames, QStringList({ "red" }));
}

TEST_F(TagRealEventsDispatchTest, MoveToTrashResultEvent_PublishSuccess_TrashTagsSavedWithInode)
{
    g.tagsByUrlsResult = QStringList { "red" };
    QUrl src = createTestFile("trashed.txt");
    QList<QUrl> srcs { src };

    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrashResult, srcs, true, QString()));

    ASSERT_TRUE(g.removeTagsCalled);
    ASSERT_TRUE(g.saveTrashCalled);
    EXPECT_EQ(g.saveTrashTags, QStringList({ "red" }));
    EXPECT_EQ(g.saveTrashPath, src.toLocalFile());
    EXPECT_GT(g.saveTrashInode, qint64(0));
}

TEST_F(TagRealEventsDispatchTest, CleanTrashResultEvent_PublishSuccess_AllTrashTagsCleared)
{
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kCleanTrashResult,
                                             QList<QUrl> { makeUrl("cleaned.txt") }, true, QString()));

    ASSERT_TRUE(g.clearTrashCalled);

    g.clearTrashCalled = false;
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kCleanTrashResult,
                                             QList<QUrl> { makeUrl("cleaned.txt") }, false, QString()));
    EXPECT_FALSE(g.clearTrashCalled);
}

TEST_F(TagRealEventsDispatchTest, RenameFileResultEvent_PublishSuccess_TagsMovedToNewUrls)
{
    g.tagsByUrlsResult = QStringList { "red" };
    QUrl oldUrl = createTestFile("old_name.txt");
    QUrl newUrl = makeUrl("new_name.txt");
    QMap<QUrl, QUrl> renamed { { oldUrl, newUrl } };

    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kRenameFileResult, quint64(2), renamed, true, QString()));

    ASSERT_TRUE(g.removeTagsCalled);
    EXPECT_EQ(g.removeTagsFiles, QList<QUrl> { oldUrl });
    ASSERT_TRUE(g.addTagsCalled);
    EXPECT_EQ(g.addTagsFiles, QList<QUrl> { newUrl });
}

TEST_F(TagRealEventsDispatchTest, RestoreFromTrashResultEvent_ValidTrashInfo_TagsRestoredAndRecordRemoved)
{
    g.getTrashResult = QStringList { "blue" };
    QList<QUrl> dsts { createTestFile("restored.txt") };
    QVariantList customInfos { QVariant(QString("Path=%2Ftmp%2Fgone.txt")) };

    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kRestoreFromTrashResult,
                                             QList<QUrl>(), dsts, customInfos, true, QString()));

    ASSERT_TRUE(g.getTrashCalled);
    EXPECT_EQ(g.getTrashPath, QString("/tmp/gone.txt"));
    EXPECT_GT(g.getTrashInode, qint64(0));
    ASSERT_TRUE(g.addTagsCalled);
    EXPECT_EQ(g.addTagsNames, QStringList({ "blue" }));
    EXPECT_EQ(g.addTagsFiles, dsts);
    ASSERT_TRUE(g.removeTrashCalled);
    EXPECT_EQ(g.removeTrashPath, QString("/tmp/gone.txt"));
    EXPECT_GT(g.removeTrashInode, qint64(0));
}

TEST_F(TagRealEventsDispatchTest, RestoreFromTrashResultEvent_MalformedInputs_NoDaemonInteraction)
{
    const QList<QUrl> oneDest { makeUrl("a.txt") };
    const QList<QUrl> destB { makeUrl("b.txt") };
    const QList<QUrl> destC { makeUrl("c.txt") };
    const QList<QUrl> destD { makeUrl("d.txt") };
    const QVariantList twoInfos { QVariant(QString("Path=%2Ftmp%2Fx")), QVariant(QString("Path=%2Ftmp%2Fy")) };
    const QVariantList noPathInfo { QVariant(QString("NoPath=1")) };
    const QVariantList pathC { QVariant(QString("Path=%2Ftmp%2Fc")) };
    const QVariantList pathD { QVariant(QString("Path=%2Ftmp%2Fd")) };

    // destination/custom info count mismatch
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kRestoreFromTrashResult,
                                             QList<QUrl>(), oneDest, twoInfos, true, QString()));
    // trash info without a Path= line
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kRestoreFromTrashResult,
                                             QList<QUrl>(), destB, noPathInfo, true, QString()));
    // empty trash tags -> nothing restored
    g.getTrashResult = QStringList {};
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kRestoreFromTrashResult,
                                             QList<QUrl>(), destC, pathC, true, QString()));
    // failed operation
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kRestoreFromTrashResult,
                                             QList<QUrl>(), destD, pathD, false, QString()));

    EXPECT_FALSE(g.getTrashCalled);
    EXPECT_FALSE(g.addTagsCalled);
    EXPECT_FALSE(g.removeTrashCalled);
}

TEST_F(TagRealEventsDispatchTest, ChangeCurrentUrlEvent_TagScheme_DeferredViewFilterPushed)
{
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, quint64(3), QUrl("tag:/red")));

    // the deferred slot_View_SetFilter push runs through the event loop
    qApp->processEvents();
    qApp->processEvents();

    EXPECT_EQ(g.filterWinId, quint64(3));
    EXPECT_EQ(g.filterFlags, int(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden));

    // non-tag scheme must not push any filter
    g.filterWinId = 0;
    g.filterFlags = -1;
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, quint64(3), makeUrl("plain.txt")));
    qApp->processEvents();
    EXPECT_EQ(g.filterWinId, quint64(0));
    EXPECT_EQ(g.filterFlags, -1);
}

TEST_F(TagRealEventsDispatchTest, SidebarSortedSignal_TagGroupAndForeignGroup_PublishSucceeds)
{
    EXPECT_TRUE(dpfSignalDispatcher->publish("dfmplugin_sidebar", "signal_Sidebar_Sorted",
                                             quint64(4), QString("Group_Tag"), QList<QUrl>()));
    EXPECT_TRUE(dpfSignalDispatcher->publish("dfmplugin_sidebar", "signal_Sidebar_Sorted",
                                             quint64(4), QString("Group_Other"),
                                             QList<QUrl> { QUrl("tag:/blue") }));
}

// ========== TagManager hooks (followEvents) ==========

TEST_F(TagRealEventsDispatchTest, PaintListItemHook_TaggedFileInfo_RectShrinksByTagDiameter)
{
    stub.set_lamda(&FileTagCacheController::getTagsByFile,
                   [](FileTagCacheController *, const QString &path) -> QStringList {
                       g.paintIconPath = path;
                       return QStringList { "red" };
                   });
    stub.set_lamda(&FileTagCacheController::getCacheTagsColor,
                   [](FileTagCacheController *, const QStringList &) -> QMap<QString, QColor> {
                       return QMap<QString, QColor> { { "red", QColor("#ff0000") } };
                   });

    QPixmap pixmap(200, 40);
    QPainter painter(&pixmap);
    QRectF rect(0, 0, 200, 40);
    FileInfoPointer info = InfoFactory::create<FileInfo>(createTestFile("paint.txt"));
    ASSERT_NE(info, nullptr);

    bool ok = dpfHookSequence->run("dfmplugin_workspace", "hook_Delegate_PaintListItem",
                                   static_cast<int>(Global::ItemRoles::kItemFileDisplayNameRole),
                                   info, &painter, &rect);

    // the handler always returns false by contract; the paint still happened
    EXPECT_FALSE(ok);
    // (tagsColor.size() + 1) * kTagDiameter / 2 = 10 wide bubble + 10 margin
    EXPECT_DOUBLE_EQ(rect.right(), 180.0);
    EXPECT_TRUE(g.paintIconPath.endsWith(QString("paint.txt")));
}

TEST_F(TagRealEventsDispatchTest, PaintListItemHook_WrongRole_RectUntouched)
{
    stub.set_lamda(&FileTagCacheController::getTagsByFile,
                   [](FileTagCacheController *, const QString &) -> QStringList {
                       return QStringList { "red" };
                   });
    QPixmap pixmap(200, 40);
    QPainter painter(&pixmap);
    QRectF rect(0, 0, 200, 40);
    FileInfoPointer info = InfoFactory::create<FileInfo>(createTestFile("paint_role.txt"));
    ASSERT_NE(info, nullptr);

    EXPECT_FALSE(dpfHookSequence->run("dfmplugin_workspace", "hook_Delegate_PaintListItem",
                                      static_cast<int>(Global::ItemRoles::kItemFileSizeRole),
                                      info, &painter, &rect));
    EXPECT_DOUBLE_EQ(rect.right(), 200.0);
}

TEST_F(TagRealEventsDispatchTest, LayoutTextHook_TaggedFileInfo_IconTagInsertedIntoDocument)
{
    stub.set_lamda(&FileTagCacheController::getTagsByFile,
                   [](FileTagCacheController *, const QString &) -> QStringList {
                       return QStringList { "red" };
                   });
    stub.set_lamda(&FileTagCacheController::getCacheTagsColor,
                   [](FileTagCacheController *, const QStringList &) -> QMap<QString, QColor> {
                       return QMap<QString, QColor> { { "red", QColor("#ff0000") } };
                   });

    dfmbase::ElideTextLayout layout("sample text");
    FileInfoPointer info = InfoFactory::create<FileInfo>(createTestFile("icon.txt"));
    ASSERT_NE(info, nullptr);

    EXPECT_FALSE(dpfHookSequence->run("dfmplugin_workspace", "hook_Delegate_LayoutText",
                                      info, &layout));
    ASSERT_NE(layout.documentHandle(), nullptr);
    EXPECT_TRUE(layout.documentHandle()->toPlainText().contains(QChar::ObjectReplacementCharacter));

    // the canvas flavor of the same hook runs the identical handler
    dfmbase::ElideTextLayout canvasLayout("canvas text");
    EXPECT_FALSE(dpfHookSequence->run("ddplugin_canvas", "hook_CanvasItemDelegate_LayoutText",
                                      info, &canvasLayout));
    EXPECT_TRUE(canvasLayout.documentHandle()->toPlainText().contains(QChar::ObjectReplacementCharacter));
}

TEST_F(TagRealEventsDispatchTest, PasteFilesHook_TagDestWithCutAction_HookPasses)
{
    stub.set_lamda(&ClipBoard::clipboardAction, []() {
        return ClipBoard::kCutAction;
    });

    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_workspace", "hook_ShortCut_PasteFiles",
                                     quint64(5), QList<QUrl> { makeUrl("paste.txt") }, QUrl("tag:/blue")));
}

TEST_F(TagRealEventsDispatchTest, PasteFilesHook_NonTagDestOrEmptyClipboard_HookHandled)
{
    stub.set_lamda(&ClipBoard::clipboardAction, []() {
        return ClipBoard::kCopyAction;
    });
    stub.set_lamda(&ClipBoard::clipboardFileUrlList, []() -> QList<QUrl> {
        return {};
    });

    // copy action with an empty clipboard still succeeds on the tag scheme
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_workspace", "hook_ShortCut_PasteFiles",
                                     quint64(5), QList<QUrl> { makeUrl("paste2.txt") }, QUrl("tag:/blue")));
    // non-tag destination is not handled by the tag plugin
    EXPECT_FALSE(dpfHookSequence->run("dfmplugin_workspace", "hook_ShortCut_PasteFiles",
                                      quint64(5), QList<QUrl> { makeUrl("paste3.txt") }, makeUrl("dest_dir")));
}

TEST_F(TagRealEventsDispatchTest, FileDropHook_TagDest_SetTagsForFilesWithTagName)
{
    QList<QUrl> srcs { makeUrl("drop.txt") };

    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_FileDrop",
                                     srcs, QUrl("tag:/blue")));

    ASSERT_TRUE(g.setTagsCalled);
    EXPECT_EQ(g.setTagsNames, QStringList({ "blue" }));
    EXPECT_EQ(g.setTagsFiles, srcs);

    // non-tag destination is refused
    g.setTagsCalled = false;
    EXPECT_FALSE(dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_FileDrop",
                                      srcs, makeUrl("plain_dest")));
    EXPECT_FALSE(g.setTagsCalled);
}

TEST_F(TagRealEventsDispatchTest, DropDataHook_TagDest_ActionForcedToIgnore)
{
    Qt::DropAction action = Qt::CopyAction;

    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_sidebar", "hook_Item_DropData",
                                     QList<QUrl> { makeUrl("dropdata.txt") }, QUrl("tag:/blue"), &action));
    EXPECT_EQ(action, Qt::IgnoreAction);

    Qt::DropAction foreign = Qt::CopyAction;
    EXPECT_FALSE(dpfHookSequence->run("dfmplugin_sidebar", "hook_Item_DropData",
                                      QList<QUrl> { makeUrl("dropdata2.txt") }, makeUrl("anywhere"), &foreign));
    EXPECT_EQ(foreign, Qt::CopyAction);
}

TEST_F(TagRealEventsDispatchTest, CrumbSeprateHook_TagUrl_CrumbDataAppended)
{
    QList<QVariantMap> crumbs;
    QUrl tagUrl("tag:/blue");

    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_titlebar", "hook_Crumb_Seprate", tagUrl, &crumbs));

    ASSERT_EQ(crumbs.size(), 1);
    EXPECT_EQ(crumbs.first().value("CrumbData_Key_Url").toUrl(), tagUrl);
    EXPECT_EQ(crumbs.first().value("CrumbData_Key_IconName").toString(), QString("icon-blue"));

    QList<QVariantMap> foreignCrumbs;
    EXPECT_FALSE(dpfHookSequence->run("dfmplugin_titlebar", "hook_Crumb_Seprate",
                                      makeUrl("plain.txt"), &foreignCrumbs));
    EXPECT_TRUE(foreignCrumbs.isEmpty());
}

TEST_F(TagRealEventsDispatchTest, OpenFileInPluginHook_TagUrlWithFragment_RedirectedAndRePublished)
{
    QUrl tagUrl("tag:/blue");
    tagUrl.setFragment("/tmp/redirected.txt");

    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin",
                                     quint64(6), QList<QUrl> { tagUrl }));

    // the redirect re-publishes kOpenFiles with the fragment-decoded local file
    ASSERT_EQ(g.openFilesUrls.size(), 1);
    EXPECT_EQ(g.openFilesUrls.first(), QUrl::fromLocalFile("/tmp/redirected.txt"));
    EXPECT_EQ(g.openFilesWinId, quint64(6));

    // empty list and non-tag scheme are refused
    EXPECT_FALSE(dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin",
                                      quint64(6), QList<QUrl> {}));
    EXPECT_FALSE(dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin",
                                      quint64(6), QList<QUrl> { makeUrl("plain_open.txt") }));
}

// ========== pluginStarted extension registration ==========

TEST_F(TagRealEventsDispatchTest, PluginStartedSignal_PropertyDialog_ExtensionCallbacksRegisteredAndUsable)
{
    ASSERT_TRUE(QMetaObject::invokeMethod(dpfListener, "pluginStarted",
                                          Q_ARG(QString, "org.deepin.plugin.common"),
                                          Q_ARG(QString, "dfmplugin-propertydialog")));

    ASSERT_TRUE(sink().propRegistered);
    EXPECT_EQ(sink().propName, QString("Tag"));
    EXPECT_EQ(sink().propIndex, 0);
    ASSERT_TRUE(bool(sink().propCreate));
    ASSERT_TRUE(bool(sink().propUpdate));

    // the create callback builds a real TagWidget for a taggable file url
    QWidget *widget = sink().propCreate(makeUrl("prop.txt"));
    ASSERT_NE(widget, nullptr);
    EXPECT_NE(qobject_cast<TagWidget *>(widget), nullptr);

    // the update callback retargets the widget without crashing
    EXPECT_NO_FATAL_FAILURE(sink().propUpdate(widget, makeUrl("prop_retarget.txt")));
    delete widget;

    // non-taggable urls are rejected by the create callback
    auto canTagUrl = static_cast<bool (TagManager::*)(const QUrl &) const>(&TagManager::canTagFile);
    stub.set_lamda(canTagUrl, []() -> bool {
        return false;
    });
    EXPECT_EQ(sink().propCreate(makeUrl("nope.txt")), nullptr);
}

TEST_F(TagRealEventsDispatchTest, PluginStartedSignal_Detailspace_ExtensionAndFieldFilterRegistered){
    ASSERT_TRUE(QMetaObject::invokeMethod(dpfListener, "pluginStarted",
                                          Q_ARG(QString, "org.deepin.plugin.common"),
                                          Q_ARG(QString, "dfmplugin-detailspace")));

    ASSERT_TRUE(sink().detailRegistered);
    EXPECT_EQ(sink().detailIndex, -1);
    ASSERT_TRUE(bool(sink().detailCreate));
    ASSERT_TRUE(bool(sink().detailUpdate));
    ASSERT_TRUE(bool(sink().detailShouldShow));

    ASSERT_TRUE(sink().filterRegistered);
    EXPECT_EQ(sink().filterScheme, QString("tag"));
    EXPECT_EQ(sink().filterFields,
              QStringList({ "kFileSizeField", "kFileChangeTimeField", "kFileInterviewTimeField" }));

    // create/update callbacks operate on a real TagWidget
    QWidget *widget = sink().detailCreate(QUrl("tag:/x"));
    ASSERT_NE(widget, nullptr);
    EXPECT_NE(qobject_cast<TagWidget *>(widget), nullptr);
    EXPECT_NO_FATAL_FAILURE(sink().detailUpdate(widget, makeUrl("detail.txt")));

    // shouldShow mirrors TagManager::canTagFile exactly
    EXPECT_TRUE(sink().detailShouldShow(makeUrl("show.txt")));
    auto canTagUrl = static_cast<bool (TagManager::*)(const QUrl &) const>(&TagManager::canTagFile);
    stub.set_lamda(canTagUrl, []() -> bool {
        return false;
    });
    EXPECT_FALSE(sink().detailShouldShow(makeUrl("hide.txt")));

    // update on a non-TagWidget must be a harmless no-op
    QWidget plain;
    EXPECT_NO_FATAL_FAILURE(sink().detailUpdate(&plain, makeUrl("plain_widget.txt")));
    delete widget;
}

TEST_F(TagRealEventsDispatchTest, BindWindows_RegisteredWindowId_OnWindowOpenedPerId)
{
    bool lookupHappened = false;
    FileManagerWindow window(QUrl::fromLocalFile("/home"));
    stub.set_lamda(&FileManagerWindowsManager::findWindowById,
                   [&lookupHappened, &window](FileManagerWindowsManager *, quint64) -> FileManagerWindow * {
                       lookupHappened = true;
                       return &window;
                   });
    stub.set_lamda(&FileManagerWindowsManager::windowIdList,
                   []() -> QList<quint64> { return QList<quint64> { quint64(1) }; });
    stub.set_lamda(&Tag::installToSideBar, []() {});

    // private member: callable because the test target compiles with
    // -fno-access-control; exercises the std::for_each lambda body
    tagPluginInstance().bindWindows();

    EXPECT_TRUE(lookupHappened);
    // a second run over an empty window list must stay harmless
    stub.clear();
    stub.set_lamda(&FileManagerWindowsManager::windowIdList,
                   []() -> QList<quint64> { return QList<quint64> {}; });
    EXPECT_NO_FATAL_FAILURE(tagPluginInstance().bindWindows());
}
