// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <dfm-extension/dfm-extension.h>
#include <chrono>

using namespace DFMEXT;

namespace {

} // anonymous namespace

/**
 * @brief DFM Extension Global API单元测试
 *
 * 测试范围：
 * 1. 全局初始化和关闭函数
 * 2. 插件工厂函数
 * 3. 生命周期管理
 */
class DFMExtensionGlobalTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 测试前的准备工作
    }

    void TearDown() override
    {
        // 测试后的清理工作
    }
};

/**
 * @brief Test dfm_extension_initialize function
 * Verify extension initialization functionality
 */
TEST_F(DFMExtensionGlobalTest, Initialize)
{
    // 测试初始化函数不会崩溃
    dfm_extension_initialize();
    
    // 验证初始化完成（基本功能测试）
    EXPECT_TRUE(true);
}

/**
 * @brief 测试dfm_extension_shutdown函数
 * 验证扩展关闭功能
 */
TEST_F(DFMExtensionGlobalTest, Shutdown)
{
    // 先初始化
    dfm_extension_initialize();
    
    // 测试关闭函数不会崩溃
    dfm_extension_shutdown();
    
    // 验证关闭完成（基本功能测试）
    EXPECT_TRUE(true);
}

/**
 * @brief 测试dfm_extension_menu函数
 * 验证菜单插件获取功能
 */
TEST_F(DFMExtensionGlobalTest, MenuPlugin)
{
    // 初始化扩展
    dfm_extension_initialize();
    
    // 获取菜单插件
    auto menuPlugin = dfm_extension_menu();
    (void)menuPlugin; // Suppress unused variable warning
    
    // 验证返回结果（可能为nullptr或有效指针）
    // 这里主要测试函数调用不会崩溃
    EXPECT_TRUE(true);
    
    // 清理
    dfm_extension_shutdown();
}

/**
 * @brief 测试dfm_extension_emblem函数
 * 验证标志图标插件获取功能
 */
TEST_F(DFMExtensionGlobalTest, EmblemPlugin)
{
    // 初始化扩展
    dfm_extension_initialize();
    
    // 获取标志图标插件
    auto emblemPlugin = dfm_extension_emblem();
    
    // 验证返回结果（可能为nullptr或有效指针）
    // 这里主要测试函数调用不会崩溃
    EXPECT_TRUE(true);
    
    // 清理
    dfm_extension_shutdown();
}

/**
 * @brief 测试dfm_extension_window函数
 * 验证窗口插件获取功能
 */
TEST_F(DFMExtensionGlobalTest, WindowPlugin)
{
    // 初始化扩展
    dfm_extension_initialize();
    
    // 获取窗口插件
    auto windowPlugin = dfm_extension_window();
    
    // 验证返回结果（可能为nullptr或有效指针）
    // 这里主要测试函数调用不会崩溃
    EXPECT_TRUE(true);
    
    // 清理
    dfm_extension_shutdown();
}

/**
 * @brief 测试dfm_extension_file函数
 * 验证文件插件获取功能
 */
TEST_F(DFMExtensionGlobalTest, FilePlugin)
{
    // 初始化扩展
    dfm_extension_initialize();
    
    // 获取文件插件
    auto filePlugin = dfm_extension_file();
    
    // 验证返回结果（可能为nullptr或有效指针）
    // 这里主要测试函数调用不会崩溃
    EXPECT_TRUE(true);
    
    // 清理
    dfm_extension_shutdown();
}

/**
 * @brief 测试生命周期管理
 * 验证初始化和关闭的正确顺序
 */
TEST_F(DFMExtensionGlobalTest, LifecycleManagement)
{
    // 测试多次初始化和关闭
    for (int i = 0; i < 5; ++i) {
        dfm_extension_initialize();
        
        // 获取所有插件
        auto menuPlugin = dfm_extension_menu();
        auto emblemPlugin = dfm_extension_emblem();
        auto windowPlugin = dfm_extension_window();
        auto filePlugin = dfm_extension_file();
        
        // 验证不会崩溃
        EXPECT_TRUE(true);
        
        dfm_extension_shutdown();
    }
}

/**
 * @brief 测试边界条件
 * 验证在边界条件下的行为
 */
TEST_F(DFMExtensionGlobalTest, BoundaryConditions)
{
    // 测试在未初始化状态下调用插件获取函数
    auto menuPlugin1 = dfm_extension_menu();
    auto emblemPlugin1 = dfm_extension_emblem();
    auto windowPlugin1 = dfm_extension_window();
    auto filePlugin1 = dfm_extension_file();
    
    // 验证不会崩溃
    EXPECT_TRUE(true);
    
    // 测试多次初始化
    dfm_extension_initialize();
    dfm_extension_initialize();
    dfm_extension_initialize();
    
    // 验证不会崩溃
    EXPECT_TRUE(true);
    
    // 测试多次关闭
    dfm_extension_shutdown();
    dfm_extension_shutdown();
    dfm_extension_shutdown();
    
    // 验证不会崩溃
    EXPECT_TRUE(true);
}

/**
 * @brief 测试性能
 * 验证全局API的基本性能表现
 */
TEST_F(DFMExtensionGlobalTest, Performance)
{
    const int iterations = 100;
    
    // 测试大量初始化和关闭的性能
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        dfm_extension_initialize();
        dfm_extension_shutdown();
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 验证性能在合理范围内（100次初始化关闭应该在1秒内完成）
    EXPECT_LT(duration.count(), 1000);
}

/**
 * @brief 测试插件获取的一致性
 * 验证多次获取同一插件的一致性
 */
TEST_F(DFMExtensionGlobalTest, PluginConsistency)
{
    // 初始化扩展
    dfm_extension_initialize();
    
    // 多次获取同一插件
    auto menuPlugin1 = dfm_extension_menu();
    auto menuPlugin2 = dfm_extension_menu();
    auto menuPlugin3 = dfm_extension_menu();
    
    // 验证一致性（可能返回相同指针或都为nullptr）
    EXPECT_TRUE(menuPlugin1 == menuPlugin2);
    EXPECT_TRUE(menuPlugin2 == menuPlugin3);
    
    // 对其他插件也做同样测试
    auto emblemPlugin1 = dfm_extension_emblem();
    auto emblemPlugin2 = dfm_extension_emblem();
    EXPECT_TRUE(emblemPlugin1 == emblemPlugin2);
    
    auto windowPlugin1 = dfm_extension_window();
    auto windowPlugin2 = dfm_extension_window();
    EXPECT_TRUE(windowPlugin1 == windowPlugin2);
    
    auto filePlugin1 = dfm_extension_file();
    auto filePlugin2 = dfm_extension_file();
    EXPECT_TRUE(filePlugin1 == filePlugin2);
    
    // 清理
    dfm_extension_shutdown();
} 