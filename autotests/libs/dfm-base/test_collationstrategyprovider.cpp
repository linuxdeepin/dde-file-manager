// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collationstrategyprovider.cpp
 * @brief CollationStrategyProvider 单元测试 —— 排序策略提供者 + 线程安全。
 *
 * 覆盖点：
 *   - 单例：instance() 返回同一指针
 *   - strategy() 返回有效引用、可产出排序键
 *   - 同线程内 strategy() 返回同一实例（thread_local，代际未变时不重建）
 *   - latinFirstEnabled() 安全可调用
 *   - strategyChanged 信号：匹配的 dconfig 变更触发、不匹配的不触发
 *   - 代际失效：valueChanged 后 strategy() 重建出新实例
 *   - 【线程安全】多线程并发 strategy() 不崩溃、结果有效
 *   - 【线程安全】并发 strategy() 期间持续变更代际（dconfig 切换竞态）不崩溃
 *   - 【线程安全】不同线程各自拥有独立 thread_local 实例
 */

#include <gtest/gtest.h>

#include <dfm-base/utils/collation/collationstrategyprovider.h>
#include <dfm-base/utils/collation/collationstrategy.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/configs/dconfig/global_dconf_defines.h>

#include <QSignalSpy>
#include <QString>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

using namespace dfmbase;
using GlobalDConfDefines::ConfigPath::kDefaultCfgPath;
using GlobalDConfDefines::BaseConfig::kSortLatinFirstZhCn;

namespace {
/// 通过元对象系统触发 DConfigManager::valueChanged 信号，模拟 dconfig 变更。
/// 信号经 DirectConnection 同步调用已连接的 onDConfigChanged，使代际自增并
/// 发出 strategyChanged。
void emitDConfigValueChanged(const QString &config, const QString &key)
{
    QMetaObject::invokeMethod(DConfigManager::instance(), "valueChanged",
                              Qt::DirectConnection,
                              Q_ARG(QString, config), Q_ARG(QString, key));
}
}   // namespace

class CollationStrategyProviderTest : public testing::Test
{
protected:
    /// 触发一次有效代际变更（匹配的 dconfig 键），用于后续测试
    void bumpGeneration()
    {
        emitDConfigValueChanged(kDefaultCfgPath, kSortLatinFirstZhCn);
    }
};

// 单例：instance() 始终返回同一指针
TEST_F(CollationStrategyProviderTest, SingletonReturnsSameInstance)
{
    auto *a = CollationStrategyProvider::instance();
    auto *b = CollationStrategyProvider::instance();
    ASSERT_NE(a, nullptr);
    EXPECT_EQ(a, b);
}

// strategy() 返回有效引用，可产出非空排序键
TEST_F(CollationStrategyProviderTest, StrategyReturnsValidReference)
{
    const CollationStrategy &s = CollationStrategyProvider::instance()->strategy();
    EXPECT_FALSE(s.sortKey("test_file.txt").isEmpty());
    EXPECT_FALSE(s.sortKey("中文文件").isEmpty());
}

// 同线程内 strategy() 返回同一实例（thread_local，代际未变时不重建）
TEST_F(CollationStrategyProviderTest, SameThreadGetsSameInstanceWithoutRebuild)
{
    auto *provider = CollationStrategyProvider::instance();
    const CollationStrategy &s1 = provider->strategy();
    const CollationStrategy &s2 = provider->strategy();
    EXPECT_EQ(&s1, &s2);
}

// latinFirstEnabled() 安全可调用、返回 bool
TEST_F(CollationStrategyProviderTest, LatinFirstEnabledIsCallable)
{
    bool ok = false;
    EXPECT_NO_FATAL_FAILURE({ ok = CollationStrategyProvider::instance()->latinFirstEnabled(); });
    (void)ok;   // 仅验证不崩溃；默认 dconfig 未注册时为 false
}

// strategyChanged 信号：匹配的 dconfig 键变更时发出
TEST_F(CollationStrategyProviderTest, StrategyChangedEmittedOnMatchingKey)
{
    auto *provider = CollationStrategyProvider::instance();
    QSignalSpy spy(provider, &CollationStrategyProvider::strategyChanged);
    ASSERT_TRUE(spy.isValid());
    bumpGeneration();
    EXPECT_GE(spy.count(), 1);
}

// strategyChanged 信号：不匹配的键变更时不发出
TEST_F(CollationStrategyProviderTest, StrategyChangedNotEmittedOnUnmatchedKey)
{
    auto *provider = CollationStrategyProvider::instance();
    QSignalSpy spy(provider, &CollationStrategyProvider::strategyChanged);
    ASSERT_TRUE(spy.isValid());
    emitDConfigValueChanged(kDefaultCfgPath, "dfm.some.unrelated.key");
    EXPECT_EQ(spy.count(), 0);
}

// 代际失效：valueChanged 后 strategy() 重建出新实例（地址不同且功能正常）
TEST_F(CollationStrategyProviderTest, GenerationChangeRebuildsStrategy)
{
    auto *provider = CollationStrategyProvider::instance();
    const CollationStrategy *before = &provider->strategy();
    bumpGeneration();
    const CollationStrategy *after = &provider->strategy();
    EXPECT_NE(before, after) << "代际变更后应重建出新的策略实例";
    // 新实例功能正常
    EXPECT_FALSE(after->sortKey("test").isEmpty());
}

// 代际多次变更：每次 valueChanged 后 strategy() 实例均重建
TEST_F(CollationStrategyProviderTest, RepeatedGenerationChangesRebuild)
{
    auto *provider = CollationStrategyProvider::instance();
    const CollationStrategy *prev = &provider->strategy();
    for (int i = 0; i < 5; ++i) {
        bumpGeneration();
        const CollationStrategy *cur = &provider->strategy();
        EXPECT_NE(prev, cur) << "第 " << i << " 次代际变更后实例应重建";
        EXPECT_FALSE(cur->sortKey("test").isEmpty());
        prev = cur;
    }
}

// ───────────────────────── 线程安全测试 ─────────────────────────

// 【线程安全】多线程并发 strategy() 不崩溃、所有结果有效
TEST_F(CollationStrategyProviderTest, ConcurrentStrategyAccessIsSafe)
{
    auto *provider = CollationStrategyProvider::instance();
    const int numThreads = 8;
    const int iterations = 500;
    std::atomic<int> emptyKeyFailures { 0 };
    std::atomic<int> crashes { 0 };
    std::vector<std::thread> threads;

    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&]() {
            try {
                for (int i = 0; i < iterations; ++i) {
                    const CollationStrategy &s = provider->strategy();
                    if (s.sortKey("concurrent_test").isEmpty())
                        ++emptyKeyFailures;
                }
            } catch (...) {
                ++crashes;
            }
        });
    }
    for (auto &th : threads)
        th.join();

    EXPECT_EQ(crashes.load(), 0);
    EXPECT_EQ(emptyKeyFailures.load(), 0);
}

// 【线程安全】并发 strategy() 期间持续变更代际（dconfig 切换竞态）不崩溃
// 模拟真实场景：排序工作线程持续取策略，同时 dconfig 变更线程反复触发代际失效。
TEST_F(CollationStrategyProviderTest, ConcurrentAccessWithGenerationChanges)
{
    auto *provider = CollationStrategyProvider::instance();
    const int numWorkers = 8;
    std::atomic<bool> stop { false };
    std::atomic<int> crashes { 0 };
    std::atomic<int> emptyKeys { 0 };
    std::vector<std::thread> threads;

    // 工作线程：持续取策略并产出排序键
    for (int t = 0; t < numWorkers; ++t) {
        threads.emplace_back([&]() {
            try {
                while (!stop.load(std::memory_order_relaxed)) {
                    const CollationStrategy &s = provider->strategy();
                    if (s.sortKey("race_test").isEmpty())
                        ++emptyKeys;
                }
            } catch (...) {
                ++crashes;
            }
        });
    }

    // 主线程：持续触发代际变更
    for (int i = 0; i < 100; ++i) {
        bumpGeneration();
        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
    stop.store(true, std::memory_order_relaxed);
    for (auto &th : threads)
        th.join();

    EXPECT_EQ(crashes.load(), 0) << "并发 + 代际变更期间不应崩溃";
    EXPECT_EQ(emptyKeys.load(), 0) << "所有线程产出的排序键应非空";
}

// 【线程安全】不同线程各自拥有独立的 thread_local 策略实例
// 代际稳定后：同一线程内 strategy() 返回同一实例；不同线程返回不同实例。
// 注意：thread_local 对象在线程退出时销毁，故对子线程策略的访问必须在线程内完成，
// 不得在 join 后解引用子线程返回的策略指针（use-after-free）。
TEST_F(CollationStrategyProviderTest, ThreadLocalInstancesArePerThread)
{
    auto *provider = CollationStrategyProvider::instance();
    // 稳定代际：先 bump 一次并消费，避免跨测试代际漂移干扰
    bumpGeneration();
    (void)provider->strategy();

    const CollationStrategy *mainThreadInstance = &provider->strategy();

    // 在子线程内完成全部验证，避免 join 后访问已销毁的 thread_local 实例
    std::atomic<bool> differentInstance { false };
    std::atomic<bool> otherInstanceFunctional { false };
    std::thread other([&]() {
        const CollationStrategy *inst = &provider->strategy();
        differentInstance.store(inst != mainThreadInstance);
        otherInstanceFunctional.store(!inst->sortKey("test").isEmpty());
    });
    other.join();

    EXPECT_TRUE(differentInstance.load())
        << "不同线程应持有独立的 thread_local 策略实例";
    EXPECT_TRUE(otherInstanceFunctional.load());
    // 主线程实例功能正常
    EXPECT_FALSE(mainThreadInstance->sortKey("test").isEmpty());
}

// 【线程安全】并发 strategy() 期间代际变更后，线程在下次调用时重建且功能正常
// 验证代际失效机制在并发下正确传播：变更后各线程都能拿到可用策略。
TEST_F(CollationStrategyProviderTest, GenerationPropagatesToAllThreads)
{
    auto *provider = CollationStrategyProvider::instance();
    const int numThreads = 6;
    std::atomic<int> functionalFailures { 0 };
    std::vector<std::thread> threads;

    // 先让所有线程拿到当前代际的策略
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&]() {
            (void)provider->strategy();   // 建立本线程 thread_local
        });
    }
    for (auto &th : threads)
        th.join();
    threads.clear();

    // 触发代际变更
    bumpGeneration();

    // 各线程再次取策略：应重建，且功能正常（compare 与 sortKey 一致）
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&]() {
            const CollationStrategy &s = provider->strategy();
            // 验证重建后的策略功能正确
            if (s.compare("a", "b") >= 0 || s.sortKey("x").isEmpty())
                ++functionalFailures;
        });
    }
    for (auto &th : threads)
        th.join();

    EXPECT_EQ(functionalFailures.load(), 0)
        << "代际变更后各线程重建的策略应功能正常";
}
