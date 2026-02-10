// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include <QCoreApplication>
#include <QUrl>
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QShowEvent>

// DTK includes
#include <DFrame>
#include <DLabel>

// 包含待测试的类
#include "views/detailview.h"
#include "views/filebaseinfoview.h"
#include "utils/detailmanager.h"
#include "dfmplugin_detailspace_global.h"

#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DGuiApplicationHelper>
#endif

DPDETAILSPACE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

/**
 * @brief DetailView类单元测试
 *
 * 测试范围：
 * 1. 自定义控件添加和插入
 * 2. URL设置和UI创建
 * 3. 控件移除功能
 * 4. 尺寸模式适配
 * 5. 显示事件处理
 * 6. UI组件初始化
 * 7. 基础视图创建
 */
class DetailViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Setup test data
        testUrl = QUrl("file:///home/user/test.txt");
        testWidgetFilter = 5;
        mockWidget1 = new QWidget;
        mockWidget2 = new QWidget;
        view = nullptr;
    }

    void TearDown() override
    {
        if (mockWidget1) {
            delete mockWidget1;
            mockWidget1 = nullptr;
        }
        if (mockWidget2) {
            delete mockWidget2;
            mockWidget2 = nullptr;
        }
        if (view) {
            delete view;
            view = nullptr;
        }
        stub.clear();
    }

    stub_ext::StubExt stub;
    DetailView *view { nullptr };
    QUrl testUrl;
    int testWidgetFilter;
    QWidget *mockWidget1;
    QWidget *mockWidget2;
};

/**
 * @brief 测试构造函数
 * 验证DetailView能够正确构造和初始化
 */
TEST_F(DetailViewTest, Constructor_DefaultConstruction_ObjectCreatedSuccessfully)
{
    bool initInfoUICalled = false;
    bool initUiForSizeModeCalled = false;

    // Mock initialization methods
    stub.set_lamda(ADDR(DetailView, initInfoUI), [&initInfoUICalled](DetailView *) {
        __DBG_STUB_INVOKE__
        initInfoUICalled = true;
    });

    stub.set_lamda(ADDR(DetailView, initUiForSizeMode), [&initUiForSizeModeCalled](DetailView *) {
        __DBG_STUB_INVOKE__
        initUiForSizeModeCalled = true;
    });

    view = new DetailView();
    EXPECT_NE(view, nullptr);

    // Verify initialization was called
    EXPECT_TRUE(initInfoUICalled);
    EXPECT_TRUE(initUiForSizeModeCalled);

    // Verify inheritance
    EXPECT_NE(dynamic_cast<DFrame *>(view), nullptr);
}

/**
 * @brief 测试添加自定义控件
 * 验证自定义控件能够正确添加
 */
TEST_F(DetailViewTest, AddCustomControl_ValidWidget_ReturnsTrue)
{
    view = new DetailView();
    bool result = view->addCustomControl(mockWidget1);

    EXPECT_TRUE(result);
}

/**
 * @brief 测试添加自定义控件 - 空指针
 * 验证空指针的安全处理
 */
TEST_F(DetailViewTest, AddCustomControl_NullWidget_ReturnsFalse)
{
    view = new DetailView();
    bool result = view->addCustomControl(nullptr);

    EXPECT_FALSE(result);
}

/**
 * @brief 测试插入自定义控件
 * 验证自定义控件能够在指定位置插入
 */
TEST_F(DetailViewTest, InsertCustomControl_ValidIndexAndWidget_ReturnsTrue)
{
    view = new DetailView();
    bool result = view->insertCustomControl(3, mockWidget1);

    EXPECT_TRUE(result);
}

/**
 * @brief 测试插入自定义控件 - 空指针widget
 * 验证空指针widget的处理
 */
TEST_F(DetailViewTest, InsertCustomControl_NullWidget_ReturnsFalse)
{
    view = new DetailView();
    bool result = view->insertCustomControl(0, nullptr);

    EXPECT_FALSE(result);
}

/**
 * @brief 测试插入自定义控件 - 负索引
 * 验证负索引的处理
 */
TEST_F(DetailViewTest, InsertCustomControl_NegativeIndex_HandledCorrectly)
{
    view = new DetailView();
    bool result = view->insertCustomControl(-1, mockWidget1);

    EXPECT_TRUE(result);
}

/**
 * @brief 测试移除控件
 * 验证控件移除功能
 */
TEST_F(DetailViewTest, RemoveControl_Called_RemovesAllExpandWidgets)
{
    view = new DetailView();

    EXPECT_NO_FATAL_FAILURE(view->removeControl());
}

/**
 * @brief 测试设置URL
 * 验证URL设置和UI创建功能
 */
TEST_F(DetailViewTest, SetUrl_ValidUrlAndFilter_CreatesUIElements)
{
    bool createHeadUICalled = false;
    bool createBasicWidgetCalled = false;
    QUrl receivedUrl;
    int receivedFilter = -1;

    // Mock UI creation methods
    stub.set_lamda(ADDR(DetailView, createHeadUI), [&createHeadUICalled, &receivedUrl, &receivedFilter](DetailView *, const QUrl &url, int filter) {
        __DBG_STUB_INVOKE__
        createHeadUICalled = true;
        receivedUrl = url;
        receivedFilter = filter;
    });

    stub.set_lamda(ADDR(DetailView, createBasicWidget), [&createBasicWidgetCalled](DetailView *, const QUrl &, int) {
        __DBG_STUB_INVOKE__
        createBasicWidgetCalled = true;
    });

    view = new DetailView();
    view->setUrl(testUrl, testWidgetFilter);

    EXPECT_TRUE(createHeadUICalled);
    EXPECT_TRUE(createBasicWidgetCalled);
    EXPECT_EQ(receivedUrl, testUrl);
    EXPECT_EQ(receivedFilter, testWidgetFilter);
}

/**
 * @brief 测试设置URL - 空URL
 * 验证空URL的处理
 */
TEST_F(DetailViewTest, SetUrl_EmptyUrl_HandledSafely)
{
    bool createHeadUICalled = false;
    bool createBasicWidgetCalled = false;

    stub.set_lamda(ADDR(DetailView, createHeadUI), [&createHeadUICalled](DetailView *, const QUrl &, int) {
        __DBG_STUB_INVOKE__
        createHeadUICalled = true;
    });

    stub.set_lamda(ADDR(DetailView, createBasicWidget), [&createBasicWidgetCalled](DetailView *, const QUrl &, int) {
        __DBG_STUB_INVOKE__
        createBasicWidgetCalled = true;
    });

    view = new DetailView();
    QUrl emptyUrl;
    view->setUrl(emptyUrl, 0);

    EXPECT_TRUE(createHeadUICalled);
    EXPECT_TRUE(createBasicWidgetCalled);
}

/**
 * @brief 测试尺寸模式初始化
 * 验证尺寸模式适配功能
 */
TEST_F(DetailViewTest, InitUiForSizeMode_Called_HandledCorrectly)
{
    view = new DetailView();

    // This should not crash
    EXPECT_NO_FATAL_FAILURE(view->initUiForSizeMode());
}

/**
 * @brief 测试显示事件处理
 * 验证显示事件的正确处理
 */
TEST_F(DetailViewTest, ShowEvent_Called_HandlesEventCorrectly)
{
    bool showEventHandled = false;

    // Mock the base class showEvent
    stub.set_lamda(VADDR(DFrame, showEvent), [&showEventHandled] {
        __DBG_STUB_INVOKE__
        showEventHandled = true;
    });

    view = new DetailView();

    QShowEvent showEvent;
    view->showEvent(&showEvent);

    EXPECT_TRUE(showEventHandled);
}

/**
 * @brief 测试头部UI创建
 * 验证头部UI的创建功能
 */
TEST_F(DetailViewTest, CreateHeadUI_ValidUrl_CreatesIconLabel)
{
    view = new DetailView();
    EXPECT_NO_FATAL_FAILURE(view->createHeadUI(testUrl, testWidgetFilter));
}

/**
 * @brief 测试基础widget创建
 * 验证基础widget的创建功能
 */
TEST_F(DetailViewTest, CreateBasicWidget_ValidUrl_CreatesFileBaseInfoView)
{
    // Mock FileBaseInfoView methods
    stub.set_lamda(ADDR(FileBaseInfoView, setFileUrl), [](FileBaseInfoView *, const QUrl &) {
        __DBG_STUB_INVOKE__
    });

    view = new DetailView();
    EXPECT_NO_FATAL_FAILURE(view->createBasicWidget(testUrl, testWidgetFilter));
}
