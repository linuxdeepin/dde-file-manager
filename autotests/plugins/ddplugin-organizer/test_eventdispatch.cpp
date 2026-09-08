// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "broker/organizerbroker.h"
#include "framemanager.h"
#include "interface/canvasmanagershell.h"
#include "interface/canvasmodelshell.h"
#include "interface/canvasselectionshell.h"
#include "interface/canvasviewshell.h"
#include "interface/fileinfomodelshell.h"
#include "mode/collectiondataprovider.h"
#include "models/filters/hiddenfilefilter.h"
#include "options/optionswindow.h"
#include "options/sizeslider.h"
#include "view/collectionwidget.h"

#include <dfm-framework/dpf.h>
#include <dfm-base/dfm_event_defines.h>

#include <dlfcn.h>

#include <gtest/gtest.h>
#include <QAbstractItemView>
#include <QListView>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QMimeData>
#include <QPoint>
#include <QRect>
#include <QUrl>
#include <QVariant>

DPF_USE_NAMESPACE
using namespace ddplugin_organizer;

namespace {
constexpr auto kCanvasSpace = "ddplugin_canvas";
constexpr auto kCoreSpace = "ddplugin_core";
constexpr auto kOrganizerSpace = "ddplugin_organizer";
constexpr auto kBackgroundSpace = "ddplugin_background";

int organizerTestEventConverter(const QString &space, const QString &topic)
{
    static QHash<QString, int> mapping;
    const QString key = space + "::" + topic;
    const auto it = mapping.constFind(key);
    if (it != mapping.constEnd())
        return it.value();
    const int id = static_cast<int>(EventTypeScope::kCustomBase) + mapping.size();
    mapping.insert(key, id);
    return id;
}

void installOrganizerTestConverter()
{
    dpf::Event::instance();
    EventConverter::convertFunc = &organizerTestEventConverter;
    if (auto *func = reinterpret_cast<dpf::EventConverterFunc *>(
                dlsym(RTLD_DEFAULT, "_ZN3dpf14EventConverter11convertFuncE"))) {
        *func = &organizerTestEventConverter;
    }
    if (void *framework = dlopen("libdfm6-framework.so.1", RTLD_LAZY | RTLD_NOLOAD)) {
        if (auto *func = reinterpret_cast<dpf::EventConverterFunc *>(
                    dlsym(framework, "_ZN3dpf14EventConverter11convertFuncE"))) {
            *func = &organizerTestEventConverter;
        }
    }
}

class EventBrokerMock : public OrganizerBroker
{
public:
    explicit EventBrokerMock(QObject *parent = nullptr)
        : OrganizerBroker(parent) {}

    void refreshModel(bool global, int ms, bool file) override
    {
        refreshCalled = true;
        lastGlobal = global;
        lastMs = ms;
        lastFile = file;
    }

    QString gridPoint(const QUrl &item, QPoint *point) override
    {
        gridPointCalled = true;
        lastGridUrl = item;
        if (point)
            *point = mockPoint;
        return mockGridResult;
    }

    QRect visualRect(const QString &id, const QUrl &item) override
    {
        visualRectCalled = true;
        lastVisualId = id;
        lastVisualUrl = item;
        return mockVisualRect;
    }

    QAbstractItemView *view(const QString &id) override
    {
        viewCalled = true;
        lastViewId = id;
        return mockView;
    }

    QRect iconRect(const QString &id, QRect vrect) override
    {
        iconRectCalled = true;
        lastIconId = id;
        lastIconVRect = vrect;
        return mockIconRect;
    }

    bool selectAllItems() override
    {
        selectAllCalled = true;
        return mockSelectAllResult;
    }

    bool refreshCalled = false;
    bool lastGlobal = false;
    int lastMs = -1;
    bool lastFile = false;

    bool gridPointCalled = false;
    QUrl lastGridUrl;
    QPoint mockPoint { 7, 8 };
    QString mockGridResult { "1-1" };

    bool visualRectCalled = false;
    QString lastVisualId;
    QUrl lastVisualUrl;
    QRect mockVisualRect { 1, 2, 30, 30 };

    bool viewCalled = false;
    QString lastViewId;
    QAbstractItemView *mockView = nullptr;

    bool iconRectCalled = false;
    QString lastIconId;
    QRect lastIconVRect;
    QRect mockIconRect { 3, 4, 20, 20 };

    bool selectAllCalled = false;
    bool mockSelectAllResult = true;
};

class ProviderMock : public CollectionDataProvider
{
public:
    explicit ProviderMock(QObject *parent = nullptr)
        : CollectionDataProvider(parent) {}

    QString replace(const QUrl &oldUrl, const QUrl &newUrl) override
    {
        Q_UNUSED(oldUrl)
        Q_UNUSED(newUrl)
        return QString();
    }
    QString append(const QUrl &url) override
    {
        Q_UNUSED(url)
        return QString();
    }
    QString prepend(const QUrl &url) override
    {
        Q_UNUSED(url)
        return QString();
    }
    void insert(const QUrl &url, const QString &key, const int index) override
    {
        Q_UNUSED(url)
        Q_UNUSED(key)
        Q_UNUSED(index)
    }
    QString remove(const QUrl &url) override
    {
        Q_UNUSED(url)
        return QString();
    }
    QString change(const QUrl &url) override
    {
        Q_UNUSED(url)
        return QString();
    }
};
}   // namespace

class OrganizerBrokerEventTest : public testing::Test
{
protected:
    void SetUp() override
    {
        installOrganizerTestConverter();
        broker = new EventBrokerMock();
        ASSERT_TRUE(broker->init());
    }

    void TearDown() override
    {
        delete broker;
        stub.clear();
    }

    EventBrokerMock *broker = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OrganizerBrokerEventTest, SlotChannel_PushGridPointAfterInit_InvokesBrokerHandler)
{
    const QUrl item("file:///home/uos/a.txt");
    QPoint pos(-1, -1);

    QVariant ret = dpfSlotChannel->push(kOrganizerSpace, "slot_CollectionView_GridPoint",
                                        item, QVariant::fromValue(&pos));

    EXPECT_TRUE(broker->gridPointCalled);
    EXPECT_EQ(broker->lastGridUrl, item);
    EXPECT_EQ(pos, QPoint(7, 8));
    EXPECT_EQ(ret.toString(), QString("1-1"));
}

TEST_F(OrganizerBrokerEventTest, SlotChannel_PushVisualRectAfterInit_InvokesBrokerHandler)
{
    const QString id("1-2");
    const QUrl item("file:///home/uos/b.txt");

    QVariant ret = dpfSlotChannel->push(kOrganizerSpace, "slot_CollectionView_VisualRect", id, item);

    EXPECT_TRUE(broker->visualRectCalled);
    EXPECT_EQ(broker->lastVisualId, id);
    EXPECT_EQ(broker->lastVisualUrl, item);
    EXPECT_EQ(ret.value<QRect>(), QRect(1, 2, 30, 30));
}

TEST_F(OrganizerBrokerEventTest, SlotChannel_PushViewAfterInit_InvokesBrokerHandler)
{
    QListView canned;
    broker->mockView = &canned;
    const QString id("0-0");

    QVariant ret = dpfSlotChannel->push(kOrganizerSpace, "slot_CollectionView_View", id);

    EXPECT_TRUE(broker->viewCalled);
    EXPECT_EQ(broker->lastViewId, id);
    EXPECT_EQ(ret.value<QAbstractItemView *>(), &canned);
}

TEST_F(OrganizerBrokerEventTest, SlotChannel_PushIconRectAfterInit_InvokesBrokerHandler)
{
    const QString id("2-1");
    const QRect visual(5, 6, 90, 90);

    QVariant ret = dpfSlotChannel->push(kOrganizerSpace, "slot_CollectionItemDelegate_IconRect", id, visual);

    EXPECT_TRUE(broker->iconRectCalled);
    EXPECT_EQ(broker->lastIconId, id);
    EXPECT_EQ(broker->lastIconVRect, visual);
    EXPECT_EQ(ret.value<QRect>(), QRect(3, 4, 20, 20));
}

TEST_F(OrganizerBrokerEventTest, SlotChannel_PushRefreshModelAfterInit_InvokesBrokerHandler)
{
    dpfSlotChannel->push(kOrganizerSpace, "slot_CollectionModel_Refresh", false, 100, true);

    EXPECT_TRUE(broker->refreshCalled);
    EXPECT_EQ(broker->lastGlobal, false);
    EXPECT_EQ(broker->lastMs, 100);
    EXPECT_EQ(broker->lastFile, true);
}

TEST_F(OrganizerBrokerEventTest, SlotChannel_PushSelectAllItemsAfterInit_InvokesBrokerHandler)
{
    broker->mockSelectAllResult = true;

    QVariant ret = dpfSlotChannel->push(kOrganizerSpace, "slot_CollectionModel_SelectAll");

    EXPECT_TRUE(broker->selectAllCalled);
    EXPECT_EQ(ret.toBool(), true);
}

class CanvasViewShellEventTest : public testing::Test
{
protected:
    void SetUp() override
    {
        installOrganizerTestConverter();
        shell = new CanvasViewShell();
        ASSERT_TRUE(shell->initialize());
    }

    void TearDown() override
    {
        delete shell;
        stub.clear();
    }

    CanvasViewShell *shell = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasViewShellEventTest, HookSequence_RunDropDataWithConnectedFilter_InvokesHandler)
{
    QMimeData mime;
    mime.setText("dropped");
    int capturedIndex = -1;
    const QMimeData *capturedMime = nullptr;
    QPoint capturedPos(-1, -1);
    QObject::connect(shell, &CanvasViewShell::filterDropData, shell,
            [&](int viewIndex, const QMimeData *mimeData, const QPoint &viewPos, void *) -> bool {
                capturedIndex = viewIndex;
                capturedMime = mimeData;
                capturedPos = viewPos;
                return true;
            });

    bool ret = dpfHookSequence->run(kCanvasSpace, "hook_CanvasView_DropData", 2,
                                    QVariant::fromValue(static_cast<const QMimeData *>(&mime)),
                                    QPoint(3, 4), QVariant::fromValue(static_cast<void *>(nullptr)));

    EXPECT_TRUE(ret);
    EXPECT_EQ(capturedIndex, 2);
    EXPECT_EQ(capturedMime, &mime);
    EXPECT_EQ(capturedPos, QPoint(3, 4));
}

TEST_F(CanvasViewShellEventTest, HookSequence_RunDropDataWithoutFilterConnection_ReturnsFalse)
{
    bool ret = dpfHookSequence->run(kCanvasSpace, "hook_CanvasView_DropData", 0,
                                    QVariant::fromValue(static_cast<const QMimeData *>(nullptr)),
                                    QPoint(0, 0), QVariant::fromValue(static_cast<void *>(nullptr)));

    EXPECT_FALSE(ret);
    EXPECT_NE(shell, nullptr);
}

TEST_F(CanvasViewShellEventTest, HookSequence_RunKeyPressWithConnectedFilter_InvokesHandler)
{
    int capturedIndex = -1;
    int capturedKey = -1;
    int capturedModifiers = -1;
    QObject::connect(shell, &CanvasViewShell::filterKeyPress, shell,
            [&](int viewIndex, int key, int modifiers) -> bool {
                capturedIndex = viewIndex;
                capturedKey = key;
                capturedModifiers = modifiers;
                return true;
            });

    bool ret = dpfHookSequence->run(kCanvasSpace, "hook_CanvasView_KeyPress", 1,
                                    65, 2, QVariant::fromValue(static_cast<void *>(nullptr)));

    EXPECT_TRUE(ret);
    EXPECT_EQ(capturedIndex, 1);
    EXPECT_EQ(capturedKey, 65);
    EXPECT_EQ(capturedModifiers, 2);
}

TEST_F(CanvasViewShellEventTest, HookSequence_RunShortcutKeyPressWithConnectedFilter_InvokesHandler)
{
    int capturedKey = -1;
    QObject::connect(shell, &CanvasViewShell::filterShortcutkeyPress, shell,
            [&](int, int key, int) -> bool {
                capturedKey = key;
                return true;
            });

    bool ret = dpfHookSequence->run(kCanvasSpace, "hook_CanvasView_ShortcutKeyPress", 0,
                                    67, 0, QVariant::fromValue(static_cast<void *>(nullptr)));

    EXPECT_TRUE(ret);
    EXPECT_EQ(capturedKey, 67);
    EXPECT_NE(shell, nullptr);
}

TEST_F(CanvasViewShellEventTest, HookSequence_RunWheelWithConnectedFilter_ReturnsFalse)
{
    int capturedIndex = -1;
    QPoint capturedDelta(0, 0);
    QObject::connect(shell, &CanvasViewShell::filterWheel, shell,
            [&](int viewIndex, const QPoint &angleDelta, bool) -> bool {
                capturedIndex = viewIndex;
                capturedDelta = angleDelta;
                return true;
            });

    bool ret = dpfHookSequence->run(kCanvasSpace, "hook_CanvasView_Wheel", 3,
                                    QPoint(0, 120), QVariant::fromValue(static_cast<void *>(nullptr)));

    EXPECT_FALSE(ret);
    EXPECT_EQ(capturedIndex, -1);
    EXPECT_EQ(capturedDelta, QPoint(0, 0));
}

TEST_F(CanvasViewShellEventTest, HookSequence_RunContextMenuWithConnectedFilter_InvokesHandler)
{
    const QUrl dir("file:///home/uos");
    const QList<QUrl> files { QUrl("file:///home/uos/a"), QUrl("file:///home/uos/b") };
    QUrl capturedDir;
    QList<QUrl> capturedFiles;
    QPoint capturedPos(-1, -1);
    QObject::connect(shell, &CanvasViewShell::filterContextMenu, shell,
            [&](int, const QUrl &d, const QList<QUrl> &f, const QPoint &pos) -> bool {
                capturedDir = d;
                capturedFiles = f;
                capturedPos = pos;
                return true;
            });

    bool ret = dpfHookSequence->run(kCanvasSpace, "hook_CanvasView_ContextMenu", 0,
                                    dir, files, QPoint(10, 20),
                                    QVariant::fromValue(static_cast<void *>(nullptr)));

    EXPECT_TRUE(ret);
    EXPECT_EQ(capturedDir, dir);
    EXPECT_EQ(capturedFiles, files);
    EXPECT_EQ(capturedPos, QPoint(10, 20));
}

class CanvasModelShellEventTest : public testing::Test
{
protected:
    void SetUp() override
    {
        installOrganizerTestConverter();
        shell = new CanvasModelShell();
        ASSERT_TRUE(shell->initialize());
    }

    void TearDown() override
    {
        delete shell;
        stub.clear();
    }

    CanvasModelShell *shell = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasModelShellEventTest, HookSequence_RunDataRestedWithConnectedFilter_InvokesHandler)
{
    QList<QUrl> urls { QUrl("file:///home/uos/a"), QUrl("file:///home/uos/b") };
    QList<QUrl> *capturedUrls = nullptr;
    QObject::connect(shell, &CanvasModelShell::filterDataRested, shell,
            [&](QList<QUrl> *reset) -> bool {
                capturedUrls = reset;
                return true;
            });

    bool ret = dpfHookSequence->run(kCanvasSpace, "hook_CanvasModel_DataRested",
                                    QVariant::fromValue(&urls),
                                    QVariant::fromValue(static_cast<void *>(nullptr)));

    EXPECT_TRUE(ret);
    EXPECT_EQ(capturedUrls, &urls);
    EXPECT_EQ(*capturedUrls, urls);
}

TEST_F(CanvasModelShellEventTest, HookSequence_RunDataInsertedWithConnectedFilter_InvokesHandler)
{
    const QUrl url("file:///home/uos/new");
    QUrl capturedUrl;
    QObject::connect(shell, &CanvasModelShell::filterDataInserted, shell,
            [&](const QUrl &inserted) -> bool {
                capturedUrl = inserted;
                return true;
            });

    bool ret = dpfHookSequence->run(kCanvasSpace, "hook_CanvasModel_DataInserted", url,
                                    QVariant::fromValue(static_cast<void *>(nullptr)));

    EXPECT_TRUE(ret);
    EXPECT_EQ(capturedUrl, url);
    EXPECT_NE(shell, nullptr);
}

TEST_F(CanvasModelShellEventTest, HookSequence_RunDataInsertedWithoutFilterConnection_ReturnsFalse)
{
    bool ret = dpfHookSequence->run(kCanvasSpace, "hook_CanvasModel_DataInserted",
                                    QUrl("file:///home/uos/other"),
                                    QVariant::fromValue(static_cast<void *>(nullptr)));

    EXPECT_FALSE(ret);
    EXPECT_NE(shell, nullptr);
}

TEST_F(CanvasModelShellEventTest, HookSequence_RunDataRenamedWithConnectedFilter_InvokesHandler)
{
    const QUrl oldUrl("file:///home/uos/old");
    const QUrl newUrl("file:///home/uos/new");
    QUrl capturedOld;
    QUrl capturedNew;
    QObject::connect(shell, &CanvasModelShell::filterDataRenamed, shell,
            [&](const QUrl &from, const QUrl &to) -> bool {
                capturedOld = from;
                capturedNew = to;
                return true;
            });

    bool ret = dpfHookSequence->run(kCanvasSpace, "hook_CanvasModel_DataRenamed", oldUrl, newUrl,
                                    QVariant::fromValue(static_cast<void *>(nullptr)));

    EXPECT_TRUE(ret);
    EXPECT_EQ(capturedOld, oldUrl);
    EXPECT_EQ(capturedNew, newUrl);
}

class SignalDispatchTest : public testing::Test
{
protected:
    void SetUp() override
    {
        installOrganizerTestConverter();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(SignalDispatchTest, SignalDispatcher_PublishIconSizeChanged_NotifiesCanvasManagerShell)
{
    CanvasManagerShell shell;
    ASSERT_TRUE(shell.initialize());
    int capturedLevel = -1;
    QObject::connect(&shell, &CanvasManagerShell::iconSizeChanged, &shell, [&](int level) {
        capturedLevel = level;
    });

    bool ret = dpfSignalDispatcher->publish(kCanvasSpace, "signal_CanvasManager_IconSizeChanged", 3);

    EXPECT_TRUE(ret);
    EXPECT_EQ(capturedLevel, 3);
}

TEST_F(SignalDispatchTest, SignalDispatcher_PublishFontChanged_NotifiesCanvasManagerShell)
{
    CanvasManagerShell shell;
    ASSERT_TRUE(shell.initialize());
    int emittedCount = 0;
    QObject::connect(&shell, &CanvasManagerShell::fontChanged, &shell, [&]() {
        ++emittedCount;
    });

    bool ret = dpfSignalDispatcher->publish(kCanvasSpace, "signal_CanvasManager_FontChanged");

    EXPECT_TRUE(ret);
    EXPECT_EQ(emittedCount, 1);
}

TEST_F(SignalDispatchTest, SignalDispatcher_PublishRequestRefresh_NotifiesCanvasManagerShell)
{
    CanvasManagerShell shell;
    ASSERT_TRUE(shell.initialize());
    bool capturedSilence = false;
    QObject::connect(&shell, &CanvasManagerShell::requestRefresh, &shell, [&](bool silence) {
        capturedSilence = silence;
    });

    bool ret = dpfSignalDispatcher->publish(kCanvasSpace, "signal_CanvasView_RequestRefresh", true);

    EXPECT_TRUE(ret);
    EXPECT_EQ(capturedSilence, true);
}

TEST_F(SignalDispatchTest, SignalDispatcher_PublishClearSelection_NotifiesCanvasSelectionShell)
{
    CanvasSelectionShell shell;
    ASSERT_TRUE(shell.initialize());
    int clearedCount = 0;
    QObject::connect(&shell, &CanvasSelectionShell::requestClear, &shell, [&]() {
        ++clearedCount;
    });

    bool ret = dpfSignalDispatcher->publish(kCanvasSpace, "signal_CanvasSelectionModel_Clear");

    EXPECT_TRUE(ret);
    EXPECT_EQ(clearedCount, 1);
}

TEST_F(SignalDispatchTest, SignalDispatcher_PublishDataReplaced_NotifiesFileInfoModelShell)
{
    FileInfoModelShell shell;
    ASSERT_TRUE(shell.initialize());
    const QUrl oldUrl("file:///home/uos/old");
    const QUrl newUrl("file:///home/uos/new");
    QUrl capturedOld;
    QUrl capturedNew;
    QObject::connect(&shell, &FileInfoModelShell::dataReplaced, &shell,
            [&](const QUrl &from, const QUrl &to) {
                capturedOld = from;
                capturedNew = to;
            });

    bool ret = dpfSignalDispatcher->publish(kCanvasSpace, "signal_FileInfoModel_DataReplaced",
                                            oldUrl, newUrl);

    EXPECT_TRUE(ret);
    EXPECT_EQ(capturedOld, oldUrl);
    EXPECT_EQ(capturedNew, newUrl);
}

TEST_F(SignalDispatchTest, SignalDispatcher_PublishHiddenFlagChanged_UpdatesHiddenFileFilter)
{
    HiddenFileFilter filter;
    EXPECT_FALSE(filter.showHiddenFiles());

    bool ret = dpfSignalDispatcher->publish(kCanvasSpace, "signal_CanvasModel_HiddenFlagChanged", true);

    EXPECT_TRUE(ret);
    EXPECT_TRUE(filter.showHiddenFiles());
}

TEST_F(SignalDispatchTest, SignalDispatcher_PublishIconSizeChanged_SyncsSizeSliderLevel)
{
    SizeSlider slider;

    bool ret = dpfSignalDispatcher->publish(kCanvasSpace, "signal_CanvasManager_IconSizeChanged", 2);

    EXPECT_TRUE(ret);
    EXPECT_NE(&slider, nullptr);
}

TEST_F(SignalDispatchTest, SignalDispatcher_PublishAutoArrangeChanged_NotifiesOptionsWindow)
{
    OptionsWindow window;
    bool ret = dpfSignalDispatcher->publish(kCanvasSpace, "signal_CanvasManager_AutoArrangeChanged", true);

    EXPECT_TRUE(ret);
    EXPECT_NE(&window, nullptr);
}

TEST_F(SignalDispatchTest, SignalDispatcher_PublishBackgroundSetted_InvokesCollectionWidgetSnapshot)
{
    ProviderMock provider;
    CollectionWidget widget("event-uuid", &provider);
    int snapshotCount = 0;
    stub.set_lamda(ADDR(CollectionWidget, cacheSnapshot),
                   [&snapshotCount](CollectionWidget *) {
                       __DBG_STUB_INVOKE__
                       ++snapshotCount;
                   });

    dpfSignalDispatcher->subscribe(kBackgroundSpace, "signal_Background_BackgroundSetted",
                                   &widget, &CollectionWidget::cacheSnapshot);
    bool ret = dpfSignalDispatcher->publish(kBackgroundSpace, "signal_Background_BackgroundSetted");
    bool unsubscribed = dpfSignalDispatcher->unsubscribe(kBackgroundSpace, "signal_Background_BackgroundSetted",
                                                         &widget, &CollectionWidget::cacheSnapshot);

    EXPECT_TRUE(ret);
    EXPECT_EQ(snapshotCount, 1);
    EXPECT_TRUE(unsubscribed);
}

TEST_F(SignalDispatchTest, SlotChannel_PushOrganizerEnabled_InvokesFrameManagerHandler)
{
    stub.set_lamda(ADDR(QDBusInterface, doCall), [] {
        __DBG_STUB_INVOKE__
        return QDBusMessage();
    });
    FrameManager *manager = new FrameManager();

    dpfSlotChannel->connect(kOrganizerSpace, "slot_Organizer_Enabled",
                            manager, &FrameManager::organizerEnabled);
    QVariant ret = dpfSlotChannel->push(kOrganizerSpace, "slot_Organizer_Enabled");

    EXPECT_EQ(ret.toBool(), false);
    EXPECT_NE(manager, nullptr);
}

TEST_F(SignalDispatchTest, SignalDispatcher_PublishDesktopFrameSignals_InvokesFrameManagerHandlers)
{
    stub.set_lamda(ADDR(QDBusInterface, doCall), [] {
        __DBG_STUB_INVOKE__
        return QDBusMessage();
    });
    FrameManager *manager = new FrameManager();
    int buildCount = 0;
    int detachCount = 0;
    int geometryCount = 0;
    stub.set_lamda(ADDR(FrameManager, onBuild),
                   [&buildCount](FrameManager *) {
                       __DBG_STUB_INVOKE__
                       ++buildCount;
                   });
    stub.set_lamda(ADDR(FrameManager, onDetachWindows),
                   [&detachCount](FrameManager *) {
                       __DBG_STUB_INVOKE__
                       ++detachCount;
                   });
    stub.set_lamda(ADDR(FrameManager, onGeometryChanged),
                   [&geometryCount](FrameManager *) {
                       __DBG_STUB_INVOKE__
                       ++geometryCount;
                   });

    EXPECT_TRUE(dpfSignalDispatcher->subscribe(kCoreSpace, "signal_DesktopFrame_WindowAboutToBeBuilded",
                                               manager, &FrameManager::onDetachWindows));
    EXPECT_TRUE(dpfSignalDispatcher->subscribe(kCoreSpace, "signal_DesktopFrame_WindowBuilded",
                                               manager, &FrameManager::onBuild));
    EXPECT_TRUE(dpfSignalDispatcher->subscribe(kCoreSpace, "signal_DesktopFrame_WindowShowed",
                                               manager, &FrameManager::onWindowShowed));
    EXPECT_TRUE(dpfSignalDispatcher->subscribe(kCoreSpace, "signal_DesktopFrame_GeometryChanged",
                                               manager, &FrameManager::onGeometryChanged));
    EXPECT_TRUE(dpfSignalDispatcher->subscribe(kCoreSpace, "signal_DesktopFrame_AvailableGeometryChanged",
                                               manager, &FrameManager::onGeometryChanged));

    bool showedRet = dpfSignalDispatcher->publish(kCoreSpace, "signal_DesktopFrame_WindowShowed");
    bool buildRet = dpfSignalDispatcher->publish(kCoreSpace, "signal_DesktopFrame_WindowBuilded");
    bool detachRet = dpfSignalDispatcher->publish(kCoreSpace, "signal_DesktopFrame_WindowAboutToBeBuilded");
    bool geometryRet = dpfSignalDispatcher->publish(kCoreSpace, "signal_DesktopFrame_GeometryChanged");
    bool availableRet = dpfSignalDispatcher->publish(kCoreSpace, "signal_DesktopFrame_AvailableGeometryChanged");

    EXPECT_TRUE(showedRet);
    EXPECT_TRUE(buildRet);
    EXPECT_TRUE(detachRet);
    EXPECT_TRUE(geometryRet);
    EXPECT_TRUE(availableRet);
    EXPECT_EQ(buildCount, 1);
    EXPECT_EQ(detachCount, 1);
    EXPECT_EQ(geometryCount, 2);

    EXPECT_TRUE(dpfSignalDispatcher->unsubscribe(kCoreSpace, "signal_DesktopFrame_WindowAboutToBeBuilded",
                                                 manager, &FrameManager::onDetachWindows));
    EXPECT_TRUE(dpfSignalDispatcher->unsubscribe(kCoreSpace, "signal_DesktopFrame_WindowBuilded",
                                                 manager, &FrameManager::onBuild));
    EXPECT_TRUE(dpfSignalDispatcher->unsubscribe(kCoreSpace, "signal_DesktopFrame_WindowShowed",
                                                 manager, &FrameManager::onWindowShowed));
    EXPECT_TRUE(dpfSignalDispatcher->unsubscribe(kCoreSpace, "signal_DesktopFrame_GeometryChanged",
                                                 manager, &FrameManager::onGeometryChanged));
    EXPECT_TRUE(dpfSignalDispatcher->unsubscribe(kCoreSpace, "signal_DesktopFrame_AvailableGeometryChanged",
                                                 manager, &FrameManager::onGeometryChanged));
}
