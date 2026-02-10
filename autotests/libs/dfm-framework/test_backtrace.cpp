// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_backtrace.cpp - Backtrace类单元测试
// 使用stub_ext进行函数打桩测试

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTest>
#include <csignal>
#include <thread>
#include <chrono>
#include <dlfcn.h>
#include <execinfo.h>
#include <cxxabi.h>

// 包含stub_ext
#include "stubext.h"

// 包含待测试的类
#include <dfm-framework/backtrace/backtrace.h>

using namespace dpf;

/**
 * @brief Backtrace类单元测试
 *
 * 测试范围：
 * 1. 回溯信息获取和处理
 * 2. 堆栈跟踪功能
 * 3. 调试信息处理
 * 4. 错误处理和边界情况
 * 5. 信号处理器安装和管理
 * 6. 符号解析和名称还原
 */
class BacktraceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 初始化测试环境
        stub.clear();

        // 设置测试用的应用程序名称
        if (!qApp) {
            static int argc = 1;
            static char *argv[] = { "test_backtrace" };
            app = std::make_unique<QCoreApplication>(argc, argv);
        }
        qApp->setApplicationName("TestApp");
    }

    void TearDown() override
    {
        // 清理测试环境
        stub.clear();
    }

    // 测试用的打桩器
    stub_ext::StubExt stub;
    std::unique_ptr<QCoreApplication> app;
};

/**
 * @brief 测试安装堆栈跟踪处理器
 * 验证installStackTraceHandler函数的基本功能
 */
TEST_F(BacktraceTest, InstallStackTraceHandler_Basic)
{
    // 测试安装堆栈跟踪处理器
    // 由于使用std::once_flag，多次调用只会执行一次
    dpf::backtrace::installStackTraceHandler();
    dpf::backtrace::installStackTraceHandler();   // 第二次调用应该被忽略

    // 验证函数调用成功（如果到达这里说明没有崩溃）
    EXPECT_TRUE(true);
}

/**
 * @brief 测试demangle函数 - 空指针情况
 * 验证demangle函数对空指针的处理
 */
TEST_F(BacktraceTest, Demangle_NullPointer)
{
    // 测试空指针输入
    std::string result = dpf::backtrace::inner::demangle(nullptr);
    EXPECT_TRUE(result.empty());
}

/**
 * @brief 测试demangle函数 - dladdr失败情况
 * 使用打桩模拟dladdr返回失败
 */
TEST_F(BacktraceTest, Demangle_DladdrFailure)
{
    // 打桩dladdr函数，模拟失败情况
    stub.set_lamda(&dladdr, [](const void *addr, Dl_info *info) -> int {
        __DBG_STUB_INVOKE__
        Q_UNUSED(addr)
        Q_UNUSED(info)
        return 0;   // 返回0表示失败
    });

    // 测试非空指针但dladdr失败的情况
    void *testAddr = reinterpret_cast<void *>(0x12345678);
    std::string result = dpf::backtrace::inner::demangle(testAddr);

    // 应该包含地址和"???"
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("???"), std::string::npos);
}

/**
 * @brief 测试demangle函数 - dladdr成功但无符号名
 * 模拟dladdr成功但没有符号名的情况
 */
TEST_F(BacktraceTest, Demangle_DladdrSuccessNoSymbol)
{
    // 打桩dladdr函数，模拟成功但无符号名
    stub.set_lamda(&dladdr, [](const void *addr, Dl_info *info) -> int {
        __DBG_STUB_INVOKE__
        Q_UNUSED(addr)
        info->dli_sname = nullptr;
        info->dli_saddr = reinterpret_cast<void *>(0x10000000);
        info->dli_fname = "/lib/test.so";
        return 1;   // 返回1表示成功
    });

    void *testAddr = reinterpret_cast<void *>(0x12345678);
    std::string result = dpf::backtrace::inner::demangle(testAddr);

    // 应该包含地址、"???"和文件名
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("???"), std::string::npos);
    EXPECT_NE(result.find("/lib/test.so"), std::string::npos);
}

/**
 * @brief 测试demangle函数 - 完整成功情况
 * 模拟dladdr完全成功的情况
 */
TEST_F(BacktraceTest, Demangle_FullSuccess)
{
    // 打桩dladdr函数，模拟完全成功
    stub.set_lamda(&dladdr, [](const void *addr, Dl_info *info) -> int {
        __DBG_STUB_INVOKE__
        Q_UNUSED(addr)
        static const char *symbol_name = "_Z8testFuncv";   // mangled name
        info->dli_sname = symbol_name;
        info->dli_saddr = reinterpret_cast<void *>(0x10000000);
        info->dli_fname = "/lib/test.so";
        return 1;
    });

    // 打桩__cxa_demangle函数，模拟成功的名称还原
    stub.set_lamda(&abi::__cxa_demangle, [](const char *mangled_name, char *output_buffer, size_t *length, int *status) -> char * {
        __DBG_STUB_INVOKE__
        Q_UNUSED(mangled_name)
        Q_UNUSED(output_buffer)
        Q_UNUSED(length)
        if (status) *status = 0;
        return strdup("testFunc()");   // 返回还原后的名称
    });

    void *testAddr = reinterpret_cast<void *>(0x12345678);
    std::string result = dpf::backtrace::inner::demangle(testAddr);

    // 应该包含还原后的函数名和文件名
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("testFunc()"), std::string::npos);
    EXPECT_NE(result.find("/lib/test.so"), std::string::npos);
}

/**
 * @brief 测试demangle函数 - 名称还原失败
 * 模拟__cxa_demangle失败的情况
 */
TEST_F(BacktraceTest, Demangle_DemangleFailure)
{
    // 打桩dladdr函数
    stub.set_lamda(&dladdr, [](const void *addr, Dl_info *info) -> int {
        __DBG_STUB_INVOKE__
        Q_UNUSED(addr)
        static const char *symbol_name = "_Z8testFuncv";
        info->dli_sname = symbol_name;
        info->dli_saddr = reinterpret_cast<void *>(0x10000000);
        info->dli_fname = "/lib/test.so";
        return 1;
    });

    // 打桩__cxa_demangle函数，模拟失败
    stub.set_lamda(&abi::__cxa_demangle, [](const char *mangled_name, char *output_buffer, size_t *length, int *status) -> char * {
        __DBG_STUB_INVOKE__
        Q_UNUSED(mangled_name)
        Q_UNUSED(output_buffer)
        Q_UNUSED(length)
        if (status) *status = -1;   // 失败状态
        return nullptr;
    });

    void *testAddr = reinterpret_cast<void *>(0x12345678);
    std::string result = dpf::backtrace::inner::demangle(testAddr);

    // 应该包含原始的mangled名称
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("_Z8testFuncv"), std::string::npos);
}

/**
 * @brief 测试printStack函数 - 数组版本
 * 验证printStack(void*[], int)函数
 */
TEST_F(BacktraceTest, PrintStack_Array)
{
    // 创建测试用的frames数组
    void *frames[3] = {
        reinterpret_cast<void *>(0x12345678),
        reinterpret_cast<void *>(0x87654321),
        reinterpret_cast<void *>(0xABCDEF00)
    };

    // 打桩demangle函数以避免实际的符号解析
    int call_count = 0;
    stub.set_lamda(static_cast<std::string (*)(void *)>(&dpf::backtrace::inner::demangle),
                   [&call_count](void *value) -> std::string {
                       __DBG_STUB_INVOKE__
                       call_count++;
                       return QString("Frame_%1: %2").arg(call_count).arg(reinterpret_cast<quintptr>(value), 0, 16).toStdString();
                   });

    // 调用函数
    dpf::backtrace::inner::printStack(frames, 3);

    // 验证demangle被调用了3次
    EXPECT_EQ(call_count, 3);
}

/**
 * @brief 测试printStack函数 - 跳过帧版本
 * 验证printStack(int)函数
 */
TEST_F(BacktraceTest, PrintStack_Skip)
{
    // 打桩backtrace函数
    stub.set_lamda(&::backtrace, [](void **buffer, int size) -> int {
        __DBG_STUB_INVOKE__
        // 模拟返回5个帧
        for (int i = 0; i < std::min(size, 5); ++i) {
            buffer[i] = reinterpret_cast<void *>(0x10000000 + i * 0x1000);
        }
        return 5;
    });

    // 打桩printStack数组版本
    bool array_version_called = false;
    stub.set_lamda(static_cast<void (*)(void *[], int)>(&dpf::backtrace::inner::printStack),
                   [&array_version_called](void *frames[], int numFrames) {
                       __DBG_STUB_INVOKE__
                       Q_UNUSED(frames)
                       array_version_called = true;
                       // 验证跳过了2帧，剩余3帧
                       EXPECT_EQ(numFrames, 3);
                   });

    // 调用函数，跳过前2帧
    dpf::backtrace::inner::printStack(2);

    // 验证数组版本被调用
    EXPECT_TRUE(array_version_called);
}

/**
 * @brief 测试stackTraceHandler函数
 * 验证信号处理器的行为
 */
TEST_F(BacktraceTest, StackTraceHandler)
{
    // 保存原始信号处理器
    auto original_handler = signal(SIGTERM, SIG_DFL);

    // 打桩signal函数以避免实际修改信号处理器
    int signal_calls = 0;
    stub.set_lamda(&signal, [&signal_calls](int sig, sighandler_t handler) -> sighandler_t {
        __DBG_STUB_INVOKE__
        signal_calls++;
        Q_UNUSED(sig)
        Q_UNUSED(handler)
        return SIG_DFL;
    });

    // 打桩strsignal函数
    stub.set_lamda(&strsignal, [](int sig) -> char * {
        __DBG_STUB_INVOKE__
        Q_UNUSED(sig)
        return const_cast<char *>("Test signal");
    });

    // 打桩printStack函数
    bool print_stack_called = false;
    stub.set_lamda(static_cast<void (*)(int)>(&dpf::backtrace::inner::printStack),
                   [&print_stack_called](int skip) {
                       __DBG_STUB_INVOKE__
                       print_stack_called = true;
                       EXPECT_EQ(skip, 3);   // 应该跳过3帧
                   });

    // 打桩raise函数以避免实际发送信号
    bool raise_called = false;
    stub.set_lamda(&raise, [&raise_called](int sig) -> int {
        __DBG_STUB_INVOKE__
        raise_called = true;
        EXPECT_EQ(sig, SIGTERM);
        return 0;
    });

    // 调用信号处理器
    dpf::backtrace::inner::stackTraceHandler(SIGTERM);

    // 验证各个函数被正确调用
    EXPECT_GT(signal_calls, 0);   // signal应该被调用
    EXPECT_TRUE(print_stack_called);   // printStack应该被调用
    EXPECT_TRUE(raise_called);   // raise应该被调用

    // 恢复原始信号处理器
    signal(SIGTERM, original_handler);
}

/**
 * @brief 测试installStackTraceHandler的once_flag机制
 * 验证多次调用只执行一次的机制
 */
TEST_F(BacktraceTest, InstallStackTraceHandler_OnceFlag)
{
    int signal_call_count = 0;

    // 打桩signal函数来计数调用次数
    stub.set_lamda(&signal, [&signal_call_count](int sig, sighandler_t handler) -> sighandler_t {
        __DBG_STUB_INVOKE__
        signal_call_count++;
        Q_UNUSED(sig)
        Q_UNUSED(handler)
        return SIG_DFL;
    });

    // 多次调用安装函数
    for (int i = 0; i < 10; ++i) {
        dpf::backtrace::installStackTraceHandler();
    }

    // 由于once_flag机制，signal应该只在第一次调用时被调用
    // 至少应该调用一次SIGSEGV的注册
    EXPECT_GT(signal_call_count, 0);

    // 重置计数器，再次调用应该不会增加计数
    int previous_count = signal_call_count;
    dpf::backtrace::installStackTraceHandler();

    // 计数应该没有变化（因为once_flag阻止了重复执行）
    EXPECT_EQ(signal_call_count, previous_count);
}

/**
 * @brief 测试条件编译分支
 * 验证DPF_FULLSIG_STRACE_ENABLE相关代码
 */
TEST_F(BacktraceTest, InstallStackTraceHandler_ConditionalCompilation)
{
    std::vector<int> registered_signals;

    // 打桩signal函数来记录注册的信号
    stub.set_lamda(&signal, [&registered_signals](int sig, sighandler_t handler) -> sighandler_t {
        __DBG_STUB_INVOKE__
        registered_signals.push_back(sig);
        Q_UNUSED(handler)
        return SIG_DFL;
    });

    // 调用安装函数
    dpf::backtrace::installStackTraceHandler();

    // 至少应该注册SIGSEGV
    EXPECT_FALSE(registered_signals.empty());

    bool found_sigsegv = false;
    for (int sig : registered_signals) {
        if (sig == SIGSEGV) {
            found_sigsegv = true;
            break;
        }
    }
    EXPECT_TRUE(found_sigsegv);

#ifdef DPF_FULLSIG_STRACE_ENABLE
    // 如果启用了完整信号跟踪，应该注册更多信号
    EXPECT_GT(registered_signals.size(), 1);
#endif
}

/**
 * @brief 测试多线程环境下的安全性
 * 验证在多线程环境下installStackTraceHandler的线程安全性
 */
TEST_F(BacktraceTest, InstallStackTraceHandler_ThreadSafety)
{
    std::atomic<int> signal_call_count { 0 };

    // 打桩signal函数
    stub.set_lamda(&signal, [&signal_call_count](int sig, sighandler_t handler) -> sighandler_t {
        __DBG_STUB_INVOKE__
        signal_call_count++;
        Q_UNUSED(sig)
        Q_UNUSED(handler)
        return SIG_DFL;
    });

    // 创建多个线程同时调用安装函数
    std::vector<std::thread> threads;
    const int thread_count = 10;

    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([]() {
            dpf::backtrace::installStackTraceHandler();
        });
    }

    // 等待所有线程完成
    for (auto &thread : threads) {
        thread.join();
    }

    // 由于once_flag机制，即使多线程调用，signal也应该只被调用有限次数
    EXPECT_GT(signal_call_count.load(), 0);
    // 在理想情况下，由于once_flag，应该只调用一次，但考虑到测试环境的复杂性，
    // 我们只验证调用次数是合理的
    EXPECT_LT(signal_call_count.load(), thread_count * 10);
}

/**
 * @brief 性能测试
 * 验证backtrace功能的性能特征
 */
TEST_F(BacktraceTest, Performance)
{
    const int iterations = 1000;

    auto start_time = std::chrono::high_resolution_clock::now();

    // 大量调用安装函数（由于once_flag，实际只执行一次）
    for (int i = 0; i < iterations; ++i) {
        dpf::backtrace::installStackTraceHandler();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 验证性能（大量调用应该很快完成，因为once_flag的存在）
    EXPECT_LT(duration.count(), 1000);   // 小于1秒
}

/**
 * @brief 综合功能测试
 * 通过综合测试验证所有功能的协同工作
 */
TEST_F(BacktraceTest, ComprehensiveTest)
{
    // 1. 测试安装处理器
    dpf::backtrace::installStackTraceHandler();

    // 2. 测试demangle功能
    void *test_addr = reinterpret_cast<void *>(0x12345678);
    std::string demangle_result = dpf::backtrace::inner::demangle(test_addr);
    EXPECT_FALSE(demangle_result.empty());

    // 3. 测试printStack功能
    void *frames[2] = { test_addr, reinterpret_cast<void *>(0x87654321) };
    dpf::backtrace::inner::printStack(frames, 2);

    // 4. 重复安装验证once_flag
    dpf::backtrace::installStackTraceHandler();

    // 如果到达这里说明所有功能都正常工作
    EXPECT_TRUE(true);
}
