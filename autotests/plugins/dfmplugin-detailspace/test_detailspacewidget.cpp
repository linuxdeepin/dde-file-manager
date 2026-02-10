// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include <QCoreApplication>
#include <QUrl>
#include <QWidget>
#include <QHBoxLayout>

// 包含待测试的类
#include "views/detailspacewidget.h"
#include "views/detailview.h"
#include "utils/detailspacehelper.h"
#include "dfmplugin_detailspace_global.h"

// 包含依赖的头文件
#include <dfm-base/interfaces/abstractframe.h>
#include <dfm-framework/event/event.h>

#ifdef DTKWIDGET_CLASS_DSizeMode
#include <DGuiApplicationHelper>
#include <DSizeMode>
#endif

DPDETAILSPACE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

/**
 * @brief DetailSpaceWidget类单元测试
 *
 * 测试范围：
 * 1. 构造函数和UI初始化
 * 2. URL设置和获取
 * 3. 扩展控件插入管理
 * 4. 尺寸模式适配
 * 5. 可见性状态处理
 * 6. 控件移除功能
 * 7. 详情宽度计算
 */
class DetailSpaceWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Setup test data
        testUrl = QUrl("file:///home/user/test.txt");
        testWindowId = 12345;
        mockWidget = reinterpret_cast<QWidget*>(0x5678);
        widget = nullptr;
    }

    void TearDown() override
    {
        if (widget) {
            delete widget;
            widget = nullptr;
        }
        stub.clear();
    }

    stub_ext::StubExt stub;
    DetailSpaceWidget *widget { nullptr };
    QUrl testUrl;
    quint64 testWindowId;
    QWidget *mockWidget;
};

/**
 * @brief 测试构造函数
 * 验证DetailSpaceWidget能够正确构造和初始化
 */
TEST_F(DetailSpaceWidgetTest, Constructor_DefaultConstruction_ObjectCreatedSuccessfully)
{
    bool uiInitialized = false;
    bool sizeModeInitialized = false;

    // Mock the initialization methods
    stub.set_lamda(ADDR(DetailSpaceWidget, initializeUi), [&uiInitialized](DetailSpaceWidget *) {
        __DBG_STUB_INVOKE__
        uiInitialized = true;
    });

    stub.set_lamda(ADDR(DetailSpaceWidget, initUiForSizeMode), [&sizeModeInitialized](DetailSpaceWidget *) {
        __DBG_STUB_INVOKE__
        sizeModeInitialized = true;
    });

    // Create widget
    widget = new DetailSpaceWidget();
    EXPECT_NE(widget, nullptr);

    // Verify initialization was called
    EXPECT_TRUE(uiInitialized);
    EXPECT_TRUE(sizeModeInitialized);

    // Verify inheritance
    EXPECT_NE(dynamic_cast<AbstractFrame*>(widget), nullptr);
}

/**
 * @brief 测试尺寸模式初始化
 * 验证尺寸模式适配功能
 */
TEST_F(DetailSpaceWidgetTest, InitUiForSizeMode_Called_SetsCorrectWidth)
{
    int detailWidth = 290;
    bool setFixedWidthCalled = false;
    int receivedWidth = 0;

    // Mock detailWidth method
    stub.set_lamda(ADDR(DetailSpaceWidget, detailWidth), [detailWidth](DetailSpaceWidget *) {
        __DBG_STUB_INVOKE__
        return detailWidth;
    });

    // Mock setFixedWidth method
    stub.set_lamda(ADDR(DetailSpaceWidget, setFixedWidth), [&setFixedWidthCalled, &receivedWidth](QWidget *, int width) {
        __DBG_STUB_INVOKE__
        setFixedWidthCalled = true;
        receivedWidth = width;
    });

    widget = new DetailSpaceWidget();
    widget->initUiForSizeMode();

    EXPECT_TRUE(setFixedWidthCalled);
    EXPECT_EQ(receivedWidth, detailWidth);
}

/**
 * @brief 测试设置当前URL - 基本版本
 * 验证URL设置的基本功能
 */
TEST_F(DetailSpaceWidgetTest, SetCurrentUrl_BasicVersion_CallsExtendedVersion)
{
    bool extendedVersionCalled = false;
    QUrl receivedUrl;
    int receivedFilter = -1;

    // Mock the extended setCurrentUrl version
    using SetCurrentUrlFunc = void (DetailSpaceWidget::*)(const QUrl &, int);
    stub.set_lamda(static_cast<SetCurrentUrlFunc>(&DetailSpaceWidget::setCurrentUrl),
                   [&extendedVersionCalled, &receivedUrl, &receivedFilter](DetailSpaceWidget *, const QUrl &url, int filter) {
        __DBG_STUB_INVOKE__
        extendedVersionCalled = true;
        receivedUrl = url;
        receivedFilter = filter;
    });

    // Mock DetailSpaceHelper
    stub.set_lamda(&DetailSpaceHelper::findWindowIdByDetailSpace, [this](DetailSpaceWidget *) {
        __DBG_STUB_INVOKE__
        return testWindowId;
    });

    widget = new DetailSpaceWidget();
    widget->setCurrentUrl(testUrl);

    EXPECT_TRUE(extendedVersionCalled);
    EXPECT_EQ(receivedUrl, testUrl);
    EXPECT_EQ(receivedFilter, 0);
}

/**
 * @brief 测试设置当前URL - 有选中文件
 * 验证有选中文件时的URL设置
 */
TEST_F(DetailSpaceWidgetTest, SetCurrentUrl_WithSelectedUrls_UsesSelectedUrl)
{
    QUrl selectedUrl("file:///home/user/selected.txt");
    bool extendedVersionCalled = false;
    QUrl receivedUrl;

    // Mock dpfSlotChannel to return selected URLs
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [selectedUrl] {
        __DBG_STUB_INVOKE__
        QList<QUrl> urls;
        urls << selectedUrl;
        return QVariant::fromValue(urls);
    });

    // Mock DetailSpaceHelper
    stub.set_lamda(&DetailSpaceHelper::findWindowIdByDetailSpace, [this](DetailSpaceWidget *) {
        __DBG_STUB_INVOKE__
        return testWindowId;
    });

    // Mock the extended setCurrentUrl version
    using SetCurrentUrlFunc = void (DetailSpaceWidget::*)(const QUrl &, int);
    stub.set_lamda(static_cast<SetCurrentUrlFunc>(&DetailSpaceWidget::setCurrentUrl),
                   [&extendedVersionCalled, &receivedUrl](DetailSpaceWidget *, const QUrl &url, int) {
        __DBG_STUB_INVOKE__
        extendedVersionCalled = true;
        receivedUrl = url;
    });

    widget = new DetailSpaceWidget();
    widget->setCurrentUrl(testUrl);

    EXPECT_TRUE(extendedVersionCalled);
    EXPECT_EQ(receivedUrl, selectedUrl);
}

/**
 * @brief 测试设置当前URL - 扩展版本
 * 验证带过滤器的URL设置
 */
TEST_F(DetailSpaceWidgetTest, SetCurrentUrl_ExtendedVersion_SetsUrlAndUpdatesView)
{
    bool removeControlsCalled = false;
    bool detailViewSetUrlCalled = false;
    QUrl receivedUrl;
    int receivedFilter = -1;

    // Mock widget visibility
    stub.set_lamda(ADDR(DetailSpaceWidget, isVisible), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock removeControls
    stub.set_lamda(ADDR(DetailSpaceWidget, removeControls), [&removeControlsCalled](DetailSpaceWidget *) {
        __DBG_STUB_INVOKE__
        removeControlsCalled = true;
    });

    // Mock DetailView setUrl
    stub.set_lamda(ADDR(DetailView, setUrl), [&detailViewSetUrlCalled, &receivedUrl, &receivedFilter](DetailView *, const QUrl &url, int filter) {
        __DBG_STUB_INVOKE__
        detailViewSetUrlCalled = true;
        receivedUrl = url;
        receivedFilter = filter;
    });

    widget = new DetailSpaceWidget();
    widget->setCurrentUrl(testUrl, 5);

    EXPECT_TRUE(removeControlsCalled);
    EXPECT_TRUE(detailViewSetUrlCalled);
    EXPECT_EQ(receivedUrl, testUrl);
    EXPECT_EQ(receivedFilter, 5);
}

/**
 * @brief 测试设置当前URL - 不可见widget
 * 验证不可见时跳过视图更新
 */
TEST_F(DetailSpaceWidgetTest, SetCurrentUrl_InvisibleWidget_SkipsViewUpdate)
{
    bool removeControlsCalled = false;
    bool detailViewSetUrlCalled = false;

    // Mock widget as invisible
    stub.set_lamda(ADDR(DetailSpaceWidget, isVisible), [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(ADDR(DetailSpaceWidget, removeControls), [&removeControlsCalled](DetailSpaceWidget *) {
        __DBG_STUB_INVOKE__
        removeControlsCalled = true;
    });

    stub.set_lamda(ADDR(DetailView, setUrl), [&detailViewSetUrlCalled](DetailView *, const QUrl &, int) {
        __DBG_STUB_INVOKE__
        detailViewSetUrlCalled = true;
    });

    widget = new DetailSpaceWidget();
    widget->setCurrentUrl(testUrl, 0);

    // Should not call view update methods for invisible widget
    EXPECT_FALSE(removeControlsCalled);
    EXPECT_FALSE(detailViewSetUrlCalled);
}

/**
 * @brief 测试获取当前URL
 * 验证URL获取功能
 */
TEST_F(DetailSpaceWidgetTest, CurrentUrl_AfterSetting_ReturnsCorrectUrl)
{
    // Mock widget as invisible to avoid view updates
    stub.set_lamda(ADDR(DetailSpaceWidget, isVisible), [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    widget = new DetailSpaceWidget();
    widget->setCurrentUrl(testUrl, 0);

    QUrl result = widget->currentUrl();
    EXPECT_EQ(result, testUrl);
}

/**
 * @brief 测试插入扩展控件
 * 验证扩展控件插入功能
 */
TEST_F(DetailSpaceWidgetTest, InsterExpandControl_ValidIndexAndWidget_CallsDetailView)
{
    bool insertCustomControlCalled = false;
    int receivedIndex = -1;
    QWidget *receivedWidget = nullptr;

    // Mock DetailView insertCustomControl
    stub.set_lamda(ADDR(DetailView, insertCustomControl), [&insertCustomControlCalled, &receivedIndex, &receivedWidget](DetailView *, int index, QWidget *widget) {
        __DBG_STUB_INVOKE__
        insertCustomControlCalled = true;
        receivedIndex = index;
        receivedWidget = widget;
        return true;
    });

    widget = new DetailSpaceWidget();
    bool result = widget->insterExpandControl(5, mockWidget);

    EXPECT_TRUE(result);
    EXPECT_TRUE(insertCustomControlCalled);
    EXPECT_EQ(receivedIndex, 5);
    EXPECT_EQ(receivedWidget, mockWidget);
}

/**
 * @brief 测试插入扩展控件 - 失败情况
 * 验证插入失败时的处理
 */
TEST_F(DetailSpaceWidgetTest, InsterExpandControl_InsertionFails_ReturnsFalse)
{
    // Mock DetailView insertCustomControl to return false
    stub.set_lamda(ADDR(DetailView, insertCustomControl), [](DetailView *, int, QWidget *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    widget = new DetailSpaceWidget();
    bool result = widget->insterExpandControl(5, mockWidget);

    EXPECT_FALSE(result);
}

/**
 * @brief 测试移除控件
 * 验证控件移除功能
 */
TEST_F(DetailSpaceWidgetTest, RemoveControls_Called_CallsDetailViewRemoveControl)
{
    bool removeControlCalled = false;

    // Mock DetailView removeControl
    stub.set_lamda(ADDR(DetailView, removeControl), [&removeControlCalled](DetailView *) {
        __DBG_STUB_INVOKE__
        removeControlCalled = true;
    });

    widget = new DetailSpaceWidget();
    widget->removeControls();

    EXPECT_TRUE(removeControlCalled);
}

/**
 * @brief 测试UI初始化
 * 验证UI组件的正确初始化
 */
TEST_F(DetailSpaceWidgetTest, InitializeUi_Called_SetsUpUICorrectly)
{
    bool setAutoFillBackgroundCalled = false;
    bool setBackgroundRoleCalled = false;

    // Mock UI setup methods
    stub.set_lamda(&DetailSpaceWidget::setAutoFillBackground, [&setAutoFillBackgroundCalled] {
        __DBG_STUB_INVOKE__
        setAutoFillBackgroundCalled = true;
    });

    stub.set_lamda(&DetailSpaceWidget::setBackgroundRole, [&setBackgroundRoleCalled] {
        __DBG_STUB_INVOKE__
        setBackgroundRoleCalled = true;
    });

    widget = new DetailSpaceWidget();

    EXPECT_TRUE(setAutoFillBackgroundCalled);
    EXPECT_TRUE(setBackgroundRoleCalled);
}

/**
 * @brief 测试零窗口ID的查找
 * 验证零窗口ID的处理
 */
TEST_F(DetailSpaceWidgetTest, SetCurrentUrl_ZeroWindowId_UsesDirectUrl)
{
    bool extendedVersionCalled = false;

    // Mock DetailSpaceHelper to return 0 (no window found)
    stub.set_lamda(&DetailSpaceHelper::findWindowIdByDetailSpace, [](DetailSpaceWidget *) {
        __DBG_STUB_INVOKE__
        return static_cast<quint64>(0);
    });

    // Mock the extended setCurrentUrl version
    using SetCurrentUrlFunc = void (DetailSpaceWidget::*)(const QUrl &, int);
    stub.set_lamda(static_cast<SetCurrentUrlFunc>(&DetailSpaceWidget::setCurrentUrl),
                   [&extendedVersionCalled] {
        __DBG_STUB_INVOKE__
        extendedVersionCalled = true;
    });

    widget = new DetailSpaceWidget();
    widget->setCurrentUrl(testUrl);

    EXPECT_TRUE(extendedVersionCalled);
}
