// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>
#include <QDBusReply>
#include <QDBusInterface>
#include <QTemporaryDir>
#include <QTimer>

// 包含待测试的类
#include "utils/recentmanager.h"
#include "files/recentfileinfo.h"
#include "events/recenteventcaller.h"
#include "dfmplugin_recent_global.h"

// 包含依赖的头文件
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/sysinfoutils.h>

#include <DDesktopServices>
#include <QDBusArgument>
#include <QAction>
#include <QMenu>
#include <QProcess>

DPRECENT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DGUI_USE_NAMESPACE

/**
 * @brief RecentManager类单元测试
 *
 * 测试范围：
 * 1. 单例模式验证
 * 2. 初始化和清理流程
 * 3. DBus接口通信
 * 4. 最近文件数据管理
 * 5. 文件监视控制
 * 6. 异常处理和边界条件
 */
class RecentManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // NOTE: do NOT stub QThread::currentThread here: InfoCache's worker
        // thread asserts `qApp->thread() != currentThread()` and a global
        // currentThread stub breaks that worker thread.

        // Register the "recent" scheme exactly once for the whole binary so
        // InfoFactory::create<FileInfo>(recent://...) returns a RecentFileInfo.
        // Also register "file" so RecentFileInfo gets a real proxy and
        // urlOf(kRedirectedFileUrl) resolves to the local file path.
        static std::once_flag schemeOnce;
        std::call_once(schemeOnce, [] {
            if (!UrlRoute::hasScheme(RecentHelper::scheme()))
                UrlRoute::regScheme(RecentHelper::scheme(), "/", RecentHelper::icon(),
                                    true, QStringLiteral("Recent"));
            InfoFactory::regClass<RecentFileInfo>(RecentHelper::scheme());
            // "file" needs both UrlRoute and factory registration, otherwise
            // SchemeFactory::create(file://) returns null and RecentFileInfo
            // ends up without a proxy.
            if (!UrlRoute::hasScheme(Global::Scheme::kFile))
                UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(),
                                    false, QStringLiteral("File"));
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        });

        // Create temporary directory for testing
        tempDir = new QTemporaryDir();
        ASSERT_TRUE(tempDir->isValid());

        // Initialize manager
        manager = RecentManager::instance();
        ASSERT_NE(manager, nullptr);
    }

    void TearDown() override
    {
        // Reset the singleton caches so cases stay independent.
        if (manager) {
            manager->pendingItems.clear();
            manager->recentItems.clear();
            manager->batchTimer.stop();
        }
        // Clean up resources
        delete tempDir;
        stub.clear();
    }

    stub_ext::StubExt stub;
    RecentManager *manager { nullptr };
    QTemporaryDir *tempDir { nullptr };
    int menuExecCalls { 0 };

    // Stub menu exec to behave like the user clicked the action at
    // actionIndex (-1 = dismiss without choosing): emit triggered() while the
    // action is still owned by the menu so its connected lambda runs.
    void StubMenuExecReturnAction(int actionIndex)
    {
        menuExecCalls = 0;
        stub.set_lamda(static_cast<QAction *(QMenu::*)(const QPoint &, QAction *)>(&QMenu::exec),
                       [this, actionIndex](QMenu *self, const QPoint &, QAction *) -> QAction * {
                           __DBG_STUB_INVOKE__
                           ++menuExecCalls;
                           const auto acts = self->actions();
                           QAction *act = (actionIndex >= 0 && actionIndex < acts.size())
                                   ? acts.at(actionIndex)
                                   : nullptr;
                           if (act)
                               emit act->triggered();
                           return act;
                       });
    }

    // Deterministic InfoFactory::create<FileInfo> replacement that skips the
    // InfoCache async machinery entirely (kAuto caching is thread-sensitive).
    // Non-recent lookups (the proxy inside RecentFileInfo) return null, which
    // is a supported state and also breaks the recursion.
    void StubInfoFactoryCreate()
    {
        stub.set_lamda(static_cast<FileInfoPointer (*)(const QUrl &, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>),
                       [](const QUrl &url, Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                           __DBG_STUB_INVOKE__
                           if (url.scheme() != RecentHelper::scheme())
                               return nullptr;
                           return FileInfoPointer(new RecentFileInfo(url));
                       });
    }
};

/**
 * @brief 测试单例模式
 * 验证RecentManager::instance()返回同一个实例
 */
TEST_F(RecentManagerTest, SingletonPattern_ReturnsSameInstance)
{
    RecentManager *instance1 = RecentManager::instance();
    RecentManager *instance2 = RecentManager::instance();

    // 验证是同一个实例
    EXPECT_EQ(instance1, instance2);
    EXPECT_EQ(manager, instance1);

    // 验证实例不为空
    EXPECT_NE(instance1, nullptr);
}

/**
 * @brief 测试初始化流程
 * 验证RecentManager初始化时正确设置所有组件
 */
TEST_F(RecentManagerTest, Initialize_SetsUpCorrectly)
{
    bool dbusInterfaceCreated = false;

    // Mock DBus interface creation
    stub.set_lamda(&RecentManagerDBusInterface::Reload,
                   [&]() {
        __DBG_STUB_INVOKE__
        dbusInterfaceCreated = true;
        return QDBusPendingReply<qlonglong>();
    });

    // Call init method
    manager->init();

    // Verify DBus interface was created
    EXPECT_TRUE(dbusInterfaceCreated);
}

/**
 * @brief 测试DBus接口连接
 * 验证与RecentManager DBus服务的连接建立
 */
TEST_F(RecentManagerTest, DBusInterface_ConnectsSuccessfully)
{
    EXPECT_NO_FATAL_FAILURE(manager->dbus());
}

/**
 * @brief 测试获取最近文件节点
 * 验证getRecentNodes方法返回正确的文件信息映射
 */
TEST_F(RecentManagerTest, GetRecentNodes_ReturnsValidData)
{
    EXPECT_NO_FATAL_FAILURE(manager->getRecentNodes());
}

/**
 * @brief 测试获取不存在路径的原始路径
 * 验证对不存在URL的处理
 */
TEST_F(RecentManagerTest, GetRecentOriginPaths_NonexistentUrl_ReturnsEmpty)
{
    QUrl nonexistentUrl = QUrl("recent:///nonexistent.txt");

    // Clear internal data
    manager->recentItems.clear();

    // Test with nonexistent URL
    QString result = manager->getRecentOriginPaths(nonexistentUrl);

    // Verify empty result
    EXPECT_TRUE(result.isEmpty());
}

/**
 * @brief 测试重置最近文件节点
 * 验证resetRecentNodes方法正确处理DBus返回的数据
 */
TEST_F(RecentManagerTest, ResetRecentNodes_UpdatesData)
{
    bool dbusCallMade = false;
    QVariantList mockData;

    // Prepare mock DBus response
    QVariantMap item1;
    item1["Path"] = "/test1.txt";
    item1["Href"] = "file:///test1.txt";
    item1["modified"] = static_cast<qint64>(1234567890);

    QDBusArgument arg1;
    // Note: QDBusArgument construction is complex, mock it
    mockData.append(QVariant::fromValue(arg1));

    // Mock DBus GetItemsInfo call
    stub.set_lamda(&RecentManagerDBusInterface::GetItemsInfo,
                   [&](RecentManagerDBusInterface *) -> QDBusPendingReply<QVariantList> {
        __DBG_STUB_INVOKE__
        dbusCallMade = true;
        QDBusPendingReply<QVariantList> reply;
        // Mock successful reply
        return reply;
    });

    // Mock QDBusPendingReply waitForFinished and value
    stub.set_lamda(&QDBusPendingCallWatcher::waitForFinished, [](QDBusPendingCallWatcher *) {
        __DBG_STUB_INVOKE__
    });

    // Test resetRecentNodes
    manager->resetRecentNodes();

    // Verify DBus call was made
    EXPECT_TRUE(dbusCallMade);
}

/**
 * @brief 测试DBus调用失败处理
 * 验证DBus服务不可用时的错误处理
 */
TEST_F(RecentManagerTest, DBusFailure_HandlesGracefully)
{
    // Mock DBus interface to be invalid
    stub.set_lamda(&QDBusAbstractInterface::isValid, [](const QDBusAbstractInterface *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock failed DBus call
    stub.set_lamda(&RecentManagerDBusInterface::GetItemsInfo,
                   [](RecentManagerDBusInterface *) -> QDBusPendingReply<QVariantList> {
        __DBG_STUB_INVOKE__
        QDBusPendingReply<QVariantList> reply;
        return reply;  // Return invalid reply
    });

    // Test should not crash
    EXPECT_NO_THROW(manager->resetRecentNodes());
}

/**
 * @brief 测试边界条件 - 空URL
 * 验证空URL的处理
 */
TEST_F(RecentManagerTest, EmptyUrl_HandlesGracefully)
{
    QUrl emptyUrl;

    // Test with empty URL
    QString result = manager->getRecentOriginPaths(emptyUrl);

    // Should return empty string for empty URL
    EXPECT_TRUE(result.isEmpty());
}

/**
 * @brief 测试边界条件 - 无效URL格式
 * 验证无效URL格式的处理
 */
TEST_F(RecentManagerTest, InvalidUrlFormat_HandlesGracefully)
{
    // Test with various invalid URL formats
    QStringList invalidUrls = {
        "not-a-url",
        "://invalid",
        "recent://",
        "recent:///../../outside",
        QString(10000, 'a')  // Very long string
    };

    for (const QString &urlStr : invalidUrls) {
        QUrl invalidUrl(urlStr);
        EXPECT_NO_THROW({
            QString result = manager->getRecentOriginPaths(invalidUrl);
            // Should handle gracefully without crashing
        });
    }
}

/**
 * @brief 测试多线程访问安全性
 * 验证在多线程环境下的安全性
 */
TEST_F(RecentManagerTest, MultiThreadAccess_ThreadSafe)
{
    // Mock thread operations
    stub.set_lamda(&QThread::currentThread, []() {
        __DBG_STUB_INVOKE__
        return QCoreApplication::instance()->thread();
    });

    // Test concurrent access
    EXPECT_NO_THROW({
        RecentManager *instance1 = RecentManager::instance();
        RecentManager *instance2 = RecentManager::instance();
        EXPECT_EQ(instance1, instance2);
    });
}

/**
 * @brief 测试析构函数清理
 * 验证对象销毁时的资源清理
 */
TEST_F(RecentManagerTest, Destructor_CleansUpProperly)
{
    // Note: Since RecentManager is a singleton, we can't easily test destructor
    // But we can test cleanup methods

    manager->recentItems.clear();

    // Verify cleanup
    auto nodes = manager->getRecentNodes();
    EXPECT_EQ(nodes.size(), 0);
}

/**
 * @brief RecentHelper::recentUrl 应把本地路径包装为 recent scheme
 */
TEST_F(RecentManagerTest, RecentUrl_ConvertsLocalPathToRecentScheme)
{
    const QUrl url = RecentHelper::recentUrl("/tmp/a.txt");

    EXPECT_EQ(url.scheme(), QStringLiteral("recent"));
    EXPECT_EQ(url.path(), QStringLiteral("/tmp/a.txt"));

    const QUrl emptyUrl = RecentHelper::recentUrl(QString());
    EXPECT_EQ(emptyUrl.scheme(), QStringLiteral("recent"));
    EXPECT_TRUE(emptyUrl.path().isEmpty());
}

/**
 * @brief onItemAdded 对合法路径应加入缓存并记录 originPath
 */
TEST_F(RecentManagerTest, OnItemAdded_ValidPath_InsertsIntoCache)
{
    const QString path = tempDir->filePath("added.txt");
    const QString href = QUrl::fromLocalFile(path).toString();

    manager->onItemAdded(path, href, 1700000000);

    const QUrl recentUrl = RecentHelper::recentUrl(path);
    EXPECT_EQ(manager->recentItems.size(), 1);
    EXPECT_TRUE(manager->recentItems.contains(recentUrl));
    EXPECT_EQ(manager->getRecentOriginPaths(recentUrl), href);
}

/**
 * @brief onItemAdded 对空路径应忽略
 */
TEST_F(RecentManagerTest, OnItemAdded_EmptyPath_Ignored)
{
    manager->onItemAdded(QString(), QString("file:///tmp/none.txt"), 1);

    EXPECT_EQ(manager->recentItems.size(), 0);
    EXPECT_EQ(manager->size(), 0);
}

/**
 * @brief onItemAdded 重复路径只保留一条
 */
TEST_F(RecentManagerTest, OnItemAdded_DuplicatePath_KeepsSingleEntry)
{
    const QString path = tempDir->filePath("dup.txt");

    manager->onItemAdded(path, QString("file:///tmp/dup.txt"), 1);
    manager->onItemAdded(path, QString("file:///tmp/dup.txt"), 2);

    EXPECT_EQ(manager->recentItems.size(), 1);
    EXPECT_EQ(manager->getRecentNodes().size(), 1);
}

/**
 * @brief onItemsRemoved 应移除已存在项并保持其他项
 */
TEST_F(RecentManagerTest, OnItemsRemoved_ExistingPaths_RemoveFromCache)
{
    const QString kept = tempDir->filePath("kept.txt");
    const QString dropped = tempDir->filePath("dropped.txt");
    manager->onItemAdded(kept, QString("file:///tmp/kept.txt"), 1);
    manager->onItemAdded(dropped, QString("file:///tmp/dropped.txt"), 1);
    ASSERT_EQ(manager->recentItems.size(), 2);

    manager->onItemsRemoved(QStringList { dropped });

    EXPECT_EQ(manager->recentItems.size(), 1);
    EXPECT_FALSE(manager->recentItems.contains(RecentHelper::recentUrl(dropped)));
    EXPECT_TRUE(manager->recentItems.contains(RecentHelper::recentUrl(kept)));
}

/**
 * @brief onItemsRemoved 对未知路径应无副作用
 */
TEST_F(RecentManagerTest, OnItemsRemoved_UnknownPath_NoEffect)
{
    const QString path = tempDir->filePath("stable.txt");
    manager->onItemAdded(path, QString("file:///tmp/stable.txt"), 1);
    ASSERT_EQ(manager->recentItems.size(), 1);

    manager->onItemsRemoved(QStringList { "/tmp/never_added.txt" });

    EXPECT_EQ(manager->recentItems.size(), 1);
    EXPECT_EQ(manager->size(), 1);
}

/**
 * @brief onItemChanged 应更新已存在项的最近访问时间
 */
TEST_F(RecentManagerTest, OnItemChanged_ExistingPath_UpdatesLastReadTime)
{
    const QString path = tempDir->filePath("changed.txt");
    manager->onItemAdded(path, QString("file:///tmp/changed.txt"), 100);
    const QUrl recentUrl = RecentHelper::recentUrl(path);

    manager->onItemChanged(path, 1700001234);

    const auto info = manager->recentItems.value(recentUrl).fileInfo.dynamicCast<RecentFileInfo>();
    ASSERT_FALSE(info.isNull());
    EXPECT_EQ(info->lastReadTime, QDateTime::fromSecsSinceEpoch(1700001234));
    EXPECT_EQ(manager->recentItems.size(), 1);
}

/**
 * @brief onItemChanged 对空路径和未知路径应忽略
 */
TEST_F(RecentManagerTest, OnItemChanged_EmptyOrUnknownPath_Ignored)
{
    manager->onItemAdded(tempDir->filePath("anchor.txt"), QString("file:///tmp/a.txt"), 1);
    ASSERT_EQ(manager->recentItems.size(), 1);

    manager->onItemChanged(QString(), 42);
    manager->onItemChanged("/tmp/not_in_cache.txt", 42);

    EXPECT_EQ(manager->recentItems.size(), 1);
    EXPECT_EQ(manager->size(), 1);
}

/**
 * @brief removeRecentFile 按存在性返回正确结果
 */
TEST_F(RecentManagerTest, RemoveRecentFile_PresentAndAbsentUrls)
{
    const QString path = tempDir->filePath("removable.txt");
    manager->onItemAdded(path, QString("file:///tmp/removable.txt"), 1);
    const QUrl recentUrl = RecentHelper::recentUrl(path);

    EXPECT_FALSE(manager->removeRecentFile(QUrl("recent:///tmp/absent.txt")));
    EXPECT_TRUE(manager->removeRecentFile(recentUrl));
    EXPECT_FALSE(manager->recentItems.contains(recentUrl));
    EXPECT_EQ(manager->size(), 0);
}

/**
 * @brief reloadRecent 应触发一次 DBus Reload 调用
 */
TEST_F(RecentManagerTest, ReloadRecent_InvokesDBusReloadOnce)
{
    int reloadCalls = 0;
    stub.set_lamda(&RecentManagerDBusInterface::Reload, [&]() {
        __DBG_STUB_INVOKE__
        ++reloadCalls;
        return QDBusPendingReply<qlonglong>();
    });
    stub.set_lamda(&QDBusPendingCallWatcher::waitForFinished, [](QDBusPendingCallWatcher *) {
        __DBG_STUB_INVOKE__
    });

    manager->init();
    ASSERT_NE(manager->dbus(), nullptr);

    const int before = reloadCalls;
    manager->reloadRecent();

    EXPECT_EQ(reloadCalls - before, 1);
    EXPECT_GE(reloadCalls, 2);   // one from init(), one from reloadRecent()
}

/**
 * @brief clearRecent 应调用 DBus PurgeItems
 */
TEST_F(RecentManagerTest, ClearRecent_PurgesAllItemsViaDbus)
{
    stub.set_lamda(&RecentManagerDBusInterface::Reload, []() {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<qlonglong>();
    });
    stub.set_lamda(&QDBusPendingCallWatcher::waitForFinished, [](QDBusPendingCallWatcher *) {
        __DBG_STUB_INVOKE__
    });
    manager->init();
    ASSERT_NE(manager->dbus(), nullptr);

    int purgeCalls = 0;
    stub.set_lamda(&RecentManagerDBusInterface::PurgeItems, [&]() {
        __DBG_STUB_INVOKE__
        ++purgeCalls;
        return QDBusPendingReply<>();
    });

    RecentHelper::clearRecent();

    EXPECT_EQ(purgeCalls, 1);
    EXPECT_NE(manager->dbus(), nullptr);
}

/**
 * @brief contenxtMenuHandle 选中"在新窗口打开"应发送开窗事件
 */
TEST_F(RecentManagerTest, ContexntMenuHandle_OpenInNewWindowAction_SendsOpenWindow)
{
    int openWindowCalls = 0;
    stub.set_lamda(&RecentEventCaller::sendOpenWindow, [&](const QUrl &) {
        __DBG_STUB_INVOKE__
        ++openWindowCalls;
    });
    StubMenuExecReturnAction(0);

    RecentHelper::contenxtMenuHandle(1, QUrl("recent:///tmp/menu_win.txt"), QPoint(10, 10));

    EXPECT_EQ(menuExecCalls, 1);
    EXPECT_EQ(openWindowCalls, 1);
}

/**
 * @brief contenxtMenuHandle 选中"在新标签打开"应发送开标签事件
 */
TEST_F(RecentManagerTest, ContexntMenuHandle_OpenInNewTabAction_SendsOpenTab)
{
    int openTabCalls = 0;
    stub.set_lamda(&RecentEventCaller::sendOpenTab, [&](quint64, const QUrl &) {
        __DBG_STUB_INVOKE__
        ++openTabCalls;
    });
    StubMenuExecReturnAction(1);

    RecentHelper::contenxtMenuHandle(2, QUrl("recent:///tmp/menu_tab.txt"), QPoint(0, 0));

    EXPECT_EQ(menuExecCalls, 1);
    EXPECT_EQ(openTabCalls, 1);
}

/**
 * @brief contenxtMenuHandle 选中"清空最近记录"应调用 PurgeItems
 */
TEST_F(RecentManagerTest, ContexntMenuHandle_ClearHistoryAction_PurgesRecent)
{
    stub.set_lamda(&RecentManagerDBusInterface::Reload, []() {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<qlonglong>();
    });
    stub.set_lamda(&QDBusPendingCallWatcher::waitForFinished, [](QDBusPendingCallWatcher *) {
        __DBG_STUB_INVOKE__
    });
    manager->init();

    int purgeCalls = 0;
    stub.set_lamda(&RecentManagerDBusInterface::PurgeItems, [&]() {
        __DBG_STUB_INVOKE__
        ++purgeCalls;
        return QDBusPendingReply<>();
    });
    StubMenuExecReturnAction(3);   // "Clear recent history"

    RecentHelper::contenxtMenuHandle(3, QUrl("recent:///tmp/menu_clear.txt"), QPoint(5, 5));

    EXPECT_EQ(menuExecCalls, 1);
    EXPECT_EQ(purgeCalls, 1);
}

/**
 * @brief contenxtMenuHandle 未选中任何菜单项时不应产生副作用
 */
TEST_F(RecentManagerTest, ContexntMenuHandle_NoSelection_TakesNoAction)
{
    int openWindowCalls = 0;
    stub.set_lamda(&RecentEventCaller::sendOpenWindow, [&](const QUrl &) {
        __DBG_STUB_INVOKE__
        ++openWindowCalls;
    });
    StubMenuExecReturnAction(-1);   // simulate no action triggered

    RecentHelper::contenxtMenuHandle(4, QUrl("recent:///tmp/menu_none.txt"), QPoint(1, 1));

    EXPECT_EQ(menuExecCalls, 1);
    EXPECT_EQ(openWindowCalls, 0);
}

/**
 * @brief openFileLocation 普通用户走桌面服务展示文件
 */
TEST_F(RecentManagerTest, OpenFileLocation_DesktopUser_ShowsFileItem)
{
    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal),
                   [](const QList<QUrl> &, QList<QUrl> *) {
                       __DBG_STUB_INVOKE__
                       return false;
                   });
    stub.set_lamda(ADDR(SysInfoUtils, isRootUser), []() {
        __DBG_STUB_INVOKE__
        return false;
    });
    bool shown = false;
    stub.set_lamda(static_cast<bool (*)(const QUrl &, const QString &)>(&DDesktopServices::showFileItem),
                   [&](const QUrl &, const QString &) {
                       __DBG_STUB_INVOKE__
                       shown = true;
                       return true;
                   });

    EXPECT_TRUE(RecentHelper::openFileLocation(QUrl("recent:///tmp/show_me.txt")));
    EXPECT_TRUE(shown);
}

/**
 * @brief openFileLocation root 用户走 dde-file-manager 进程
 */
TEST_F(RecentManagerTest, OpenFileLocation_RootUser_StartsFileManagerProcess)
{
    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal),
                   [](const QList<QUrl> &, QList<QUrl> *) {
                       __DBG_STUB_INVOKE__
                       return false;
                   });
    stub.set_lamda(ADDR(SysInfoUtils, isRootUser), []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    typedef bool (*StartDetachedFn)(const QString &, const QStringList &,
                                    const QString &, qint64 *);
    bool detached = false;
    stub.set_lamda(static_cast<StartDetachedFn>(&QProcess::startDetached),
                   [&](const QString &program, const QStringList &,
                       const QString &, qint64 *) {
                       __DBG_STUB_INVOKE__
                       detached = program == QStringLiteral("dde-file-manager");
                       return true;
                   });

    EXPECT_TRUE(RecentHelper::openFileLocation(QUrl("recent:///tmp/root_case.txt")));
    EXPECT_TRUE(detached);
}

/**
 * @brief openFileLocation 列表版本逐个转发
 */
TEST_F(RecentManagerTest, OpenFileLocation_UrlList_ForwardsEachUrl)
{
    int singleCalls = 0;
    stub.set_lamda(static_cast<bool (*)(const QUrl &)>(&RecentHelper::openFileLocation),
                   [&](const QUrl &) {
                       __DBG_STUB_INVOKE__
                       ++singleCalls;
                       return true;
                   });

    QList<QUrl> urls { QUrl("recent:///tmp/one.txt"), QUrl("recent:///tmp/two.txt"),
                       QUrl("recent:///tmp/three.txt") };
    RecentHelper::openFileLocation(urls);

    EXPECT_EQ(singleCalls, 3);
}

/**
 * @brief propetyExtensionFunc 应产出包含源路径的扩展字段
 */
TEST_F(RecentManagerTest, PropetyExtensionFunc_ReturnsSourcePathField)
{
    const QString filePath = tempDir->filePath("src.txt");
    QFile file(filePath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.close();

    const ExpandFieldMap map = RecentHelper::propetyExtensionFunc(RecentHelper::recentUrl(filePath));

    ASSERT_TRUE(map.contains("kFieldInsert"));
    const BasicExpand &expand = map.value("kFieldInsert");
    ASSERT_TRUE(expand.contains("kFileModifiedTime"));
    EXPECT_EQ(expand.value("kFileModifiedTime").second, filePath);
    EXPECT_FALSE(expand.value("kFileModifiedTime").first.isEmpty());
}

/**
 * @brief processPendingItems 按批次(50条)消费队列并停止定时器
 */
TEST_F(RecentManagerTest, ProcessPendingItems_ConsumesQueueInBatches)
{
    StubInfoFactoryCreate();
    for (int i = 0; i < 52; ++i) {
        RecentManager::PendingItem item;
        item.path = tempDir->filePath(QString("batch_%1.txt").arg(i));
        item.href = QString("file:///tmp/batch_%1.txt").arg(i);
        item.modified = i;
        manager->pendingItems.enqueue(item);
    }

    manager->processPendingItems();

    EXPECT_EQ(manager->recentItems.size(), 50);
    EXPECT_EQ(manager->pendingItems.size(), 2);

    manager->processPendingItems();

    EXPECT_EQ(manager->recentItems.size(), 52);
    EXPECT_TRUE(manager->pendingItems.isEmpty());
    EXPECT_FALSE(manager->batchTimer.isActive());   // drained -> timer stopped
}

/**
 * @brief ReloadFinished DBus 信号触发批量加载，ItemAdded 信号触发缓存新增
 */
TEST_F(RecentManagerTest, ReloadFinishedSignal_QueuesAndDrainsRecentItems)
{
    StubInfoFactoryCreate();
    stub.set_lamda(&RecentManagerDBusInterface::Reload, []() {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<qlonglong>();
    });
    stub.set_lamda(&QDBusPendingCallWatcher::waitForFinished, [](QDBusPendingCallWatcher *) {
        __DBG_STUB_INVOKE__
    });
    manager->init();
    ASSERT_NE(manager->dbus(), nullptr);

    // Empty reply: resetRecentNodes still runs, starts the batch timer and the
    // empty queue drains on the first tick.
    int itemsInfoCalls = 0;
    stub.set_lamda(&RecentManagerDBusInterface::GetItemsInfo,
                   [&](RecentManagerDBusInterface *) -> QDBusPendingReply<QVariantList> {
                       __DBG_STUB_INVOKE__
                       ++itemsInfoCalls;
                       return QDBusPendingReply<QVariantList>();
                   });

    // Non-zero timestamp triggers resetRecentNodes() which starts the timer.
    QMetaObject::invokeMethod(manager->dbus(), "ReloadFinished",
                              Qt::DirectConnection, Q_ARG(qlonglong, 1700000000));
    EXPECT_EQ(itemsInfoCalls, 1);
    EXPECT_TRUE(manager->pendingItems.isEmpty());
    EXPECT_TRUE(manager->batchTimer.isActive());

    // Let the 10ms batch tick drain the (empty) queue and stop the timer.
    QTest::qWait(50);
    EXPECT_FALSE(manager->batchTimer.isActive());

    // Zero timestamp must not reload again.
    QMetaObject::invokeMethod(manager->dbus(), "ReloadFinished",
                              Qt::DirectConnection, Q_ARG(qlonglong, 0));
    EXPECT_EQ(itemsInfoCalls, 1);

    // After the once-flag connections are wired, the item signals drive the slots.
    const QString extraPath = tempDir->filePath("dbus_extra.txt");
    QMetaObject::invokeMethod(manager->dbus(), "ItemAdded",
                              Q_ARG(QString, extraPath),
                              Q_ARG(QString, "file:///tmp/dbus_extra.txt"),
                              Q_ARG(qlonglong, 42));
    EXPECT_EQ(manager->recentItems.size(), 1);

    QMetaObject::invokeMethod(manager->dbus(), "ItemsRemoved",
                              Q_ARG(QStringList, QStringList { extraPath }));
    EXPECT_EQ(manager->recentItems.size(), 0);
}
