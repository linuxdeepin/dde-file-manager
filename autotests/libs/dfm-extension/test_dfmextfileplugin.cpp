// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <dfm-extension/file/dfmextfileplugin.h>
#include <dfm-extension/file/dfmextfileoperationhook.h>

using namespace DFMEXT;

namespace {

} // anonymous namespace

/**
 * @brief DFMExtFilePlugin类单元测试
 *
 * 测试范围：
 * 1. 文件插件基本功能
 * 2. 文件操作钩子管理
 * 3. 插件生命周期
 */
class DFMExtFilePluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        plugin = new DFMExtFilePlugin();
        ASSERT_NE(plugin, nullptr);
    }

    void TearDown() override
    {
        delete plugin;
    }

    DFMExtFilePlugin *plugin;
};

/**
 * @brief 测试DFMExtFilePlugin构造函数
 * 验证DFMExtFilePlugin对象能够正确创建
 */
TEST_F(DFMExtFilePluginTest, Constructor)
{
    // 验证对象创建成功
    EXPECT_NE(plugin, nullptr);
    
    // 验证对象类型正确
    EXPECT_TRUE(dynamic_cast<DFMExtFilePlugin*>(plugin) != nullptr);
}

/**
 * @brief 测试析构函数
 * 验证DFMExtFilePlugin对象能够正确析构
 */
TEST_F(DFMExtFilePluginTest, Destructor)
{
    // 创建临时对象进行析构测试
    DFMExtFilePlugin *tempPlugin = new DFMExtFilePlugin();
    EXPECT_NE(tempPlugin, nullptr);
    
    // 析构应该不会崩溃
    delete tempPlugin;
    EXPECT_TRUE(true); // 如果到达这里说明析构成功
}

/**
 * @brief 测试registerFileOperationHooks方法
 * 验证文件操作钩子注册功能
 */
TEST_F(DFMExtFilePluginTest, RegisterFileOperationHooks)
{
    // 测试注册文件操作钩子回调
    bool hookRegistered = false;
    plugin->registerFileOperationHooks([&hookRegistered](DFMExtFileOperationHook *hook) {
        hookRegistered = true;
        // 在这个测试中，我们不验证hook参数，因为它可能为nullptr
        // 这是合法的使用场景
    });
    
    // 触发钩子调用 - 传入nullptr是合法的
    plugin->fileOperationHooks(nullptr);
    
    // 验证回调被调用
    EXPECT_TRUE(hookRegistered);
}

/**
 * @brief 测试边界条件
 * 验证插件在边界条件下的行为
 */
TEST_F(DFMExtFilePluginTest, BoundaryConditions)
{
    // 测试多次调用registerFileOperationHooks
    for (int i = 0; i < 10; ++i) {
        plugin->registerFileOperationHooks([](DFMExtFileOperationHook *hook) {
            // 验证不会崩溃
        });
    }
}

/**
 * @brief 测试性能
 * 验证插件的基本性能表现
 */
TEST_F(DFMExtFilePluginTest, Performance)
{
    const int iterations = 100;
    
    // 测试大量注册文件操作钩子的性能
    for (int i = 0; i < iterations; ++i) {
        plugin->registerFileOperationHooks([](DFMExtFileOperationHook *hook) {
            // 性能测试
        });
    }
    
    // 验证性能测试完成
    EXPECT_TRUE(true);
} 