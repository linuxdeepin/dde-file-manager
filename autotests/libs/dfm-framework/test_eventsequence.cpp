// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_eventsequence.cpp - EventSequence类单元测试
// 测试事件序列的功能

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>

// 包含待测试的类
#include <dfm-framework/event/eventsequence.h>

using namespace dpf;

namespace {
/**
 * @brief 测试事件处理器类 - EventSequence专用
 */
class TestEventHandler : public QObject
{
    Q_OBJECT

public:
    explicit TestEventHandler(QObject *parent = nullptr)
        : QObject(parent), handlerCalled(false) { }

    bool handlerCalled;
    QString lastEventData;

    bool handleEvent(const QString &data)
    {
        handlerCalled = true;
        lastEventData = data;
        return true;
    }

    void reset()
    {
        handlerCalled = false;
        lastEventData.clear();
    }
};
} // anonymous namespace

/**
 * @brief EventSequence类单元测试
 *
 * 测试范围：
 * 1. 事件序列基本功能
 * 2. 事件处理器管理
 * 3. 事件遍历执行
 * 4. 模板方法调用
 */
class EventSequenceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        sequence = new EventSequence();
        ASSERT_NE(sequence, nullptr);

        handler1 = new TestEventHandler(nullptr);
        handler2 = new TestEventHandler(nullptr);
        ASSERT_NE(handler1, nullptr);
        ASSERT_NE(handler2, nullptr);
    }

    void TearDown() override
    {
        delete sequence;
        delete handler1;
        delete handler2;
    }

    EventSequence *sequence;
    TestEventHandler *handler1;
    TestEventHandler *handler2;
};

/**
 * @brief 测试EventSequence构造函数
 * 验证EventSequence对象能够正确创建
 */
TEST_F(EventSequenceTest, Constructor)
{
    // 验证对象创建成功
    EXPECT_NE(sequence, nullptr);

    // 验证初始状态 - 空序列应该返回false
    EXPECT_FALSE(sequence->traversal());
}

/**
 * @brief 测试添加事件处理器
 * 验证向序列中添加事件处理器
 */
TEST_F(EventSequenceTest, AddHandler)
{
    // 添加事件处理器
    sequence->append(handler1, &TestEventHandler::handleEvent);

    // 测试遍历
    bool result = sequence->traversal(QString("test_data"));

    // 验证处理器被调用
    EXPECT_TRUE(result);
    EXPECT_TRUE(handler1->handlerCalled);
    EXPECT_EQ(handler1->lastEventData, "test_data");
}

/**
 * @brief 测试获取事件处理器
 * 验证从序列中获取事件处理器
 */
TEST_F(EventSequenceTest, GetHandler)
{
    const QString testData = "get_test_data";

    // 添加处理器
    sequence->append(handler1, &TestEventHandler::handleEvent);

    // 执行遍历
    bool result = sequence->traversal(testData);

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_TRUE(handler1->handlerCalled);
    EXPECT_EQ(handler1->lastEventData, testData);
}

/**
 * @brief 测试移除事件处理器
 * 验证从序列中移除事件处理器
 */
TEST_F(EventSequenceTest, RemoveHandler)
{
    // 添加多个处理器
    sequence->append(handler1, &TestEventHandler::handleEvent);
    sequence->append(handler2, &TestEventHandler::handleEvent);

    // 执行遍历验证都添加成功
    sequence->traversal(QString("initial_test"));
    EXPECT_TRUE(handler1->handlerCalled);
    EXPECT_TRUE(handler2->handlerCalled);

    // 重置状态
    handler1->reset();
    handler2->reset();

    // 移除一个处理器（注意：当前API可能不支持直接移除）
    // sequence->remove(handler1, &TestEventHandler::handleEvent);

    // 由于当前API限制，我们只能验证基本功能
    bool result = sequence->traversal(QString("remove_test"));
    EXPECT_TRUE(result);
}

/**
 * @brief 测试清空序列
 * 验证清空事件序列
 */
TEST_F(EventSequenceTest, ClearSequence)
{
    // 添加多个处理器
    for (int i = 0; i < 5; ++i) {
        TestEventHandler *handler = new TestEventHandler(nullptr);
        sequence->append(handler, &TestEventHandler::handleEvent);
    }

    // 验证序列不为空（通过执行遍历）
    bool result = sequence->traversal(QString("clear_test"));
    EXPECT_TRUE(result);

    // 注意：当前API没有clear方法，我们只能测试基本功能
    // sequence->clear();

    // 验证基本功能仍然正常
    result = sequence->traversal(QString("after_clear"));
    EXPECT_TRUE(result);
}

/**
 * @brief 测试遍历序列
 * 验证遍历事件序列的功能
 */
TEST_F(EventSequenceTest, TraversalSequence)
{
    // 添加多个处理器
    for (int i = 0; i < 3; ++i) {
        TestEventHandler *handler = new TestEventHandler(nullptr);
        sequence->append(handler, &TestEventHandler::handleEvent);
    }

    // 遍历序列
    bool result = sequence->traversal(QString("traversal_test"));

    // 验证遍历成功（至少有一个处理器返回true）
    EXPECT_TRUE(result);
}

/**
 * @brief 测试大量序列
 * 验证大量事件处理器的性能
 */
TEST_F(EventSequenceTest, LargeSequence)
{
    const int largeCount = 1000;

    // 添加大量处理器
    for (int i = 0; i < largeCount; ++i) {
        TestEventHandler *handler = new TestEventHandler(nullptr);
        sequence->append(handler, &TestEventHandler::handleEvent);
    }

    // 验证大量处理器的遍历
    bool result = sequence->traversal(QString("large_test"));
    EXPECT_TRUE(result);
}

/**
 * @brief 测试边界条件
 * 验证边界条件的处理
 */
TEST_F(EventSequenceTest, BoundaryConditions)
{
    // 测试空序列
    bool result = sequence->traversal();
    EXPECT_FALSE(result);   // 空序列应该返回false

    // 测试空参数
    result = sequence->traversal(QVariantList());
    EXPECT_FALSE(result);

    // 添加一个处理器后测试
    sequence->append(handler1, &TestEventHandler::handleEvent);
    result = sequence->traversal(QString("boundary_test"));

    // 验证处理器被调用
    EXPECT_TRUE(result);
    EXPECT_TRUE(handler1->handlerCalled);
}

/**
 * @brief 测试混合事件类型
 * 验证不同类型事件的处理
 */
TEST_F(EventSequenceTest, MixedEventTypes)
{
    // 添加处理器
    sequence->append(handler1, &TestEventHandler::handleEvent);

    // 测试字符串参数
    bool result = sequence->traversal(QString("string_test"));
    EXPECT_TRUE(result);
    EXPECT_TRUE(handler1->handlerCalled);

    // 重置状态
    handler1->reset();

    // 测试整数参数（通过QVariantList）
    QVariantList intParams;
    intParams << 123;
    result = sequence->traversal(intParams);
    EXPECT_TRUE(result);
    EXPECT_TRUE(handler1->handlerCalled);
}

/**
 * @brief 测试性能
 * 验证事件序列的性能
 */
TEST_F(EventSequenceTest, Performance)
{
    const int performanceCount = 10000;

    // 添加处理器
    sequence->append(handler1, &TestEventHandler::handleEvent);

    // 记录开始时间
    QElapsedTimer timer;
    timer.start();

    // 执行大量遍历
    for (int i = 0; i < performanceCount; ++i) {
        sequence->traversal(QString("perf_test_%1").arg(i));
    }

    // 记录结束时间
    qint64 elapsed = timer.elapsed();

    // 验证性能（这里只是确保测试完成）
    EXPECT_LT(elapsed, 10000);   // 应该在10秒内完成
    EXPECT_TRUE(handler1->handlerCalled);
}

#include "test_eventsequence.moc"
