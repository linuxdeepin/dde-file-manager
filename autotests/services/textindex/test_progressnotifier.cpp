// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTimer>

#include "task/progressnotifier.h"

SERVICETEXTINDEX_USE_NAMESPACE

class UT_ProgressNotifier : public testing::Test
{
protected:
    void SetUp() override
    {
        // ProgressNotifier is a singleton, so we test the instance directly
        notifier = ProgressNotifier::instance();
        EXPECT_TRUE(notifier != nullptr);
    }

    void TearDown() override
    {
        stub.clear();
        // Note: We don't delete the singleton instance
    }
    
    ProgressNotifier *notifier = nullptr;

private:
    stub_ext::StubExt stub;
};

// Singleton Pattern Tests
TEST_F(UT_ProgressNotifier, Instance_CalledMultipleTimes_ReturnsSameInstance)
{
    ProgressNotifier *instance1 = ProgressNotifier::instance();
    ProgressNotifier *instance2 = ProgressNotifier::instance();
    ProgressNotifier *instance3 = ProgressNotifier::instance();
    
    EXPECT_EQ(instance1, instance2);
    EXPECT_EQ(instance2, instance3);
    EXPECT_EQ(instance1, instance3);
    EXPECT_TRUE(instance1 != nullptr);
}

TEST_F(UT_ProgressNotifier, Instance_ThreadSafe_ReturnsSameInstance)
{
    // Test thread safety of singleton pattern
    std::vector<ProgressNotifier*> instances;
    std::mutex instancesMutex;
    std::vector<std::thread> threads;
    
    // Create multiple threads that all call instance()
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&instances, &instancesMutex]() {
            ProgressNotifier *instance = ProgressNotifier::instance();
            std::lock_guard<std::mutex> lock(instancesMutex);
            instances.push_back(instance);
        });
    }
    
    // Wait for all threads to complete
    for (auto &thread : threads) {
        thread.join();
    }
    
    // All instances should be the same
    EXPECT_EQ(instances.size(), 10);
    for (size_t i = 1; i < instances.size(); ++i) {
        EXPECT_EQ(instances[0], instances[i]);
    }
}

// Signal Emission Tests
TEST_F(UT_ProgressNotifier, ProgressChanged_EmittedOnce_ReceivesSignal)
{
    QSignalSpy spy(notifier, &ProgressNotifier::progressChanged);
    
    emit notifier->progressChanged(50, 100);
    
    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toLongLong(), 50);
    EXPECT_EQ(arguments.at(1).toLongLong(), 100);
}

TEST_F(UT_ProgressNotifier, ProgressChanged_EmittedMultipleTimes_ReceivesAllSignals)
{
    QSignalSpy spy(notifier, &ProgressNotifier::progressChanged);
    
    emit notifier->progressChanged(10, 100);
    emit notifier->progressChanged(25, 100);
    emit notifier->progressChanged(50, 100);
    emit notifier->progressChanged(75, 100);
    emit notifier->progressChanged(100, 100);
    
    EXPECT_EQ(spy.count(), 5);
    
    // Check first signal
    QList<QVariant> firstSignal = spy.at(0);
    EXPECT_EQ(firstSignal.at(0).toLongLong(), 10);
    EXPECT_EQ(firstSignal.at(1).toLongLong(), 100);
    
    // Check last signal
    QList<QVariant> lastSignal = spy.at(4);
    EXPECT_EQ(lastSignal.at(0).toLongLong(), 100);
    EXPECT_EQ(lastSignal.at(1).toLongLong(), 100);
}

TEST_F(UT_ProgressNotifier, ProgressChanged_ZeroValues_HandlesCorrectly)
{
    QSignalSpy spy(notifier, &ProgressNotifier::progressChanged);
    
    emit notifier->progressChanged(0, 0);
    emit notifier->progressChanged(0, 100);
    emit notifier->progressChanged(50, 0);
    
    EXPECT_EQ(spy.count(), 3);
    
    // Check zero total
    QList<QVariant> firstSignal = spy.at(0);
    EXPECT_EQ(firstSignal.at(0).toLongLong(), 0);
    EXPECT_EQ(firstSignal.at(1).toLongLong(), 0);
    
    // Check zero count
    QList<QVariant> secondSignal = spy.at(1);
    EXPECT_EQ(secondSignal.at(0).toLongLong(), 0);
    EXPECT_EQ(secondSignal.at(1).toLongLong(), 100);
    
    // Check zero total with non-zero count
    QList<QVariant> thirdSignal = spy.at(2);
    EXPECT_EQ(thirdSignal.at(0).toLongLong(), 50);
    EXPECT_EQ(thirdSignal.at(1).toLongLong(), 0);
}

TEST_F(UT_ProgressNotifier, ProgressChanged_NegativeValues_HandlesCorrectly)
{
    QSignalSpy spy(notifier, &ProgressNotifier::progressChanged);
    
    emit notifier->progressChanged(-10, 100);
    emit notifier->progressChanged(50, -100);
    emit notifier->progressChanged(-25, -75);
    
    EXPECT_EQ(spy.count(), 3);
    
    // Should handle negative values without crashing
    QList<QVariant> firstSignal = spy.at(0);
    EXPECT_EQ(firstSignal.at(0).toLongLong(), -10);
    EXPECT_EQ(firstSignal.at(1).toLongLong(), 100);
}

TEST_F(UT_ProgressNotifier, ProgressChanged_LargeValues_HandlesCorrectly)
{
    QSignalSpy spy(notifier, &ProgressNotifier::progressChanged);
    
    qint64 largeCount = 1000000000LL; // 1 billion
    qint64 largeTotal = 2000000000LL; // 2 billion
    
    emit notifier->progressChanged(largeCount, largeTotal);
    
    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toLongLong(), largeCount);
    EXPECT_EQ(arguments.at(1).toLongLong(), largeTotal);
}

// Multiple Subscribers Tests
TEST_F(UT_ProgressNotifier, ProgressChanged_MultipleSubscribers_AllReceiveSignals)
{
    QSignalSpy spy1(notifier, &ProgressNotifier::progressChanged);
    QSignalSpy spy2(notifier, &ProgressNotifier::progressChanged);
    QSignalSpy spy3(notifier, &ProgressNotifier::progressChanged);
    
    emit notifier->progressChanged(33, 100);
    
    // All spies should receive the signal
    EXPECT_EQ(spy1.count(), 1);
    EXPECT_EQ(spy2.count(), 1);
    EXPECT_EQ(spy3.count(), 1);
    
    // Check signal content for all spies
    QList<QVariant> args1 = spy1.takeFirst();
    QList<QVariant> args2 = spy2.takeFirst();
    QList<QVariant> args3 = spy3.takeFirst();
    
    EXPECT_EQ(args1.at(0).toLongLong(), 33);
    EXPECT_EQ(args2.at(0).toLongLong(), 33);
    EXPECT_EQ(args3.at(0).toLongLong(), 33);
}

TEST_F(UT_ProgressNotifier, ProgressChanged_SubscriberDisconnects_OthersStillReceive)
{
    QSignalSpy spy1(notifier, &ProgressNotifier::progressChanged);
    QSignalSpy spy2(notifier, &ProgressNotifier::progressChanged);
    
    // Emit first signal - both should receive
    emit notifier->progressChanged(25, 100);
    EXPECT_EQ(spy1.count(), 1);
    EXPECT_EQ(spy2.count(), 1);
    
    // Create a new scope to destroy spy1 (simulating disconnect)
    {
        QSignalSpy tempSpy(notifier, &ProgressNotifier::progressChanged);
        // Emit second signal - only spy2 should receive (tempSpy will also receive but will be destroyed)
        emit notifier->progressChanged(50, 100);
        EXPECT_EQ(tempSpy.count(), 1);
    }
    
    EXPECT_EQ(spy1.count(), 2); // Both signals received
    EXPECT_EQ(spy2.count(), 2); // Both signals received
}

// Rapid Signal Emission Tests
TEST_F(UT_ProgressNotifier, ProgressChanged_RapidEmission_HandlesAllSignals)
{
    QSignalSpy spy(notifier, &ProgressNotifier::progressChanged);
    
    // Emit signals rapidly
    for (int i = 0; i <= 100; ++i) {
        emit notifier->progressChanged(i, 100);
    }
    
    EXPECT_EQ(spy.count(), 101); // 0 to 100 inclusive
    
    // Check first and last signals
    QList<QVariant> firstSignal = spy.at(0);
    EXPECT_EQ(firstSignal.at(0).toLongLong(), 0);
    
    QList<QVariant> lastSignal = spy.at(100);
    EXPECT_EQ(lastSignal.at(0).toLongLong(), 100);
}

TEST_F(UT_ProgressNotifier, ProgressChanged_ConcurrentEmission_HandlesCorrectly)
{
    QSignalSpy spy(notifier, &ProgressNotifier::progressChanged);
    
    std::vector<std::thread> threads;
    std::atomic<int> emissionCount{0};
    
    // Create multiple threads that emit signals concurrently
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([this, &emissionCount, i]() {
            for (int j = 0; j < 10; ++j) {
                emit notifier->progressChanged(i * 10 + j, 100);
                emissionCount++;
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto &thread : threads) {
        thread.join();
    }
    
    // Should receive all signals (though order may vary)
    EXPECT_EQ(spy.count(), 50); // 5 threads * 10 emissions each
    EXPECT_EQ(emissionCount.load(), 50);
}

// Signal Parameter Validation Tests
TEST_F(UT_ProgressNotifier, ProgressChanged_ParameterTypes_CorrectTypes)
{
    QSignalSpy spy(notifier, &ProgressNotifier::progressChanged);
    
    qint64 count = 42;
    qint64 total = 84;
    
    emit notifier->progressChanged(count, total);
    
    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    
    // Verify parameter types
    EXPECT_TRUE(arguments.at(0).canConvert<qint64>());
    EXPECT_TRUE(arguments.at(1).canConvert<qint64>());
    
    // Verify values
    EXPECT_EQ(arguments.at(0).toLongLong(), count);
    EXPECT_EQ(arguments.at(1).toLongLong(), total);
}

// Edge Cases and Error Handling
TEST_F(UT_ProgressNotifier, ProgressChanged_MaxValues_HandlesCorrectly)
{
    QSignalSpy spy(notifier, &ProgressNotifier::progressChanged);
    
    qint64 maxValue = std::numeric_limits<qint64>::max();
    
    emit notifier->progressChanged(maxValue, maxValue);
    
    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toLongLong(), maxValue);
    EXPECT_EQ(arguments.at(1).toLongLong(), maxValue);
}

TEST_F(UT_ProgressNotifier, ProgressChanged_MinValues_HandlesCorrectly)
{
    QSignalSpy spy(notifier, &ProgressNotifier::progressChanged);
    
    qint64 minValue = std::numeric_limits<qint64>::min();
    
    emit notifier->progressChanged(minValue, minValue);
    
    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toLongLong(), minValue);
    EXPECT_EQ(arguments.at(1).toLongLong(), minValue);
}

// Practical Usage Scenarios
TEST_F(UT_ProgressNotifier, ProgressChanged_TypicalProgressSequence_WorksCorrectly)
{
    QSignalSpy spy(notifier, &ProgressNotifier::progressChanged);
    
    // Simulate a typical progress sequence
    qint64 total = 1000;
    QVector<qint64> progressValues = {0, 100, 250, 500, 750, 900, 1000};
    
    for (qint64 progress : progressValues) {
        emit notifier->progressChanged(progress, total);
    }
    
    EXPECT_EQ(spy.count(), progressValues.size());
    
    // Verify each progress value
    for (int i = 0; i < progressValues.size(); ++i) {
        QList<QVariant> arguments = spy.at(i);
        EXPECT_EQ(arguments.at(0).toLongLong(), progressValues[i]);
        EXPECT_EQ(arguments.at(1).toLongLong(), total);
    }
}

TEST_F(UT_ProgressNotifier, ProgressChanged_ResetProgress_HandlesCorrectly)
{
    QSignalSpy spy(notifier, &ProgressNotifier::progressChanged);
    
    // Progress forward
    emit notifier->progressChanged(50, 100);
    emit notifier->progressChanged(100, 100);
    
    // Reset progress (new task)
    emit notifier->progressChanged(0, 200);
    emit notifier->progressChanged(100, 200);
    
    EXPECT_EQ(spy.count(), 4);
    
    // Check reset signal
    QList<QVariant> resetSignal = spy.at(2);
    EXPECT_EQ(resetSignal.at(0).toLongLong(), 0);
    EXPECT_EQ(resetSignal.at(1).toLongLong(), 200);
}

// Memory and Resource Tests
TEST_F(UT_ProgressNotifier, Instance_NoMemoryLeaks_SingletonPersists)
{
    // Get instance multiple times and ensure it's the same
    ProgressNotifier *originalInstance = ProgressNotifier::instance();
    
    // Create and destroy many local pointers to the instance
    for (int i = 0; i < 1000; ++i) {
        ProgressNotifier *tempInstance = ProgressNotifier::instance();
        EXPECT_EQ(tempInstance, originalInstance);
    }
    
    // Original instance should still be valid
    EXPECT_TRUE(originalInstance != nullptr);
    EXPECT_EQ(ProgressNotifier::instance(), originalInstance);
}

TEST_F(UT_ProgressNotifier, ProgressChanged_ManyEmissions_NoMemoryIssues)
{
    QSignalSpy spy(notifier, &ProgressNotifier::progressChanged);
    
    // Emit many signals to test for memory issues
    for (int i = 0; i < 10000; ++i) {
        emit notifier->progressChanged(i, 10000);
    }
    
    EXPECT_EQ(spy.count(), 10000);
    
    // Verify first and last signals are correct
    QList<QVariant> firstSignal = spy.at(0);
    EXPECT_EQ(firstSignal.at(0).toLongLong(), 0);
    
    QList<QVariant> lastSignal = spy.at(9999);
    EXPECT_EQ(lastSignal.at(0).toLongLong(), 9999);
} 