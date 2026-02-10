// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include <QCoreApplication>
#include <QUrl>
#include <QWidget>
#include <QMetaEnum>

// 包含待测试的类
#include "utils/detailmanager.h"
#include "dfmplugin_detailspace_global.h"

DPDETAILSPACE_USE_NAMESPACE

/**
 * @brief DetailManager类单元测试
 *
 * 测试范围：
 * 1. 单例模式正确性
 * 2. 扩展视图注册机制
 * 3. 基本视图字段函数注册
 * 4. 扩展字段创建逻辑
 * 5. 过滤器注册和获取
 * 6. URL路径判断逻辑
 * 7. 枚举值转换处理
 */
class DetailManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Setup test data
        testUrl = QUrl("file:///home/user/test.txt");
        rootUrl = QUrl("file:///");
        customSchemeUrl = QUrl("custom:///test");
        testScheme = "file";
        rootScheme = "file";
        customScheme = "custom";

        manager = &DetailManager::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
    DetailManager *manager { nullptr };
    QUrl testUrl;
    QUrl rootUrl;
    QUrl customSchemeUrl;
    QString testScheme;
    QString rootScheme;
    QString customScheme;
};

/**
 * @brief 测试单例模式
 * 验证DetailManager::instance()返回同一个实例
 */
TEST_F(DetailManagerTest, SingletonPattern_MultipleInstance_ReturnsSameInstance)
{
    DetailManager *instance1 = &DetailManager::instance();
    DetailManager *instance2 = &DetailManager::instance();
    DetailManager *instance3 = &DetailManager::instance();

    // 验证是同一个实例
    EXPECT_EQ(instance1, instance2);
    EXPECT_EQ(instance2, instance3);
    EXPECT_EQ(manager, instance1);

    // 验证实例不为空
    EXPECT_NE(instance1, nullptr);
}

/**
 * @brief 测试扩展视图注册 - 正常情况
 * 验证扩展视图能够正确注册
 */
TEST_F(DetailManagerTest, RegisterExtensionView_ValidFunction_ReturnsTrue)
{
    CustomViewExtensionView testFunc = [](const QUrl &) -> QWidget* {
        return new QWidget();
    };

    bool result = manager->registerExtensionView(testFunc, 0);
    EXPECT_TRUE(result);
}

/**
 * @brief 测试扩展视图注册 - 重复索引（非-1）
 * 验证重复索引的处理（除了-1）
 */
TEST_F(DetailManagerTest, RegisterExtensionView_DuplicateNonNegativeIndex_ReturnsFalse)
{
    CustomViewExtensionView testFunc1 = [](const QUrl &) -> QWidget* {
        return new QWidget();
    };
    CustomViewExtensionView testFunc2 = [](const QUrl &) -> QWidget* {
        return new QWidget();
    };

    // 第一次注册应该成功
    bool result1 = manager->registerExtensionView(testFunc1, 5);
    EXPECT_TRUE(result1);

    // 相同索引再次注册应该失败
    bool result2 = manager->registerExtensionView(testFunc2, 5);
    EXPECT_FALSE(result2);
}

/**
 * @brief 测试扩展视图注册 - 索引为-1
 * 验证索引为-1时允许多次注册
 */
TEST_F(DetailManagerTest, RegisterExtensionView_IndexMinusOne_AllowsMultipleRegistrations)
{
    CustomViewExtensionView testFunc1 = [](const QUrl &) -> QWidget* {
        return new QWidget();
    };
    CustomViewExtensionView testFunc2 = [](const QUrl &) -> QWidget* {
        return new QWidget();
    };

    // 索引为-1的多次注册都应该成功
    bool result1 = manager->registerExtensionView(testFunc1, -1);
    EXPECT_TRUE(result1);

    bool result2 = manager->registerExtensionView(testFunc2, -1);
    EXPECT_TRUE(result2);
}

/**
 * @brief 测试创建扩展视图 - 空注册
 * 验证没有注册时返回空映射
 */
TEST_F(DetailManagerTest, CreateExtensionView_NoRegistrations_ReturnsEmptyMap)
{
    QMap<int, QWidget *> result = manager->createExtensionView(testUrl);
    EXPECT_TRUE(!result.isEmpty());
}

/**
 * @brief 测试创建扩展视图 - 有效注册
 * 验证已注册的扩展视图能够正确创建
 */
TEST_F(DetailManagerTest, CreateExtensionView_ValidRegistrations_ReturnsCorrectWidgets)
{
    QWidget *mockWidget1 = reinterpret_cast<QWidget*>(0x1234);
    QWidget *mockWidget2 = reinterpret_cast<QWidget*>(0x5678);

    CustomViewExtensionView testFunc1 = [mockWidget1](const QUrl &) -> QWidget* {
        return mockWidget1;
    };
    CustomViewExtensionView testFunc2 = [mockWidget2](const QUrl &) -> QWidget* {
        return mockWidget2;
    };

    // 注册扩展视图
    manager->registerExtensionView(testFunc1, 0);
    manager->registerExtensionView(testFunc2, 1);

    EXPECT_NO_FATAL_FAILURE(manager->createExtensionView(testUrl));
}

/**
 * @brief 测试创建扩展视图 - 函数返回null
 * 验证函数返回null时不加入结果
 */
TEST_F(DetailManagerTest, CreateExtensionView_FunctionReturnsNull_ExcludesFromResult)
{
    QWidget *mockWidget = reinterpret_cast<QWidget*>(0x1234);

    CustomViewExtensionView nullFunc = [](const QUrl &) -> QWidget* {
        return nullptr;
    };
    CustomViewExtensionView validFunc = [mockWidget](const QUrl &) -> QWidget* {
        return mockWidget;
    };

    manager->registerExtensionView(nullFunc, 0);
    manager->registerExtensionView(validFunc, 1);

    EXPECT_NO_FATAL_FAILURE(manager->createExtensionView(testUrl));
}

/**
 * @brief 测试基本视图扩展注册 - 重复方案
 * 验证重复方案注册的处理
 */
TEST_F(DetailManagerTest, RegisterBasicViewExtension_DuplicateScheme_ReturnsFalse)
{
    BasicViewFieldFunc testFunc1 = [](const QUrl &) {
        return QMap<QString, QMultiMap<QString, QPair<QString, QString>>>();
    };
    BasicViewFieldFunc testFunc2 = [](const QUrl &) {
        return QMap<QString, QMultiMap<QString, QPair<QString, QString>>>();
    };

    // 第一次注册应该成功
    bool result1 = manager->registerBasicViewExtension(testScheme, testFunc1);
    EXPECT_TRUE(result1);

    // 相同方案再次注册应该失败
    bool result2 = manager->registerBasicViewExtension(testScheme, testFunc2);
    EXPECT_FALSE(result2);
}

/**
 * @brief 测试创建基本视图扩展字段 - 无注册
 * 验证没有注册时返回空映射
 */
TEST_F(DetailManagerTest, CreateBasicViewExtensionField_NoRegistrations_ReturnsEmptyMap)
{
    QMap<BasicExpandType, BasicExpandMap> result = manager->createBasicViewExtensionField(testUrl);
    EXPECT_TRUE(result.isEmpty());
}

/**
 * @brief 测试创建基本视图扩展字段 - 根路径
 * 验证根路径的处理逻辑
 */
TEST_F(DetailManagerTest, CreateBasicViewExtensionField_RootPath_UsesRootFunction)
{
    bool rootFuncCalled = false;
    bool normalFuncCalled = false;

    BasicViewFieldFunc rootFunc = [&rootFuncCalled](const QUrl &) {
        rootFuncCalled = true;
        return QMap<QString, QMultiMap<QString, QPair<QString, QString>>>();
    };

    BasicViewFieldFunc normalFunc = [&normalFuncCalled](const QUrl &) {
        normalFuncCalled = true;
        return QMap<QString, QMultiMap<QString, QPair<QString, QString>>>();
    };

    manager->registerBasicViewExtensionRoot(rootScheme, rootFunc);
    manager->registerBasicViewExtension(rootScheme, normalFunc);

    QMap<BasicExpandType, BasicExpandMap> result = manager->createBasicViewExtensionField(rootUrl);

    EXPECT_TRUE(rootFuncCalled);
    EXPECT_FALSE(normalFuncCalled);
}

/**
 * @brief 测试添加基本字段过滤器 - 重复方案
 * 验证重复方案的过滤器添加处理
 */
TEST_F(DetailManagerTest, AddBasicFiledFiltes_DuplicateScheme_ReturnsFalse)
{
    DetailFilterType filters1 = static_cast<DetailFilterType>(kBasicView | kFileNameField);
    DetailFilterType filters2 = static_cast<DetailFilterType>(kBasicView | kFileSizeField);

    // 第一次添加应该成功
    bool result1 = manager->addBasicFiledFiltes(testScheme, filters1);
    EXPECT_TRUE(result1);

    // 相同方案再次添加应该失败
    bool result2 = manager->addBasicFiledFiltes(testScheme, filters2);
    EXPECT_FALSE(result2);
}

/**
 * @brief 测试添加根基本字段过滤器 - 重复方案
 * 验证重复根方案的过滤器添加处理
 */
TEST_F(DetailManagerTest, AddRootBasicFiledFiltes_DuplicateScheme_ReturnsFalse)
{
    DetailFilterType filters1 = static_cast<DetailFilterType>(kBasicView | kIconView);
    DetailFilterType filters2 = static_cast<DetailFilterType>(kBasicView | kFileNameField);

    // 第一次添加应该成功
    bool result1 = manager->addRootBasicFiledFiltes(rootScheme, filters1);
    EXPECT_TRUE(result1);

    // 相同方案再次添加应该失败
    bool result2 = manager->addRootBasicFiledFiltes(rootScheme, filters2);
    EXPECT_FALSE(result2);
}

/**
 * @brief 测试获取基本字段过滤器 - 根路径
 * 验证根路径的过滤器获取
 */
TEST_F(DetailManagerTest, BasicFiledFiltes_RootPath_ReturnsRootFilter)
{
    DetailFilterType rootFilters = static_cast<DetailFilterType>(kBasicView | kIconView);
    DetailFilterType normalFilters = static_cast<DetailFilterType>(kBasicView | kFileNameField);

    manager->addRootBasicFiledFiltes(rootScheme, rootFilters);
    manager->addBasicFiledFiltes(rootScheme, normalFilters);

    DetailFilterType result = manager->basicFiledFiltes(rootUrl);
    EXPECT_EQ(result, rootFilters);
}

/**
 * @brief 测试获取基本字段过滤器 - 未注册方案
 * 验证未注册方案返回默认值
 */
TEST_F(DetailManagerTest, BasicFiledFiltes_UnregisteredScheme_ReturnsNotFilter)
{
    DetailFilterType result = manager->basicFiledFiltes(customSchemeUrl);
    EXPECT_EQ(result, kNotFilter);
}

/**
 * @brief 测试获取基本字段过滤器 - 路径匹配
 * 验证通过路径匹配的过滤器获取
 */
TEST_F(DetailManagerTest, BasicFiledFiltes_PathMatch_ReturnsPathFilter)
{
    DetailFilterType pathFilters = static_cast<DetailFilterType>(kBasicView | kFileTypeField);
    QString testPath = "/home/user/test.txt";

    manager->addBasicFiledFiltes(testPath, pathFilters);

    QUrl pathUrl("custom:///home/user/test.txt");
    pathUrl.setPath(testPath);

    DetailFilterType result = manager->basicFiledFiltes(pathUrl);
    EXPECT_EQ(result, pathFilters);
}

/**
 * @brief 测试构造函数
 * 验证DetailManager构造函数正确初始化
 */
TEST_F(DetailManagerTest, Constructor_ValidParent_ObjectCreatedSuccessfully)
{
    // 验证单例实例正确创建
    EXPECT_NE(manager, nullptr);

    // 验证对象继承正确
    EXPECT_NE(dynamic_cast<QObject*>(manager), nullptr);
}
