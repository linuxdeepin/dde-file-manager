// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include <QCoreApplication>
#include <QUrl>
#include <QMetaEnum>
#include <QItemSelection>

// 包含待测试的类
#include "events/detailspaceeventreceiver.h"
#include "utils/detailspacehelper.h"
#include "utils/detailmanager.h"
#include "dfmplugin_detailspace_global.h"

// 包含依赖的头文件
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-framework/event/event.h>

DPDETAILSPACE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

/**
 * @brief DetailSpaceEventReceiver类单元测试
 *
 * 测试范围：
 * 1. 单例模式正确性
 * 2. 事件连接服务初始化
 * 3. 各类事件处理方法
 * 4. 枚举值转换逻辑
 * 5. 选择变化事件处理
 * 6. DetailSpaceHelper集成
 * 7. DetailManager集成
 * 8. 错误处理和边界条件
 */
class DetailSpaceEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Setup test data
        testWindowId = 12345;
        testUrl = QUrl("file:///home/user/test.txt");
        testUrls = { QUrl("file:///home/user/test1.txt"), QUrl("file:///home/user/test2.txt") };
        testScheme = "file";
        testEnums = QStringList() << "kBasicView" << "kFileNameField";
        receiver = &DetailSpaceEventReceiver::instance();
        ASSERT_NE(receiver, nullptr);
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
    DetailSpaceEventReceiver *receiver { nullptr };
    quint64 testWindowId;
    QUrl testUrl;
    QList<QUrl> testUrls;
    QString testScheme;
    QStringList testEnums;
};

/**
 * @brief 测试单例模式
 * 验证DetailSpaceEventReceiver::instance()返回同一个实例
 */
TEST_F(DetailSpaceEventReceiverTest, SingletonPattern_MultipleInstance_ReturnsSameInstance)
{
    DetailSpaceEventReceiver *instance1 = &DetailSpaceEventReceiver::instance();
    DetailSpaceEventReceiver *instance2 = &DetailSpaceEventReceiver::instance();
    DetailSpaceEventReceiver *instance3 = &DetailSpaceEventReceiver::instance();

    // 验证是同一个实例
    EXPECT_EQ(instance1, instance2);
    EXPECT_EQ(instance2, instance3);
    EXPECT_EQ(receiver, instance1);

    // 验证实例不为空
    EXPECT_NE(instance1, nullptr);
}

/**
 * @brief 测试连接服务
 * 验证事件连接服务的正确初始化
 */
TEST_F(DetailSpaceEventReceiverTest, ConnectService_Called_ConnectsAllEvents)
{
    // Mock dpfSlotChannel connect
    typedef void (DetailSpaceEventReceiver::*SigFunc1)(quint64, bool);
    typedef bool (EventChannelManager::*SigType1)(const QString &, const QString &, DetailSpaceEventReceiver *, SigFunc1);
    typedef bool (DetailSpaceEventReceiver::*SigFunc2)(CustomViewExtensionView, int);
    typedef bool (EventChannelManager::*SigType2)(const QString &, const QString &, DetailSpaceEventReceiver *, SigFunc2);
    typedef bool (DetailSpaceEventReceiver::*SigFunc3)(BasicViewFieldFunc, const QString &);
    typedef bool (EventChannelManager::*SigType3)(const QString &, const QString &, DetailSpaceEventReceiver *, SigFunc3);
    typedef bool (DetailSpaceEventReceiver::*SigFunc4)(const QString &, const QStringList &);
    typedef bool (EventChannelManager::*SigType4)(const QString &, const QString &, DetailSpaceEventReceiver *, SigFunc4);
    auto connect1 = static_cast<SigType1>(&EventChannelManager::connect);
    stub.set_lamda(connect1, [] { return true; });
    auto connect2 = static_cast<SigType2>(&EventChannelManager::connect);
    stub.set_lamda(connect2, [] { return true; });
    auto connect3 = static_cast<SigType3>(&EventChannelManager::connect);
    stub.set_lamda(connect3, [] { return true; });
    auto connect4 = static_cast<SigType3>(&EventChannelManager::connect);
    stub.set_lamda(connect4, [] { return true; });

    typedef void (DetailSpaceEventReceiver::*SubFunc)(const quint64, const QItemSelection &, const QItemSelection &);
    typedef bool (EventDispatcherManager::*SubType)(const QString &, const QString &, DetailSpaceEventReceiver *, SubFunc);
    auto subscribe = static_cast<SubType>(&EventDispatcherManager::subscribe);
    stub.set_lamda(subscribe, [] { return true; });

    EXPECT_NO_FATAL_FAILURE(receiver->connectService());
}

/**
 * @brief 测试标题栏显示详情视图处理
 * 验证标题栏显示详情视图事件的处理
 */
TEST_F(DetailSpaceEventReceiverTest, HandleTileBarShowDetailView_ValidParameters_CallsDetailSpaceHelper)
{
    bool showDetailViewCalled = false;
    quint64 receivedWindowId = 0;
    bool receivedChecked = false;

    // Mock DetailSpaceHelper::showDetailView
    stub.set_lamda(&DetailSpaceHelper::showDetailView, [&showDetailViewCalled, &receivedWindowId, &receivedChecked](quint64 windowId, bool checked) {
        __DBG_STUB_INVOKE__
        showDetailViewCalled = true;
        receivedWindowId = windowId;
        receivedChecked = checked;
    });

    receiver->handleTileBarShowDetailView(testWindowId, true);

    EXPECT_TRUE(showDetailViewCalled);
    EXPECT_EQ(receivedWindowId, testWindowId);
    EXPECT_TRUE(receivedChecked);
}

/**
 * @brief 测试设置选择处理
 * 验证设置选择事件的处理
 */
TEST_F(DetailSpaceEventReceiverTest, HandleSetSelect_ValidParameters_CallsDetailSpaceHelper)
{
    bool setDetailViewSelectFileUrlCalled = false;
    quint64 receivedWindowId = 0;
    QUrl receivedUrl;

    // Mock DetailSpaceHelper::setDetailViewSelectFileUrl
    stub.set_lamda(&DetailSpaceHelper::setDetailViewSelectFileUrl, [&setDetailViewSelectFileUrlCalled, &receivedWindowId, &receivedUrl](quint64 windowId, const QUrl &url) {
        __DBG_STUB_INVOKE__
        setDetailViewSelectFileUrlCalled = true;
        receivedWindowId = windowId;
        receivedUrl = url;
    });

    receiver->handleSetSelect(testWindowId, testUrl);

    EXPECT_TRUE(setDetailViewSelectFileUrlCalled);
    EXPECT_EQ(receivedWindowId, testWindowId);
    EXPECT_EQ(receivedUrl, testUrl);
}

/**
 * @brief 测试扩展视图注册处理
 * 验证扩展视图注册事件的处理
 */
TEST_F(DetailSpaceEventReceiverTest, HandleViewExtensionRegister_ValidFunction_CallsDetailManager)
{
    bool registerExtensionViewCalled = false;
    int receivedIndex = -1;

    // Mock DetailManager::registerExtensionView
    stub.set_lamda(&DetailManager::registerExtensionView, [&registerExtensionViewCalled, &receivedIndex](DetailManager *, CustomViewExtensionView, int index) {
        __DBG_STUB_INVOKE__
        registerExtensionViewCalled = true;
        receivedIndex = index;
        return true;
    });

    CustomViewExtensionView testFunc = [](const QUrl &) -> QWidget* {
        return new QWidget();
    };

    bool result = receiver->handleViewExtensionRegister(testFunc, 5);

    EXPECT_TRUE(result);
    EXPECT_TRUE(registerExtensionViewCalled);
    EXPECT_EQ(receivedIndex, 5);
}

/**
 * @brief 测试基本视图扩展注册处理
 * 验证基本视图扩展注册事件的处理
 */
TEST_F(DetailSpaceEventReceiverTest, HandleBasicViewExtensionRegister_ValidParameters_CallsDetailManager)
{
    bool registerBasicViewExtensionCalled = false;
    QString receivedScheme;

    // Mock DetailManager::registerBasicViewExtension
    stub.set_lamda(&DetailManager::registerBasicViewExtension, [&registerBasicViewExtensionCalled, &receivedScheme](DetailManager *, const QString &scheme, BasicViewFieldFunc) {
        __DBG_STUB_INVOKE__
        registerBasicViewExtensionCalled = true;
        receivedScheme = scheme;
        return true;
    });

    BasicViewFieldFunc testFunc = [](const QUrl &) {
        return QMap<QString, QMultiMap<QString, QPair<QString, QString>>>();
    };

    bool result = receiver->handleBasicViewExtensionRegister(testFunc, testScheme);

    EXPECT_TRUE(result);
    EXPECT_TRUE(registerBasicViewExtensionCalled);
    EXPECT_EQ(receivedScheme, testScheme);
}

/**
 * @brief 测试根基本视图扩展注册处理
 * 验证根基本视图扩展注册事件的处理
 */
TEST_F(DetailSpaceEventReceiverTest, HandleBasicViewExtensionRootRegister_ValidParameters_CallsDetailManager)
{
    bool registerBasicViewExtensionRootCalled = false;
    QString receivedScheme;

    // Mock DetailManager::registerBasicViewExtensionRoot
    stub.set_lamda(&DetailManager::registerBasicViewExtensionRoot, [&registerBasicViewExtensionRootCalled, &receivedScheme](DetailManager *, const QString &scheme, BasicViewFieldFunc) {
        __DBG_STUB_INVOKE__
        registerBasicViewExtensionRootCalled = true;
        receivedScheme = scheme;
        return true;
    });

    BasicViewFieldFunc testFunc = [](const QUrl &) {
        return QMap<QString, QMultiMap<QString, QPair<QString, QString>>>();
    };

    bool result = receiver->handleBasicViewExtensionRootRegister(testFunc, testScheme);

    EXPECT_TRUE(result);
    EXPECT_TRUE(registerBasicViewExtensionRootCalled);
    EXPECT_EQ(receivedScheme, testScheme);
}

/**
 * @brief 测试基本字段过滤器添加处理
 * 验证基本字段过滤器添加事件的处理
 */
TEST_F(DetailSpaceEventReceiverTest, HandleBasicFiledFilterAdd_ValidEnums_CallsDetailManager)
{
    bool addBasicFiledFiltersCalled = false;
    QString receivedScheme;
    DetailFilterType receivedFilters = kNotFilter;

    // Mock QMetaEnum keysToValue
    stub.set_lamda(&QMetaEnum::keysToValue, [](const QMetaEnum *, const char *, bool *ok) {
        __DBG_STUB_INVOKE__
        *ok = true;
        return static_cast<int>(kBasicView | kFileNameField);
    });

    // Mock DetailManager::addBasicFiledFiltes
    stub.set_lamda(&DetailManager::addBasicFiledFiltes, [&addBasicFiledFiltersCalled, &receivedScheme, &receivedFilters](DetailManager *, const QString &scheme, DetailFilterType filters) {
        __DBG_STUB_INVOKE__
        addBasicFiledFiltersCalled = true;
        receivedScheme = scheme;
        receivedFilters = filters;
        return true;
    });

    bool result = receiver->handleBasicFiledFilterAdd(testScheme, testEnums);

    EXPECT_TRUE(result);
    EXPECT_TRUE(addBasicFiledFiltersCalled);
    EXPECT_EQ(receivedScheme, testScheme);
    EXPECT_NE(receivedFilters, kNotFilter);
}

/**
 * @brief 测试基本字段过滤器添加处理 - 无效枚举
 * 验证无效枚举的处理
 */
TEST_F(DetailSpaceEventReceiverTest, HandleBasicFiledFilterAdd_InvalidEnums_ReturnsFalse)
{
    bool addBasicFiledFiltersCalled = false;

    // Mock QMetaEnum keysToValue to return invalid
    stub.set_lamda(&QMetaEnum::keysToValue, [](const QMetaEnum *, const char *, bool *ok) {
        __DBG_STUB_INVOKE__
        *ok = false;
        return 0;
    });

    stub.set_lamda(&DetailManager::addBasicFiledFiltes, [&addBasicFiledFiltersCalled](DetailManager *, const QString &, DetailFilterType) {
        __DBG_STUB_INVOKE__
        addBasicFiledFiltersCalled = true;
        return true;
    });

    bool result = receiver->handleBasicFiledFilterAdd(testScheme, testEnums);

    EXPECT_FALSE(result);
    EXPECT_FALSE(addBasicFiledFiltersCalled);
}

/**
 * @brief 测试根基本字段过滤器添加处理
 * 验证根基本字段过滤器添加事件的处理
 */
TEST_F(DetailSpaceEventReceiverTest, HandleBasicFiledFilterRootAdd_ValidEnums_CallsDetailManager)
{
    bool addRootBasicFiledFiltersCalled = false;
    QString receivedScheme;

    // Mock QMetaEnum keysToValue
    stub.set_lamda(&QMetaEnum::keysToValue, [](const QMetaEnum *, const char *, bool *ok) {
        __DBG_STUB_INVOKE__
        *ok = true;
        return static_cast<int>(kBasicView | kIconView);
    });

    // Mock DetailManager::addRootBasicFiledFiltes
    stub.set_lamda(&DetailManager::addRootBasicFiledFiltes, [&addRootBasicFiledFiltersCalled, &receivedScheme](DetailManager *, const QString &scheme, DetailFilterType) {
        __DBG_STUB_INVOKE__
        addRootBasicFiledFiltersCalled = true;
        receivedScheme = scheme;
        return true;
    });

    bool result = receiver->handleBasicFiledFilterRootAdd(testScheme, testEnums);

    EXPECT_TRUE(result);
    EXPECT_TRUE(addRootBasicFiledFiltersCalled);
    EXPECT_EQ(receivedScheme, testScheme);
}

/**
 * @brief 测试根基本字段过滤器添加处理 - 无效枚举
 * 验证无效枚举的处理
 */
TEST_F(DetailSpaceEventReceiverTest, HandleBasicFiledFilterRootAdd_InvalidEnums_ReturnsFalse)
{
    bool addRootBasicFiledFiltersCalled = false;

    // Mock QMetaEnum keysToValue to return invalid
    stub.set_lamda(&QMetaEnum::keysToValue, [](const QMetaEnum *, const char *, bool *ok) {
        __DBG_STUB_INVOKE__
        *ok = false;
        return 0;
    });

    stub.set_lamda(&DetailManager::addRootBasicFiledFiltes, [&addRootBasicFiledFiltersCalled](DetailManager *, const QString &, DetailFilterType) {
        __DBG_STUB_INVOKE__
        addRootBasicFiledFiltersCalled = true;
        return true;
    });

    bool result = receiver->handleBasicFiledFilterRootAdd(testScheme, testEnums);

    EXPECT_FALSE(result);
    EXPECT_FALSE(addRootBasicFiledFiltersCalled);
}

/**
 * @brief 测试视图选择变化处理 - 有选中URL
 * 验证有选中URL时的处理
 */
TEST_F(DetailSpaceEventReceiverTest, HandleViewSelectionChanged_WithSelectedUrls_UsesFirstUrl)
{
    bool setDetailViewSelectFileUrlCalled = false;
    quint64 receivedWindowId = 0;
    QUrl receivedUrl;

    // Mock dpfSlotChannel push to return selected URLs
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [this] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(testUrls);
    });

    // Mock DetailSpaceHelper::setDetailViewSelectFileUrl
    stub.set_lamda(&DetailSpaceHelper::setDetailViewSelectFileUrl, [&setDetailViewSelectFileUrlCalled, &receivedWindowId, &receivedUrl](quint64 windowId, const QUrl &url) {
        __DBG_STUB_INVOKE__
        setDetailViewSelectFileUrlCalled = true;
        receivedWindowId = windowId;
        receivedUrl = url;
    });

    QItemSelection selected;
    QItemSelection deselected;
    receiver->handleViewSelectionChanged(testWindowId, selected, deselected);

    EXPECT_TRUE(setDetailViewSelectFileUrlCalled);
    EXPECT_EQ(receivedWindowId, testWindowId);
    EXPECT_EQ(receivedUrl, testUrls.first());
}

/**
 * @brief 测试视图选择变化处理 - 无选中URL
 * 验证无选中URL时的处理
 */
TEST_F(DetailSpaceEventReceiverTest, HandleViewSelectionChanged_WithoutSelectedUrls_UsesCurrentUrl)
{
    bool setDetailViewSelectFileUrlCalled = false;
    bool findWindowByIdCalled = false;
    quint64 receivedWindowId = 0;
    QUrl receivedUrl;

    // Mock dpfSlotChannel push to return empty URL list
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), []() {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(QList<QUrl>());
    });

    // Mock FMWindowsIns findWindowById
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [&findWindowByIdCalled, this](FileManagerWindowsManager *, quint64) {
        __DBG_STUB_INVOKE__
        findWindowByIdCalled = true;
        auto mockWindow = reinterpret_cast<FileManagerWindow*>(0x1234);
        return mockWindow;
    });

    // Mock window currentUrl
    stub.set_lamda(ADDR(FileManagerWindow, currentUrl), [this](FileManagerWindow *) {
        __DBG_STUB_INVOKE__
        return testUrl;
    });

    // Mock DetailSpaceHelper::setDetailViewSelectFileUrl
    stub.set_lamda(&DetailSpaceHelper::setDetailViewSelectFileUrl, [&setDetailViewSelectFileUrlCalled, &receivedWindowId, &receivedUrl](quint64 windowId, const QUrl &url) {
        __DBG_STUB_INVOKE__
        setDetailViewSelectFileUrlCalled = true;
        receivedWindowId = windowId;
        receivedUrl = url;
    });

    QItemSelection selected;
    QItemSelection deselected;
    receiver->handleViewSelectionChanged(testWindowId, selected, deselected);

    EXPECT_TRUE(findWindowByIdCalled);
    EXPECT_TRUE(setDetailViewSelectFileUrlCalled);
    EXPECT_EQ(receivedWindowId, testWindowId);
    EXPECT_EQ(receivedUrl, testUrl);
}

/**
 * @brief 测试视图选择变化处理 - 窗口未找到
 * 验证窗口未找到时的处理
 */
TEST_F(DetailSpaceEventReceiverTest, HandleViewSelectionChanged_WindowNotFound_HandledSafely)
{
    bool setDetailViewSelectFileUrlCalled = false;

    // Mock dpfSlotChannel push to return empty URL list
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(QList<QUrl>());
    });

    // Mock FMWindowsIns findWindowById to return null
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [](FileManagerWindowsManager *, quint64) {
        __DBG_STUB_INVOKE__
        return static_cast<FileManagerWindow*>(nullptr);
    });

    stub.set_lamda(&DetailSpaceHelper::setDetailViewSelectFileUrl, [&setDetailViewSelectFileUrlCalled](quint64, const QUrl &) {
        __DBG_STUB_INVOKE__
        setDetailViewSelectFileUrlCalled = true;
    });

    QItemSelection selected;
    QItemSelection deselected;

    // This should not crash
    EXPECT_NO_FATAL_FAILURE(receiver->handleViewSelectionChanged(testWindowId, selected, deselected));
    EXPECT_FALSE(setDetailViewSelectFileUrlCalled);
}

/**
 * @brief 测试构造函数
 * 验证DetailSpaceEventReceiver构造函数正确初始化
 */
TEST_F(DetailSpaceEventReceiverTest, Constructor_WithParent_ObjectCreatedSuccessfully)
{
    // 验证单例实例正确创建
    EXPECT_NE(receiver, nullptr);

    // 验证对象继承正确
    EXPECT_NE(dynamic_cast<QObject*>(receiver), nullptr);
}

/**
 * @brief 测试多个事件处理的组合
 * 验证多个事件处理方法的组合使用
 */
TEST_F(DetailSpaceEventReceiverTest, MultipleEventHandling_CombinedUsage_WorksCorrectly)
{
    bool showDetailViewCalled = false;
    bool setSelectCalled = false;
    bool registerViewCalled = false;

    // Mock various DetailSpaceHelper and DetailManager methods
    stub.set_lamda(&DetailSpaceHelper::showDetailView, [&showDetailViewCalled](quint64, bool) {
        __DBG_STUB_INVOKE__
        showDetailViewCalled = true;
    });

    stub.set_lamda(&DetailSpaceHelper::setDetailViewSelectFileUrl, [&setSelectCalled](quint64, const QUrl &) {
        __DBG_STUB_INVOKE__
        setSelectCalled = true;
    });

    stub.set_lamda(&DetailManager::registerExtensionView, [&registerViewCalled](DetailManager *, CustomViewExtensionView, int) {
        __DBG_STUB_INVOKE__
        registerViewCalled = true;
        return true;
    });

    // Execute multiple event handlers
    receiver->handleTileBarShowDetailView(testWindowId, true);
    receiver->handleSetSelect(testWindowId, testUrl);

    CustomViewExtensionView testFunc = [](const QUrl &) -> QWidget* { return nullptr; };
    receiver->handleViewExtensionRegister(testFunc, 0);

    EXPECT_TRUE(showDetailViewCalled);
    EXPECT_TRUE(setSelectCalled);
    EXPECT_TRUE(registerViewCalled);
}
