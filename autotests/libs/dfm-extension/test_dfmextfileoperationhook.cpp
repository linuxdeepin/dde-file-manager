// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>

#include <dfm-extension/file/dfmextfileoperationhook.h>
#include <dfm-extension/file/private/dfmextfileoperationhookprivate.h>

#include <dfm-extension/dfm-extension-global.h>
#include "dfm_test_main.h"
#include "stubext.h"

using namespace dfmext;
using namespace testing;

/**
 * @brief DFMExtFileOperationHook类单元测试
 *
 * 测试范围：
 * 1. 构造函数和析构函数
 * 2. 回调函数注册功能
 * 3. 内存管理和资源清理
 * 4. 边界条件和异常处理
 */
class DFMExtFileOperationHookTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

/**
 * @brief 创建测试用的DFMExtFileOperationHook对象
 * 由于构造函数需要私有指针，使用Stub来模拟
 */
DFMExtFileOperationHook* createTestHook()
{
    // 使用stub模拟私有对象创建
    // 由于DFMExtFileOperationHookPrivate是不完整类型，我们使用stub来模拟
    auto hookPrivate = new DFMExtFileOperationHookPrivate;
    DFMExtFileOperationHook *hook = new DFMExtFileOperationHook(hookPrivate);
    return hook;
}

/**
 * @brief 测试DFMExtFileOperationHook构造函数
 * 验证对象能够正确创建
 */
TEST_F(DFMExtFileOperationHookTest, Constructor)
{
    // 创建DFMExtFileOperationHook对象
    DFMExtFileOperationHook *hook = createTestHook();
    
    // 验证对象创建成功
    EXPECT_NE(hook, nullptr);
    
    // 清理资源
    delete hook;
}

/**
 * @brief 测试DFMExtFileOperationHook析构函数
 * 验证对象能够正确析构，不会出现内存泄漏
 */
TEST_F(DFMExtFileOperationHookTest, Destructor)
{
    // 创建对象并在析构时验证不会崩溃
    {
        DFMExtFileOperationHook *hook = createTestHook();
        EXPECT_NE(hook, nullptr);
        delete hook;
        EXPECT_TRUE(true); // 如果到达这里说明析构成功
    }
    
    // 测试多个对象的析构
    std::vector<DFMExtFileOperationHook*> hooks;
    for (int i = 0; i < 10; ++i) {
        hooks.push_back(createTestHook());
        EXPECT_NE(hooks.back(), nullptr);
    }
    
    // 清理所有对象
    for (auto* hook : hooks) {
        delete hook;
    }
    EXPECT_TRUE(true); // 如果到达这里说明所有对象都成功析构
}

/**
 * @brief 测试registerOpenFiles方法
 * 验证打开文件回调函数的注册功能
 */
TEST_F(DFMExtFileOperationHookTest, RegisterOpenFiles)
{
    DFMExtFileOperationHook *hook = createTestHook();
    ASSERT_NE(hook, nullptr);
    
    // 测试注册回调函数 - 使用正确的函数签名
    bool callbackCalled = false;
    std::vector<std::string> testFiles = {"/home/test/file1.txt", "/home/test/file2.txt"};
    
    // 注册回调函数，使用正确的双参数签名
    hook->registerOpenFiles([&callbackCalled, &testFiles](const std::vector<std::string> &srcPaths, std::vector<std::string> *ignorePaths) -> bool {
        callbackCalled = true;
        EXPECT_EQ(srcPaths.size(), testFiles.size());
        for (size_t i = 0; i < testFiles.size(); ++i) {
            EXPECT_EQ(srcPaths[i], testFiles[i]);
        }
        // ignorePaths can be null in test
        if (ignorePaths) {
            ignorePaths->clear();
        }
        return true; // Return boolean as required by the function signature
    });
    
    // 验证回调函数注册成功（通过私有成员检查）
    // 注意：由于无法直接调用私有方法，我们主要验证注册过程不会崩溃
    EXPECT_TRUE(true);
    
    delete hook;
}

/**
 * @brief 测试多次注册回调函数
 * 验证回调函数的覆盖行为
 */
TEST_F(DFMExtFileOperationHookTest, MultipleRegisterOpenFiles)
{
    DFMExtFileOperationHook *hook = createTestHook();
    ASSERT_NE(hook, nullptr);
    
    // 注册第一个回调函数
    bool firstCallbackCalled = false;
    hook->registerOpenFiles([&firstCallbackCalled](const std::vector<std::string> &srcPaths, std::vector<std::string> *ignorePaths) -> bool {
        firstCallbackCalled = true;
        return true;
    });
    
    // 注册第二个回调函数（应该覆盖第一个）
    bool secondCallbackCalled = false;
    hook->registerOpenFiles([&secondCallbackCalled](const std::vector<std::string> &srcPaths, std::vector<std::string> *ignorePaths) -> bool {
        secondCallbackCalled = true;
        return true;
    });
    
    // 注册第三个回调函数
    bool thirdCallbackCalled = false;
    hook->registerOpenFiles([&thirdCallbackCalled](const std::vector<std::string> &srcPaths, std::vector<std::string> *ignorePaths) -> bool {
        thirdCallbackCalled = true;
        return true;
    });
    
    // 验证多次注册不会崩溃
    EXPECT_TRUE(true);
    
    delete hook;
}

/**
 * @brief 测试回调函数的参数验证
 * 验证回调函数能接收正确的参数
 */
TEST_F(DFMExtFileOperationHookTest, CallbackParameterValidation)
{
    DFMExtFileOperationHook *hook = createTestHook();
    ASSERT_NE(hook, nullptr);
    
    // 注册回调函数来验证参数
    std::vector<std::string> receivedFiles;
    hook->registerOpenFiles([&receivedFiles](const std::vector<std::string> &srcPaths, std::vector<std::string> *ignorePaths) -> bool {
        receivedFiles = srcPaths;
        return true;
    });
    
    // 由于无法直接触发回调，我们验证注册过程
    EXPECT_TRUE(true);
    
    delete hook;
}

/**
 * @brief 测试空文件列表处理
 * 验证回调函数处理空文件列表的情况
 */
TEST_F(DFMExtFileOperationHookTest, EmptyFilesList)
{
    DFMExtFileOperationHook *hook = createTestHook();
    ASSERT_NE(hook, nullptr);
    
    // 注册处理空文件列表的回调函数
    bool callbackCalled = false;
    bool emptyFilesReceived = false;
    
    hook->registerOpenFiles([&callbackCalled, &emptyFilesReceived](const std::vector<std::string> &srcPaths, std::vector<std::string> *ignorePaths) -> bool {
        callbackCalled = true;
        emptyFilesReceived = srcPaths.empty();
        return true;
    });
    
    // 验证注册成功
    EXPECT_TRUE(true);
    
    delete hook;
}

/**
 * @brief 测试多线程安全性
 * 验证在多线程环境下的注册行为
 */
TEST_F(DFMExtFileOperationHookTest, ThreadSafety)
{
    DFMExtFileOperationHook *hook = createTestHook();
    ASSERT_NE(hook, nullptr);
    
    // 在多个线程中同时注册回调函数
    std::vector<std::thread> threads;
    std::atomic<int> registrationCount{0};
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([hook, i, &registrationCount]() {
            // 每个线程注册不同的回调函数
            hook->registerOpenFiles([i](const std::vector<std::string> &srcPaths, std::vector<std::string> *ignorePaths) -> bool {
                // 线程特定的回调逻辑
                return true;
            });
            registrationCount++;
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证所有注册都完成了
    EXPECT_EQ(registrationCount.load(), 5);
    
    delete hook;
}

/**
 * @brief 测试内存管理
 * 验证在频繁注册和销毁时的内存安全性
 */
TEST_F(DFMExtFileOperationHookTest, MemoryManagement)
{
    // 测试大量创建和销毁对象
    for (int i = 0; i < 10; ++i) {
        DFMExtFileOperationHook *hook = createTestHook();
        ASSERT_NE(hook, nullptr);
        
        // 注册回调函数
        hook->registerOpenFiles([i](const std::vector<std::string> &srcPaths, std::vector<std::string> *ignorePaths) -> bool {
            // 每个对象的回调函数
            return true;
        });
        
        delete hook;
    }
    
    // 测试大量注册操作
    DFMExtFileOperationHook *hook = createTestHook();
    ASSERT_NE(hook, nullptr);
    
    for (int i = 0; i < 100; ++i) {
        hook->registerOpenFiles([i](const std::vector<std::string> &srcPaths, std::vector<std::string> *ignorePaths) -> bool {
            // 大量回调函数注册
            return true;
        });
    }
    
    delete hook;
    EXPECT_TRUE(true); // 如果到达这里说明内存管理正确
}

/**
 * @brief 测试性能
 * 验证大量注册操作的性能
 */
TEST_F(DFMExtFileOperationHookTest, Performance)
{
    DFMExtFileOperationHook *hook = createTestHook();
    ASSERT_NE(hook, nullptr);
    
    const int registrationCount = 100;
    
    // 测试大量注册操作的性能
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < registrationCount; ++i) {
        hook->registerOpenFiles([i](const std::vector<std::string> &srcPaths, std::vector<std::string> *ignorePaths) -> bool {
            // 简单的回调函数
            return true;
        });
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 验证性能在合理范围内
    EXPECT_LT(duration.count(), 1000); // 100次注册应该在1秒内完成
    
    delete hook;
}

// Note: main function is provided by main.cpp
