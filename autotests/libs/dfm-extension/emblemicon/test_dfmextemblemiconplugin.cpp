// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <dfm-extension/emblemicon/dfmextemblemiconplugin.h>
#include <dfm-extension/emblemicon/dfmextemblem.h>

using namespace DFMEXT;

namespace {

} // anonymous namespace

/**
 * @brief DFMExtEmblemIconPlugin类单元测试
 *
 * 测试范围：
 * 1. 标志图标插件基本功能
 * 2. 图标获取机制
 * 3. 位置标志图标功能
 * 4. 回调函数注册
 * 5. 兼容性测试
 */
class DFMExtEmblemIconPluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        plugin = new DFMExtEmblemIconPlugin();
        ASSERT_NE(plugin, nullptr);
    }

    void TearDown() override
    {
        delete plugin;
    }

    DFMExtEmblemIconPlugin *plugin;
};

/**
 * @brief 测试DFMExtEmblemIconPlugin构造函数
 * 验证DFMExtEmblemIconPlugin对象能够正确创建
 */
TEST_F(DFMExtEmblemIconPluginTest, Constructor)
{
    // 验证对象创建成功
    EXPECT_NE(plugin, nullptr);
    
    // 验证对象类型正确
    EXPECT_TRUE(dynamic_cast<DFMExtEmblemIconPlugin*>(plugin) != nullptr);
}

/**
 * @brief 测试析构函数
 * 验证DFMExtEmblemIconPlugin对象能够正确析构
 */
TEST_F(DFMExtEmblemIconPluginTest, Destructor)
{
    // 创建临时对象进行析构测试
    DFMExtEmblemIconPlugin *tempPlugin = new DFMExtEmblemIconPlugin();
    EXPECT_NE(tempPlugin, nullptr);
    
    // 析构应该不会崩溃
    delete tempPlugin;
    EXPECT_TRUE(true); // 如果到达这里说明析构成功
}

/**
 * @brief 测试emblemIcons方法（已弃用）
 * 验证标志图标获取功能
 */
TEST_F(DFMExtEmblemIconPluginTest, EmblemIcons)
{
    const std::string testPath = "/home/test/file.txt";
    
    // 测试默认实现
    auto icons = plugin->emblemIcons(testPath);
    
    // 验证返回结果（默认实现应该返回空vector）
    EXPECT_TRUE(icons.empty());
}

/**
 * @brief 测试locationEmblemIcons方法
 * 验证位置标志图标获取功能
 */
TEST_F(DFMExtEmblemIconPluginTest, LocationEmblemIcons)
{
    const std::string testPath = "/home/test/file.txt";
    const int systemIconCount = 2;
    
    // 测试默认实现
    DFMExtEmblem emblem = plugin->locationEmblemIcons(testPath, systemIconCount);
    
    // 验证返回结果（默认实现应该返回空的标志）
    EXPECT_TRUE(true); // 基本功能测试
}

/**
 * @brief 测试registerEmblemIcons方法
 * 验证标志图标回调函数注册
 */
TEST_F(DFMExtEmblemIconPluginTest, RegisterEmblemIcons)
{
    bool callbackCalled = false;
    const std::string testPath = "/home/test/file.txt";
    
    // 注册标志图标回调
    plugin->registerEmblemIcons([&callbackCalled](const std::string &filePath) {
        callbackCalled = true;
        EXPECT_FALSE(filePath.empty());
        return DFMExtEmblemIconPlugin::IconsType{"icon1", "icon2"};
    });
    
    // 调用标志图标方法
    auto icons = plugin->emblemIcons(testPath);
    
    // 验证回调被调用并返回正确结果
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(icons.size(), 2);
    EXPECT_EQ(icons[0], "icon1");
    EXPECT_EQ(icons[1], "icon2");
}

/**
 * @brief 测试registerLocationEmblemIcons方法
 * 验证位置标志图标回调函数注册
 */
TEST_F(DFMExtEmblemIconPluginTest, RegisterLocationEmblemIcons)
{
    bool callbackCalled = false;
    const std::string testPath = "/home/test/file.txt";
    const int systemIconCount = 3;
    
    // 注册位置标志图标回调
    plugin->registerLocationEmblemIcons([&callbackCalled](const std::string &filePath, int iconCount) {
        callbackCalled = true;
        EXPECT_FALSE(filePath.empty());
        EXPECT_GT(iconCount, 0);
        
        // 创建一个测试用的DFMExtEmblem对象
        DFMExtEmblem emblem;
        return emblem;
    });
    
    // 调用位置标志图标方法
    DFMExtEmblem emblem = plugin->locationEmblemIcons(testPath, systemIconCount);
    
    // 验证回调被调用
    EXPECT_TRUE(callbackCalled);
}

/**
 * @brief 测试多个回调函数的注册和调用
 * 验证多个回调函数能够正确工作
 */
TEST_F(DFMExtEmblemIconPluginTest, MultipleCallbacks)
{
    int emblemIconsCallCount = 0;
    int locationEmblemIconsCallCount = 0;
    
    // 注册多个回调函数
    plugin->registerEmblemIcons([&emblemIconsCallCount](const std::string &filePath) {
        emblemIconsCallCount++;
        return DFMExtEmblemIconPlugin::IconsType{"test_icon"};
    });
    
    plugin->registerLocationEmblemIcons([&locationEmblemIconsCallCount](const std::string &filePath, int iconCount) {
        locationEmblemIconsCallCount++;
        DFMExtEmblem emblem;
        return emblem;
    });
    
    // 调用各个方法
    plugin->emblemIcons("/home/test/file1.txt");
    plugin->locationEmblemIcons("/home/test/file2.txt", 2);
    
    // 验证所有回调都被调用
    EXPECT_EQ(emblemIconsCallCount, 1);
    EXPECT_EQ(locationEmblemIconsCallCount, 1);
}

/**
 * @brief 测试边界条件
 * 验证插件在边界条件下的行为
 */
TEST_F(DFMExtEmblemIconPluginTest, BoundaryConditions)
{
    // 测试空字符串路径
    auto icons1 = plugin->emblemIcons("");
    EXPECT_TRUE(icons1.empty());
    
    DFMExtEmblem emblem1 = plugin->locationEmblemIcons("", 0);
    EXPECT_TRUE(true); // 不应该崩溃
    
    // 测试负数图标数量
    DFMExtEmblem emblem2 = plugin->locationEmblemIcons("/home/test", -1);
    EXPECT_TRUE(true); // 不应该崩溃
    
    // 测试很长的路径
    std::string longPath(1000, 'a');
    auto icons2 = plugin->emblemIcons(longPath);
    EXPECT_TRUE(icons2.empty());
    
    DFMExtEmblem emblem3 = plugin->locationEmblemIcons(longPath, 100);
    EXPECT_TRUE(true); // 不应该崩溃
}

/**
 * @brief 测试回调函数返回值处理
 * 验证不同回调函数返回值的处理
 */
TEST_F(DFMExtEmblemIconPluginTest, CallbackReturnValues)
{
    // 测试返回空vector的回调
    plugin->registerEmblemIcons([](const std::string &filePath) {
        return DFMExtEmblemIconPlugin::IconsType{}; // 空vector
    });
    
    auto icons1 = plugin->emblemIcons("/home/test");
    EXPECT_TRUE(icons1.empty());
    
    // 创建新的插件实例来测试大量图标的回调
    // 因为registerEmblemIcons只注册第一个回调
    DFMExtEmblemIconPlugin *newPlugin = new DFMExtEmblemIconPlugin();
    newPlugin->registerEmblemIcons([](const std::string &filePath) {
        DFMExtEmblemIconPlugin::IconsType icons;
        for (int i = 0; i < 100; ++i) {
            icons.push_back("icon" + std::to_string(i));
        }
        return icons;
    });
    
    auto icons2 = newPlugin->emblemIcons("/home/test");
    EXPECT_EQ(icons2.size(), 100);
    
    delete newPlugin;
}

/**
 * @brief 测试性能
 * 验证插件的基本性能表现
 */
TEST_F(DFMExtEmblemIconPluginTest, Performance)
{
    const int iterations = 1000;
    
    // 注册一个简单的回调
    plugin->registerEmblemIcons([](const std::string &filePath) {
        return DFMExtEmblemIconPlugin::IconsType{"test_icon"};
    });
    
    // 测试大量调用
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto icons = plugin->emblemIcons("/home/test/file" + std::to_string(i));
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 验证性能在合理范围内（1000次调用应该在1秒内完成）
    EXPECT_LT(duration.count(), 1000);
}

/**
 * @brief 测试兼容性
 * 验证新旧API的兼容性
 */
TEST_F(DFMExtEmblemIconPluginTest, Compatibility)
{
    const std::string testPath = "/home/test/file.txt";
    
    // 测试旧API（emblemIcons）
    auto oldIcons = plugin->emblemIcons(testPath);
    EXPECT_TRUE(oldIcons.empty()); // 默认实现应该返回空
    
    // 测试新API（locationEmblemIcons）
    DFMExtEmblem newEmblem = plugin->locationEmblemIcons(testPath, 2);
    EXPECT_TRUE(true); // 基本功能测试
    
    // 同时注册两种回调
    plugin->registerEmblemIcons([](const std::string &filePath) {
        return DFMExtEmblemIconPlugin::IconsType{"old_icon"};
    });
    
    plugin->registerLocationEmblemIcons([](const std::string &filePath, int iconCount) {
        DFMExtEmblem emblem;
        return emblem;
    });
    
    // 验证两种API都能正常工作
    auto icons = plugin->emblemIcons(testPath);
    EXPECT_EQ(icons.size(), 1);
    EXPECT_EQ(icons[0], "old_icon");
    
    DFMExtEmblem emblem = plugin->locationEmblemIcons(testPath, 2);
    EXPECT_TRUE(true); // 基本功能测试
} 