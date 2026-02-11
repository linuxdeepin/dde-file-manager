// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <dfm-extension/window/dfmextwindowplugin.h>

using namespace DFMEXT;

namespace {

} // anonymous namespace

/**
 * @brief DFMExtWindowPlugin类单元测试
 *
 * 测试范围：
 * 1. 窗口插件基本功能
 * 2. 窗口创建和管理
 * 3. 插件生命周期
 */
class DFMExtWindowPluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        plugin = new DFMExtWindowPlugin();
        ASSERT_NE(plugin, nullptr);
    }

    void TearDown() override
    {
        delete plugin;
    }

    DFMExtWindowPlugin *plugin;
};

/**
 * @brief 测试DFMExtWindowPlugin构造函数
 * 验证DFMExtWindowPlugin对象能够正确创建
 */
TEST_F(DFMExtWindowPluginTest, Constructor)
{
    // 验证对象创建成功
    EXPECT_NE(plugin, nullptr);
    
    // 验证对象类型正确
    EXPECT_TRUE(dynamic_cast<DFMExtWindowPlugin*>(plugin) != nullptr);
}

/**
 * @brief 测试析构函数
 * 验证DFMExtWindowPlugin对象能够正确析构
 */
TEST_F(DFMExtWindowPluginTest, Destructor)
{
    // 创建临时对象进行析构测试
    DFMExtWindowPlugin *tempPlugin = new DFMExtWindowPlugin();
    EXPECT_NE(tempPlugin, nullptr);
    
    // 析构应该不会崩溃
    delete tempPlugin;
    EXPECT_TRUE(true); // 如果到达这里说明析构成功
}

/**
 * @brief 测试基本功能
 * 验证窗口插件的基本功能
 */
TEST_F(DFMExtWindowPluginTest, BasicFunctionality)
{
    // 测试窗口事件方法不会崩溃
    plugin->windowOpened(12345);
    plugin->windowClosed(12345);
    plugin->firstWindowOpened(67890);
    plugin->lastWindowClosed(67890);
    plugin->windowUrlChanged(12345, "file:///home/test");
    
    // 基本功能测试通过
    EXPECT_TRUE(true);
}

/**
 * @brief 测试窗口打开回调注册
 * 验证windowOpened回调的注册和触发
 */
TEST_F(DFMExtWindowPluginTest, WindowOpenedCallback)
{
    bool callbackCalled = false;
    uint64_t receivedWinId = 0;
    
    // 注册窗口打开回调
    plugin->registerWindowOpened([&callbackCalled, &receivedWinId](uint64_t winId) {
        callbackCalled = true;
        receivedWinId = winId;
    });
    
    // 触发窗口打开事件
    const uint64_t testWinId = 12345;
    plugin->windowOpened(testWinId);
    
    // 验证回调被调用
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedWinId, testWinId);
}

/**
 * @brief 测试窗口关闭回调注册
 * 验证windowClosed回调的注册和触发
 */
TEST_F(DFMExtWindowPluginTest, WindowClosedCallback)
{
    bool callbackCalled = false;
    uint64_t receivedWinId = 0;
    
    // 注册窗口关闭回调
    plugin->registerWindowClosed([&callbackCalled, &receivedWinId](uint64_t winId) {
        callbackCalled = true;
        receivedWinId = winId;
    });
    
    // 触发窗口关闭事件
    const uint64_t testWinId = 67890;
    plugin->windowClosed(testWinId);
    
    // 验证回调被调用
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedWinId, testWinId);
}

/**
 * @brief 测试窗口URL变化回调注册
 * 验证windowUrlChanged回调的注册和触发
 */
TEST_F(DFMExtWindowPluginTest, WindowUrlChangedCallback)
{
    bool callbackCalled = false;
    uint64_t receivedWinId = 0;
    std::string receivedUrl;
    
    // 注册窗口URL变化回调
    plugin->registerWindowUrlChanged([&callbackCalled, &receivedWinId, &receivedUrl](uint64_t winId, const std::string &url) {
        callbackCalled = true;
        receivedWinId = winId;
        receivedUrl = url;
    });
    
    // 触发窗口URL变化事件
    const uint64_t testWinId = 33333;
    const std::string testUrl = "file:///home/user/documents";
    plugin->windowUrlChanged(testWinId, testUrl);
    
    // 验证回调被调用
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedWinId, testWinId);
    EXPECT_EQ(receivedUrl, testUrl);
}

/**
 * @brief 测试边界条件
 * 验证插件在边界条件下的行为
 */
TEST_F(DFMExtWindowPluginTest, BoundaryConditions)
{
    // 测试多次创建和销毁
    for (int i = 0; i < 10; ++i) {
        DFMExtWindowPlugin *tempPlugin = new DFMExtWindowPlugin();
        EXPECT_NE(tempPlugin, nullptr);
        delete tempPlugin;
    }
    
    // 验证原始插件仍然有效
    EXPECT_NE(plugin, nullptr);
}

/**
 * @brief 测试性能
 * 验证插件的基本性能表现
 */
TEST_F(DFMExtWindowPluginTest, Performance)
{
    const int iterations = 100;
    
    // 测试大量创建和销毁的性能
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        DFMExtWindowPlugin *tempPlugin = new DFMExtWindowPlugin();
        delete tempPlugin;
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 验证性能在合理范围内（100次创建销毁应该在100毫秒内完成）
    EXPECT_LT(duration.count(), 100);
} 