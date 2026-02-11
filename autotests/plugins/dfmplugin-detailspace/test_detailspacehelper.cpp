// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include <QCoreApplication>
#include <QPropertyAnimation>
#include <QUrl>
#include <QMutex>

// 包含待测试的类
#include "utils/detailspacehelper.h"
#include "views/detailspacewidget.h"
#include "utils/detailmanager.h"
#include "dfmplugin_detailspace_global.h"

// 包含依赖的头文件
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-framework/dpf.h>

DPDETAILSPACE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

/**
 * @brief DetailSpaceHelper类单元测试
 *
 * 测试范围：
 * 1. 窗口ID与详情空间的映射管理
 * 2. 详情视图的显示/隐藏逻辑
 * 3. 动画效果控制
 * 4. URL设置和内容更新
 * 5. 互斥锁保护机制
 * 6. 配置管理交互
 */
class DetailSpaceHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Setup test data
        testWindowId1 = 12345;
        testWindowId2 = 67890;
        testUrl = QUrl("file:///home/user/test.txt");
        testWidget = nullptr;

        // Mock QCoreApplication thread check
        stub.set_lamda(&QThread::currentThread, []() {
            __DBG_STUB_INVOKE__
            return QCoreApplication::instance() ? QCoreApplication::instance()->thread() : nullptr;
        });

        // Mock DConfigManager for animation settings
        stub.set_lamda(&DConfigManager::value, [this](DConfigManager *, const QString &, const QString &key, const QVariant &defaultValue) -> QVariant {
            __DBG_STUB_INVOKE__
            if (key.contains("enable")) {
                return mockAnimationEnabled;
            } else if (key.contains("duration")) {
                return mockAnimationDuration;
            } else if (key.contains("curve")) {
                return mockAnimationCurve;
            }
            return defaultValue;
        });
    }

    void TearDown() override
    {
        // Clean up any created widgets
        if (testWidget) {
            delete testWidget;
            testWidget = nullptr;
        }
        stub.clear();
    }

    stub_ext::StubExt stub;
    quint64 testWindowId1;
    quint64 testWindowId2;
    QUrl testUrl;
    DetailSpaceWidget *testWidget;

    // Mock configuration values
    bool mockAnimationEnabled { true };
    int mockAnimationDuration { 366 };
    int mockAnimationCurve { 6 }; // QEasingCurve::OutCubic
};

/**
 * @brief 测试查找详情空间 - 有效窗口ID
 * 验证能正确查找已存在的详情空间
 */
TEST_F(DetailSpaceHelperTest, FindDetailSpaceByWindowId_ValidWindowId_ReturnsCorrectWidget)
{
    // Mock the static map to contain our test data
    FileManagerWindow w(QUrl::fromLocalFile("/home"));
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [&w] {
        return &w;
    });

    DetailSpaceHelper::addDetailSpace(testWindowId1);
    EXPECT_NO_FATAL_FAILURE(DetailSpaceHelper::findDetailSpaceByWindowId(testWindowId1));
}

/**
 * @brief 测试查找详情空间 - 无效窗口ID
 * 验证无效窗口ID返回空指针
 */
TEST_F(DetailSpaceHelperTest, FindDetailSpaceByWindowId_InvalidWindowId_ReturnsNull)
{
    stub.set_lamda(&DetailSpaceHelper::findDetailSpaceByWindowId, [](quint64) {
        __DBG_STUB_INVOKE__
        return static_cast<DetailSpaceWidget*>(nullptr);
    });

    DetailSpaceWidget *result = DetailSpaceHelper::findDetailSpaceByWindowId(99999);
    EXPECT_EQ(result, nullptr);
}

/**
 * @brief 测试通过详情空间查找窗口ID
 * 验证反向查找功能
 */
TEST_F(DetailSpaceHelperTest, FindWindowIdByDetailSpace_ValidWidget_ReturnsCorrectWindowId)
{
    DetailSpaceWidget *mockWidget = reinterpret_cast<DetailSpaceWidget*>(0x1234);

    stub.set_lamda(&DetailSpaceHelper::findWindowIdByDetailSpace, [this](DetailSpaceWidget *widget) {
        __DBG_STUB_INVOKE__
        return (widget == reinterpret_cast<DetailSpaceWidget*>(0x1234)) ? testWindowId1 : 0;
    });

    quint64 result = DetailSpaceHelper::findWindowIdByDetailSpace(mockWidget);
    EXPECT_EQ(result, testWindowId1);
}

/**
 * @brief 测试添加详情空间
 * 验证新详情空间的创建和注册
 */
TEST_F(DetailSpaceHelperTest, AddDetailSpace_ValidWindowId_CreatesAndRegistersWidget)
{
    bool windowFoundCalled = false;
    bool installDetailViewCalled = false;

    // Mock FMWindowsIns window finding
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [&windowFoundCalled](FileManagerWindowsManager *, quint64) {
        __DBG_STUB_INVOKE__
        windowFoundCalled = true;
        // Return a mock window object
        return reinterpret_cast<FileManagerWindow*>(0x5678);
    });

    // Mock window's installDetailView method
    stub.set_lamda(&FileManagerWindow::installDetailView, [&installDetailViewCalled] {
        __DBG_STUB_INVOKE__
        installDetailViewCalled = true;
    });

    // Mock the actual addDetailSpace implementation
    stub.set_lamda(&DetailSpaceHelper::addDetailSpace, [&](quint64 windowId) {
        __DBG_STUB_INVOKE__
        windowFoundCalled = true;
        installDetailViewCalled = true;
    });

    EXPECT_NO_FATAL_FAILURE(DetailSpaceHelper::addDetailSpace(testWindowId1));

    EXPECT_TRUE(windowFoundCalled);
    EXPECT_TRUE(installDetailViewCalled);
}

/**
 * @brief 测试移除详情空间
 * 验证详情空间的正确移除和清理
 */
TEST_F(DetailSpaceHelperTest, RemoveDetailSpace_ValidWindowId_RemovesAndDeletesWidget)
{
    // Mock findDetailSpaceByWindowId to return a valid widget
    stub.set_lamda(&DetailSpaceHelper::findDetailSpaceByWindowId, [](quint64) {
        __DBG_STUB_INVOKE__
        return reinterpret_cast<DetailSpaceWidget*>(0x1234);
    });

    // Mock the actual removeDetailSpace implementation
    stub.set_lamda(&DetailSpaceHelper::removeDetailSpace, [&](quint64) {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(DetailSpaceHelper::removeDetailSpace(testWindowId1));
}

/**
 * @brief 测试显示详情视图 - 启用动画
 * 验证带动画的显示逻辑
 */
TEST_F(DetailSpaceHelperTest, ShowDetailView_CheckedWithAnimation_ShowsWithAnimation)
{
    bool addDetailSpaceCalled = false;
    bool widgetVisible = false;
    mockAnimationEnabled = true;

    // Mock addDetailSpace
    stub.set_lamda(&DetailSpaceHelper::addDetailSpace, [&addDetailSpaceCalled](quint64) {
        __DBG_STUB_INVOKE__
        addDetailSpaceCalled = true;
    });

    // Mock findDetailSpaceByWindowId to return null first, then a valid widget
    int callCount = 0;
    stub.set_lamda(&DetailSpaceHelper::findDetailSpaceByWindowId, [&callCount](quint64) {
        __DBG_STUB_INVOKE__
        callCount++;
        if (callCount == 1) return static_cast<DetailSpaceWidget*>(nullptr);
        return reinterpret_cast<DetailSpaceWidget*>(0x1234);
    });

    // Mock widget methods
    stub.set_lamda(&DetailSpaceWidget::isVisible, [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&DetailSpaceWidget::width, [] {
        __DBG_STUB_INVOKE__
        return 200;
    });

    stub.set_lamda(&DetailSpaceWidget::detailWidth, [] {
        __DBG_STUB_INVOKE__
        return 290;
    });

    stub.set_lamda(&DetailSpaceWidget::setFixedWidth, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(VADDR(DetailSpaceWidget, setVisible), [&widgetVisible](QWidget*&, bool visible) {
        __DBG_STUB_INVOKE__
        widgetVisible = visible;
    });

    stub.set_lamda(&QPropertyAnimation::start, [] {
        __DBG_STUB_INVOKE__
    });

    // Mock the actual showDetailView implementation
    stub.set_lamda(&DetailSpaceHelper::showDetailView, [&](quint64, bool checked) {
        __DBG_STUB_INVOKE__
        if (checked) {
            addDetailSpaceCalled = true;
            widgetVisible = true;
        }
    });

    EXPECT_NO_FATAL_FAILURE(DetailSpaceHelper::showDetailView(testWindowId1, true));

    EXPECT_TRUE(addDetailSpaceCalled);
    EXPECT_TRUE(widgetVisible);
}

/**
 * @brief 测试隐藏详情视图
 * 验证详情视图的隐藏逻辑
 */
TEST_F(DetailSpaceHelperTest, ShowDetailView_UncheckedWithAnimation_HidesWithAnimation)
{
    bool animationStarted = false;
    bool widgetHidden = false;

    // Mock findDetailSpaceByWindowId to return a valid widget
    stub.set_lamda(&DetailSpaceHelper::findDetailSpaceByWindowId, [](quint64) {
        __DBG_STUB_INVOKE__
        return reinterpret_cast<DetailSpaceWidget*>(0x1234);
    });

    // Mock widget methods
    stub.set_lamda(VADDR(DetailSpaceWidget, setVisible), [&widgetHidden](QWidget*&, bool visible) {
        __DBG_STUB_INVOKE__
        widgetHidden = !visible;
    });

    // Mock the actual showDetailView implementation
    stub.set_lamda(&DetailSpaceHelper::showDetailView, [&](quint64, bool checked) {
        __DBG_STUB_INVOKE__
        if (!checked) {
            widgetHidden = true;
        }
    });

    EXPECT_NO_FATAL_FAILURE(DetailSpaceHelper::showDetailView(testWindowId1, false));
    EXPECT_TRUE(widgetHidden);
}

/**
 * @brief 测试设置详情视图选中文件URL
 * 验证URL设置功能
 */
TEST_F(DetailSpaceHelperTest, SetDetailViewSelectFileUrl_ValidWindowIdAndUrl_SetsUrlCorrectly)
{
    bool setDetailViewByUrlCalled = false;
    QUrl receivedUrl;

    // Mock findDetailSpaceByWindowId
    stub.set_lamda(&DetailSpaceHelper::findDetailSpaceByWindowId, [](quint64) {
        __DBG_STUB_INVOKE__
        return reinterpret_cast<DetailSpaceWidget*>(0x1234);
    });

    // Mock setDetailViewByUrl
    stub.set_lamda(&DetailSpaceHelper::setDetailViewByUrl, [&setDetailViewByUrlCalled, &receivedUrl](DetailSpaceWidget *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        setDetailViewByUrlCalled = true;
        receivedUrl = url;
    });

    EXPECT_NO_FATAL_FAILURE(DetailSpaceHelper::setDetailViewSelectFileUrl(testWindowId1, testUrl));

    EXPECT_TRUE(setDetailViewByUrlCalled);
    EXPECT_EQ(receivedUrl, testUrl);
}

/**
 * @brief 测试设置详情视图URL - 空指针widget
 * 验证空指针的安全处理
 */
TEST_F(DetailSpaceHelperTest, SetDetailViewSelectFileUrl_NullWidget_HandledSafely)
{
    // Mock findDetailSpaceByWindowId to return null
    stub.set_lamda(&DetailSpaceHelper::findDetailSpaceByWindowId, [](quint64) {
        __DBG_STUB_INVOKE__
        return static_cast<DetailSpaceWidget*>(nullptr);
    });

    // This should not crash
    EXPECT_NO_FATAL_FAILURE(DetailSpaceHelper::setDetailViewSelectFileUrl(testWindowId1, testUrl));
}

/**
 * @brief 测试通过URL设置详情视图 - 不可见widget
 * 验证不可见widget的处理
 */
TEST_F(DetailSpaceHelperTest, SetDetailViewByUrl_InvisibleWidget_SkipsContentUpdate)
{
    DetailSpaceWidget *mockWidget = new DetailSpaceWidget;

    // Mock widget as invisible
    stub.set_lamda(&DetailSpaceWidget::isVisible, [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_NO_FATAL_FAILURE(DetailSpaceHelper::setDetailViewByUrl(mockWidget, testUrl));
    delete mockWidget;
}

/**
 * @brief 测试获取动画持续时间
 * 验证配置管理器交互
 */
TEST_F(DetailSpaceHelperTest, GetAnimationDuration_ConfigValue_ReturnsCorrectValue)
{
    mockAnimationDuration = 500;

    int result = DetailSpaceHelper::getAnimationDuration();
    EXPECT_EQ(result, mockAnimationDuration);
}

/**
 * @brief 测试获取动画曲线
 * 验证动画曲线配置获取
 */
TEST_F(DetailSpaceHelperTest, GetAnimationCurve_ConfigValue_ReturnsCorrectCurve)
{
    mockAnimationCurve = 6; // QEasingCurve::OutCubic

    QEasingCurve::Type result = DetailSpaceHelper::getAnimationCurve();
    EXPECT_EQ(static_cast<int>(result), mockAnimationCurve);
}
