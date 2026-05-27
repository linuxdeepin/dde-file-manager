// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/emblemimpl/extensionemblemmanager.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/emblemimpl/extensionemblemmanager_p.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/pluginsload/extensionpluginmanager.h"

#include <dfm-framework/event/event.h>

#include <QIcon>
#include <QSignalSpy>
#include <QThread>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DPF_USE_NAMESPACE

// ========== ExtensionEmblemManagerPrivate Tests ==========

class UT_ExtensionEmblemManagerPrivate : public testing::Test
{
protected:
    void SetUp() override
    {
        manager = new ExtensionEmblemManager();
        d = new ExtensionEmblemManagerPrivate(manager);
    }

    void TearDown() override
    {
        delete d;
        d = nullptr;
        delete manager;
        manager = nullptr;
        stub.clear();
    }

    ExtensionEmblemManager *manager { nullptr };
    ExtensionEmblemManagerPrivate *d { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_ExtensionEmblemManagerPrivate, Constructor_InitializesFields)
{
    EXPECT_NE(d, nullptr);
    EXPECT_EQ(d->q_ptr, manager);
    EXPECT_FALSE(d->readyFlag);
    EXPECT_TRUE(d->readyLocalPaths.isEmpty());
}

TEST_F(UT_ExtensionEmblemManagerPrivate, addReadyLocalPath_AddsPath)
{
    QPair<QString, int> path("/test/file.txt", 0);

    d->addReadyLocalPath(path);

    EXPECT_TRUE(d->readyLocalPaths.contains(path));
    EXPECT_TRUE(d->readyFlag);
}

TEST_F(UT_ExtensionEmblemManagerPrivate, addReadyLocalPath_DuplicatePath_NotAdded)
{
    QPair<QString, int> path("/test/file.txt", 0);

    d->addReadyLocalPath(path);
    d->addReadyLocalPath(path);

    EXPECT_EQ(d->readyLocalPaths.count(), 1);
}

TEST_F(UT_ExtensionEmblemManagerPrivate, addReadyLocalPath_DifferentPaths_AllAdded)
{
    d->addReadyLocalPath({ "/test/file1.txt", 0 });
    d->addReadyLocalPath({ "/test/file2.txt", 1 });
    d->addReadyLocalPath({ "/test/file3.txt", 2 });

    EXPECT_EQ(d->readyLocalPaths.count(), 3);
}

TEST_F(UT_ExtensionEmblemManagerPrivate, clearReadyLocalPath_ClearsAll)
{
    d->addReadyLocalPath({ "/test/file1.txt", 0 });
    d->addReadyLocalPath({ "/test/file2.txt", 1 });

    d->clearReadyLocalPath();

    EXPECT_TRUE(d->readyLocalPaths.isEmpty());
    EXPECT_FALSE(d->readyFlag);
}

TEST_F(UT_ExtensionEmblemManagerPrivate, makeIcon_ThemeName_ReturnsThemeIcon)
{
    QIcon result = d->makeIcon("dialog-information");

    EXPECT_FALSE(result.isNull());
}

TEST_F(UT_ExtensionEmblemManagerPrivate, makeIcon_FilePath_ReturnsFileIcon)
{
    QIcon result = d->makeIcon("/nonexistent/path/icon.png");
}

TEST_F(UT_ExtensionEmblemManagerPrivate, makeIcon_EmptyPath_ReturnsEmptyIcon)
{
    QIcon result = d->makeIcon("");

    EXPECT_TRUE(result.isNull());
}

// ========== ExtensionEmblemManager Tests ==========

class UT_ExtensionEmblemManager : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(UT_ExtensionEmblemManager, instance_ReturnsSingleton)
{
    ExtensionEmblemManager &instance1 = ExtensionEmblemManager::instance();
    ExtensionEmblemManager &instance2 = ExtensionEmblemManager::instance();

    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(UT_ExtensionEmblemManager, initialize_RegistersMetaType)
{
    ExtensionEmblemManager::instance().initialize();
}

TEST_F(UT_ExtensionEmblemManager, onFetchCustomEmblems_InvalidUrl_ReturnsFalse)
{
    QUrl invalidUrl;
    QList<QIcon> emblems;

    bool result = ExtensionEmblemManager::instance().onFetchCustomEmblems(invalidUrl, &emblems);

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionEmblemManager, onFetchCustomEmblems_PluginsNotInitialized_ReturnsFalse)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, currentState),
                   [] {
                       __DBG_STUB_INVOKE__
                       return ExtensionPluginManager::kReady;
                   });

    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QIcon> emblems;

    bool result = ExtensionEmblemManager::instance().onFetchCustomEmblems(url, &emblems);

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionEmblemManager, onFetchCustomEmblems_NoEmblemPlugin_ReturnsFalse)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, currentState),
                   [] {
                       __DBG_STUB_INVOKE__
                       return ExtensionPluginManager::kInitialized;
                   });

    stub.set_lamda(ADDR(ExtensionPluginManager, exists),
                   [](ExtensionPluginManager *, ExtensionPluginManager::ExtensionType) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QIcon> emblems;

    bool result = ExtensionEmblemManager::instance().onFetchCustomEmblems(url, &emblems);

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionEmblemManager, onFetchCustomEmblems_TooManyEmblems_ReturnsFalse)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, currentState),
                   [] {
                       __DBG_STUB_INVOKE__
                       return ExtensionPluginManager::kInitialized;
                   });

    stub.set_lamda(ADDR(ExtensionPluginManager, exists),
                   [](ExtensionPluginManager *, ExtensionPluginManager::ExtensionType) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QIcon> emblems;
    for (int i = 0; i < 5; ++i)
        emblems.append(QIcon());

    bool result = ExtensionEmblemManager::instance().onFetchCustomEmblems(url, &emblems);

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionEmblemManager, onEmblemIconChanged_UpdatesCache)
{
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QUrl);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push),
                   [](EventChannelManager *, const QString &, const QString &, QUrl) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant();
                   });

    stub.set_lamda(ADDR(Event, eventType),
                   [](Event *, const QString &, const QString &) -> DPF_NAMESPACE::EventType {
                       __DBG_STUB_INVOKE__
                       return DPF_NAMESPACE::EventTypeScope::kInValid;
                   });

    QString path = "/tmp/test.txt";
    QList<QPair<QString, int>> group = { qMakePair(QString("emblem-default"), 0) };

    ExtensionEmblemManager::instance().onEmblemIconChanged(path, group);
}

TEST_F(UT_ExtensionEmblemManager, onUrlChanged_ClearsCacheAndEmitsSignal)
{
    QSignalSpy spy(&ExtensionEmblemManager::instance(), &ExtensionEmblemManager::requestClearCache);

    bool result = ExtensionEmblemManager::instance().onUrlChanged(0, QUrl::fromLocalFile("/tmp"));

    EXPECT_FALSE(result);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_ExtensionEmblemManager, onAllPluginsInitialized_StartsWorkerThread)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, emblemPlugins),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QList<DFMEXT::DFMExtEmblemIconPlugin *>();
                   });

    ExtensionEmblemManager::instance().onAllPluginsInitialized();
}

// ========== EmblemIconWorker Tests ==========

class UT_EmblemIconWorker : public testing::Test
{
protected:
    void SetUp() override
    {
        worker = new EmblemIconWorker();
    }

    void TearDown() override
    {
        delete worker;
        worker = nullptr;
        stub.clear();
    }

    EmblemIconWorker *worker { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_EmblemIconWorker, Constructor_CreatesWorker)
{
    EXPECT_NE(worker, nullptr);
}

TEST_F(UT_EmblemIconWorker, onClearCache_ClearsCaches)
{
    worker->onClearCache();
}

TEST_F(UT_EmblemIconWorker, onFetchEmblemIcons_EmptyPaths_ReturnsEarly)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, emblemPlugins),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QList<DFMEXT::DFMExtEmblemIconPlugin *>();
                   });

    stub.set_lamda(&QThread::currentThread, []() {
        __DBG_STUB_INVOKE__
        return nullptr;   // Different from app thread
    });

    QList<QPair<QString, int>> emptyPaths;
    worker->onFetchEmblemIcons(emptyPaths);
}

TEST_F(UT_EmblemIconWorker, makeCache_ReturnsValidCache)
{
    QString path = "/test/file.txt";
    QList<QPair<QString, int>> group = { qMakePair(QString("icon"), 0) };

    auto cache = worker->makeCache(path, group);

    EXPECT_TRUE(cache.contains(path));
    EXPECT_EQ(cache.value(path), group);
}

TEST_F(UT_EmblemIconWorker, makeLayoutGroup_EmptyLayouts_ReturnsEmptyGroup)
{
    std::vector<DFMEXT::DFMExtEmblemIconLayout> layouts;
    QList<QPair<QString, int>> group;

    worker->makeLayoutGroup(layouts, &group);

    EXPECT_TRUE(group.isEmpty());
}

TEST_F(UT_EmblemIconWorker, makeNormalGroup_EmptyIcons_ReturnsEmptyGroup)
{
    std::vector<std::string> icons;
    QList<QPair<QString, int>> group;

    worker->makeNormalGroup(icons, 0, &group);

    EXPECT_TRUE(group.isEmpty());
}

TEST_F(UT_EmblemIconWorker, makeNormalGroup_WithIcons_CreatesGroup)
{
    std::vector<std::string> icons = { "icon1", "icon2" };
    QList<QPair<QString, int>> group;

    worker->makeNormalGroup(icons, 0, &group);

    EXPECT_EQ(group.size(), 2);
    EXPECT_EQ(group[0].first, "icon1");
    EXPECT_EQ(group[0].second, 0);
    EXPECT_EQ(group[1].first, "icon2");
    EXPECT_EQ(group[1].second, 1);
}

TEST_F(UT_EmblemIconWorker, mergeGroup_BothEmpty_ReturnsEmpty)
{
    QList<QPair<QString, int>> oldGroup;
    QList<QPair<QString, int>> newGroup;
    QList<QPair<QString, int>> result;

    worker->mergeGroup(oldGroup, newGroup, &result);

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_EmblemIconWorker, mergeGroup_OnlyOldGroup_ReturnsOld)
{
    QList<QPair<QString, int>> oldGroup = { qMakePair(QString("old-icon"), 0) };
    QList<QPair<QString, int>> newGroup;
    QList<QPair<QString, int>> result;

    worker->mergeGroup(oldGroup, newGroup, &result);

    EXPECT_EQ(result.size(), 1);
}

TEST_F(UT_EmblemIconWorker, mergeGroup_OnlyNewGroup_ReturnsNew)
{
    QList<QPair<QString, int>> oldGroup;
    QList<QPair<QString, int>> newGroup = { qMakePair(QString("new-icon"), 0) };
    QList<QPair<QString, int>> result;

    worker->mergeGroup(oldGroup, newGroup, &result);

    EXPECT_EQ(result.size(), 1);
}

TEST_F(UT_EmblemIconWorker, mergeGroup_SamePosition_NewOverwritesOld)
{
    QList<QPair<QString, int>> oldGroup = { qMakePair(QString("old-icon"), 0) };
    QList<QPair<QString, int>> newGroup = { qMakePair(QString("new-icon"), 0) };
    QList<QPair<QString, int>> result;

    worker->mergeGroup(oldGroup, newGroup, &result);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].first, "new-icon");
}

TEST_F(UT_EmblemIconWorker, updateLayoutGroup_EmptyCache_ReturnsGroup)
{
    QList<QPair<QString, int>> cache;
    QList<QPair<QString, int>> group = { qMakePair(QString("icon"), 0) };

    auto result = worker->updateLayoutGroup(cache, group);

    EXPECT_EQ(result, group);
}

TEST_F(UT_EmblemIconWorker, updateLayoutGroup_SameAsCache_ReturnsGroup)
{
    QList<QPair<QString, int>> data = { qMakePair(QString("icon"), 0) };

    auto result = worker->updateLayoutGroup(data, data);

    EXPECT_EQ(result, data);
}

TEST_F(UT_EmblemIconWorker, hasCachedByOtherLocationEmblem_NoCache_ReturnsFalse)
{
    bool result = worker->hasCachedByOtherLocationEmblem("/test/file.txt", 0);

    EXPECT_FALSE(result);
}

TEST_F(UT_EmblemIconWorker, makeNormalGroup_PositionOverflow_LimitsGroup)
{
    std::vector<std::string> icons = { "icon1", "icon2", "icon3", "icon4", "icon5" };
    QList<QPair<QString, int>> group;

    worker->makeNormalGroup(icons, 0, &group);

    EXPECT_LE(group.size(), 4);   // kMaxEmblemCount = 4
}

TEST_F(UT_EmblemIconWorker, mergeGroup_DifferentPositions_MergesBoth)
{
    QList<QPair<QString, int>> oldGroup = { qMakePair(QString("old-icon"), 0) };
    QList<QPair<QString, int>> newGroup = { qMakePair(QString("new-icon"), 1) };
    QList<QPair<QString, int>> result;

    worker->mergeGroup(oldGroup, newGroup, &result);

    EXPECT_EQ(result.size(), 2);
}

TEST_F(UT_EmblemIconWorker, updateLayoutGroup_DifferentCache_ReturnsUpdatedGroup)
{
    QList<QPair<QString, int>> cache = { qMakePair(QString("cached"), 0) };
    QList<QPair<QString, int>> group = { qMakePair(QString("new"), 1) };

    auto result = worker->updateLayoutGroup(cache, group);

    EXPECT_EQ(result.size(), 2);
}

TEST_F(UT_EmblemIconWorker, makeLayoutGroup_WithLayouts_CreatesGroup)
{
    std::vector<DFMEXT::DFMExtEmblemIconLayout> layouts;
    DFMEXT::DFMExtEmblemIconLayout layout(DFMEXT::DFMExtEmblemIconLayout::LocationType::BottomRight, "test-icon");
    layouts.push_back(layout);

    QList<QPair<QString, int>> group;
    worker->makeLayoutGroup(layouts, &group);

    EXPECT_EQ(group.size(), 1);
    EXPECT_EQ(group[0].first, "test-icon");
}

// ========== ExtensionEmblemManager Additional Tests ==========

TEST_F(UT_ExtensionEmblemManager, onFetchCustomEmblems_ValidUrlWithCache_ReturnsResult)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, currentState),
                   [] {
                       __DBG_STUB_INVOKE__
                       return ExtensionPluginManager::kInitialized;
                   });

    stub.set_lamda(ADDR(ExtensionPluginManager, exists),
                   [](ExtensionPluginManager *, ExtensionPluginManager::ExtensionType) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QIcon> emblems;

    bool result = ExtensionEmblemManager::instance().onFetchCustomEmblems(url, &emblems);

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionEmblemManager, onEmblemIconChanged_DesktopPlugin_PushesToCanvas)
{
    stub.set_lamda(ADDR(Event, eventType),
                   [](Event *, const QString &, const QString &) -> DPF_NAMESPACE::EventType {
                       __DBG_STUB_INVOKE__
                       return 1;   // Valid event type
                   });

    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QUrl);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push),
                   [](EventChannelManager *, const QString &, const QString &, QUrl) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant();
                   });

    QString path = "/tmp/test.txt";
    QList<QPair<QString, int>> group = { qMakePair(QString("emblem"), 0) };

    ExtensionEmblemManager::instance().onEmblemIconChanged(path, group);
}
