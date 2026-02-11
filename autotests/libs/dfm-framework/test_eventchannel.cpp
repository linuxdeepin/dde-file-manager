// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_eventchannel.cpp - EventChannel类单元测试
// 基于原始ut_eventchannel.cpp，使用新的测试架构

#include <gtest/gtest.h>

// 包含待测试的类
#include <dfm-framework/event/eventchannel.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

/**
 * @brief 测试用的QObject类
 */
class TestReceiver : public QObject
{
    Q_OBJECT
public:
    explicit TestReceiver(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    int addOne(int value) { return value + 1; }
    int addTen(int value) { return value + 10; }
    QString combineStrings(const QString &a, const QString &b) { return a + b; }
    QVariant processVariant(const QVariant &var) { return var; }
    void voidFunction() { lastCallCount++; }
    int getCallCount() { return lastCallCount; }

private:
    int lastCallCount = 0;
};

/**
 * @brief EventChannel类单元测试
 *
 * 测试范围：
 * 1. EventChannel基本功能（同步/异步发送）
 * 2. EventChannelFuture功能（状态管理、取消等）
 * 3. EventChannelManager管理功能（连接/断开、推送等）
 * 4. 错误处理和边界情况
 */
class EventChannelTest : public ::testing::Test
{
protected:
    // 在整个测试套件运行前执行一次
    static void SetUpTestSuite()
    {
        // 强制Event单例在任何测试开始前被初始化，
        // 从而解决静态初始化顺序问题。
        Event::instance();
    }

    void SetUp() override
    {
        receiver = new TestReceiver(nullptr);
        event = Event::instance();
        channelManager = event->channel();
        
        // 确保每个测试开始时都有干净的状态
        testEventTypes.clear();
    }

    void TearDown() override
    {
        // 清理注册的事件类型
        for (const auto& eventType : testEventTypes) {
            channelManager->disconnect(eventType);
        }
        
        delete receiver;
    }
    
    // 辅助方法：注册测试事件类型
    EventType registerTestEvent(const QString& topic) {
        QString fullTopic = "slot_" + topic;  // 使用下划线而不是点号
        event->registerEventType(EventStratege::kSlot, "test", fullTopic);
        EventType eventType = event->eventType("test", fullTopic);
        testEventTypes.append(eventType);
        return eventType;
    }

    TestReceiver *receiver = nullptr;
    Event *event = nullptr;
    EventChannelManager *channelManager = nullptr;
    QList<EventType> testEventTypes;  // 记录测试中注册的事件类型，用于清理
};

/**
 * @brief 测试EventChannel基本功能
 */
TEST_F(EventChannelTest, BasicChannelFunctionality)
{
    EventChannel channel;
    
    // 设置接收器
    channel.setReceiver(receiver, &TestReceiver::addOne);
    
    // 测试同步发送
    QVariant result = channel.send(5);
    EXPECT_EQ(result.toInt(), 6);
    
    // 测试带参数列表的发送
    QVariantList params;
    params << 10;
    result = channel.send(params);
    EXPECT_EQ(result.toInt(), 11);
    
    // 测试无参数发送
    channel.setReceiver(receiver, &TestReceiver::getCallCount);
    result = channel.send();
    EXPECT_EQ(result.toInt(), 0);  // 初始调用次数
}

/**
 * @brief 测试EventChannel模板发送方法
 */
TEST_F(EventChannelTest, TemplateChannelSend)
{
    EventChannel channel;
    channel.setReceiver(receiver, &TestReceiver::addTen);
    
    // 测试模板发送方法
    QVariant result = channel.send(15);
    EXPECT_EQ(result.toInt(), 25);
    
    // 测试多参数模板发送
    channel.setReceiver(receiver, &TestReceiver::combineStrings);
    result = channel.send(QString("Hello"), QString(" World"));
    EXPECT_EQ(result.toString(), QString("Hello World"));
}

/**
 * @brief 测试EventChannel异步发送
 */
TEST_F(EventChannelTest, AsyncChannelSend)
{
    EventChannel channel;
    channel.setReceiver(receiver, &TestReceiver::addOne);
    
    // 测试异步发送
    EventChannelFuture future = channel.asyncSend(20);
    
    // 验证future基本状态
    EXPECT_TRUE(future.isStarted() || future.isFinished());
    
    // 等待完成并获取结果
    future.waitForFinished();
    EXPECT_TRUE(future.isFinished());
    EXPECT_FALSE(future.isRunning());
    
    QVariant result = future.result();
    EXPECT_EQ(result.toInt(), 21);
}

/**
 * @brief 测试EventChannelFuture状态管理
 */
TEST_F(EventChannelTest, ChannelFutureStateManagement)
{
    EventChannel channel;
    channel.setReceiver(receiver, &TestReceiver::addOne);
    
    // 测试异步发送和状态
    EventChannelFuture future = channel.asyncSend(1);
    
    // 快速操作可能立即完成
    bool wasStarted = future.isStarted();
    bool wasFinished = future.isFinished();
    bool wasRunning = future.isRunning();
    
    // 等待完成
    future.waitForFinished();
    
    // 完成后的状态
    EXPECT_TRUE(future.isFinished());
    EXPECT_FALSE(future.isRunning());
    
    // 验证结果
    QVariant result = future.result();
    EXPECT_EQ(result.toInt(), 2);
}

/**
 * @brief 测试EventChannelFuture取消功能
 */
TEST_F(EventChannelTest, ChannelFutureCancel)
{
    EventChannel channel;
    channel.setReceiver(receiver, &TestReceiver::addOne);
    
    EventChannelFuture future = channel.asyncSend(1);
    
    // 尝试取消（可能已经完成）
    future.cancel();
    
    // 验证状态 - 取消成功或已经完成
    bool wasCanceled = future.isCanceled();
    bool wasFinished = future.isFinished();
    
    EXPECT_TRUE(wasCanceled || wasFinished);
}

/**
 * @brief 测试EventChannelManager连接功能
 */
TEST_F(EventChannelTest, ChannelManagerConnect)
{
    // 注册事件类型
    EventType eventType = registerTestEvent("addone");
    
    // 连接处理器
    bool connected = channelManager->connect(eventType, receiver, &TestReceiver::addOne);
    EXPECT_TRUE(connected);
    
    // 测试推送
    QVariant result = channelManager->push(eventType, 100);
    EXPECT_EQ(result.toInt(), 101);
}

/**
 * @brief 测试EventChannelManager断开连接
 */
TEST_F(EventChannelTest, ChannelManagerDisconnect)
{
    // 注册和连接
    EventType eventType = registerTestEvent("disconnect");
    
    channelManager->connect(eventType, receiver, &TestReceiver::addTen);
    
    // 验证连接有效
    QVariant result = channelManager->push(eventType, 5);
    EXPECT_EQ(result.toInt(), 15);
    
    // 断开连接
    bool disconnected = channelManager->disconnect(eventType);
    EXPECT_TRUE(disconnected);
    
    // 验证断开后无结果
    result = channelManager->push(eventType, 5);
    EXPECT_FALSE(result.isValid());
    
    // 重复断开应该返回false
    bool disconnected2 = channelManager->disconnect(eventType);
    EXPECT_FALSE(disconnected2);
}

/**
 * @brief 测试EventChannelManager异步推送
 */
TEST_F(EventChannelTest, ChannelManagerAsyncPost)
{
    // 注册事件类型
    EventType eventType = registerTestEvent("async");
    
    // 连接处理器
    channelManager->connect(eventType, receiver, &TestReceiver::addOne);
    
    // 异步推送
    EventChannelFuture future = channelManager->post(eventType, 50);
    
    // 等待完成
    future.waitForFinished();
    EXPECT_TRUE(future.isFinished());
    
    QVariant result = future.result();
    EXPECT_EQ(result.toInt(), 51);
}

/**
 * @brief 测试EventChannelManager多参数推送
 */
TEST_F(EventChannelTest, ChannelManagerMultiParams)
{
    // 注册事件类型
    EventType eventType = registerTestEvent("combine");
    
    // 连接字符串组合处理器
    channelManager->connect(eventType, receiver, &TestReceiver::combineStrings);
    
    // 测试多参数推送
    QVariant result = channelManager->push(eventType, QString("Hello"), QString(" World"));
    EXPECT_EQ(result.toString(), QString("Hello World"));
    
    // 测试异步多参数推送
    EventChannelFuture future = channelManager->post(eventType, QString("Async"), QString(" Test"));
    future.waitForFinished();
    
    QVariant asyncResult = future.result();
    EXPECT_EQ(asyncResult.toString(), QString("Async Test"));
}

/**
 * @brief 测试无效事件类型处理
 */
TEST_F(EventChannelTest, InvalidEventTypeHandling)
{
    EventType invalidType;
    
    // 尝试连接无效类型
    bool connected = channelManager->connect(invalidType, receiver, &TestReceiver::addOne);
    EXPECT_FALSE(connected);
    
    // 尝试推送到无效类型
    QVariant result = channelManager->push(invalidType, 1);
    EXPECT_FALSE(result.isValid());
    
    // 尝试异步推送到无效类型
    EventChannelFuture future = channelManager->post(invalidType, 1);
    future.waitForFinished();
    QVariant asyncResult = future.result();
    EXPECT_FALSE(asyncResult.isValid());
}

/**
 * @brief 测试重复连接处理
 */
TEST_F(EventChannelTest, ReconnectHandling)
{
    // 注册事件类型
    EventType eventType = registerTestEvent("reconnect");
    
    // 第一次连接
    channelManager->connect(eventType, receiver, &TestReceiver::addOne);
    QVariant result1 = channelManager->push(eventType, 10);
    EXPECT_EQ(result1.toInt(), 11);
    
    // 重新连接到不同的处理器 - 应该替换原有连接
    channelManager->connect(eventType, receiver, &TestReceiver::addTen);
    QVariant result2 = channelManager->push(eventType, 10);
    EXPECT_EQ(result2.toInt(), 20);  // 应该使用新的处理器
}

/**
 * @brief 测试空参数推送
 */
TEST_F(EventChannelTest, EmptyParameterPush)
{
    // 注册事件类型
    EventType eventType = registerTestEvent("void");
    
    // 连接void函数
    channelManager->connect(eventType, receiver, &TestReceiver::voidFunction);
    
    // 推送空参数
    QVariant result = channelManager->push(eventType);
    
    // void函数返回无效QVariant是正常的
    EXPECT_FALSE(result.isValid());
    
    // 验证函数确实被调用了
    EXPECT_EQ(receiver->getCallCount(), 1);
}

/**
 * @brief 测试EventChannel无连接时的行为
 * 这个测试直接调用eventchannel.cpp中的send()实现
 */
TEST_F(EventChannelTest, ChannelWithoutConnection)
{
    EventChannel channel;
    
    // 没有设置接收器时发送 - 这会调用eventchannel.cpp中的send()方法
    QVariant result = channel.send(5);
    EXPECT_FALSE(result.isValid());
    
    // 测试无参数版本 - 这会调用eventchannel.cpp中的send()重载
    QVariant result2 = channel.send();
    EXPECT_FALSE(result2.isValid());
    
    // 测试带参数列表版本 - 直接调用send(QVariantList)
    QVariantList params;
    params << 42;
    QVariant result3 = channel.send(params);
    EXPECT_FALSE(result3.isValid());
    
    // 异步发送也应该返回无效结果 - 这会调用asyncSend()方法
    EventChannelFuture future = channel.asyncSend(5);
    future.waitForFinished();
    result = future.result();
    EXPECT_FALSE(result.isValid());
    
    // 测试异步发送无参数版本
    EventChannelFuture future2 = channel.asyncSend();
    future2.waitForFinished();
    QVariant result4 = future2.result();
    EXPECT_FALSE(result4.isValid());
    
    // 测试异步发送带参数列表版本
    EventChannelFuture future3 = channel.asyncSend(params);
    future3.waitForFinished();
    QVariant result5 = future3.result();
    EXPECT_FALSE(result5.isValid());
}

/**
 * @brief 测试线程安全性（基础测试）
 */
TEST_F(EventChannelTest, BasicThreadSafety)
{
    // 注册事件类型
    EventType eventType = registerTestEvent("threadsafe");
    
    // 连接处理器
    channelManager->connect(eventType, receiver, &TestReceiver::addOne);
    
    // 在当前线程中多次调用（模拟并发）
    std::vector<QVariant> results;
    for (int i = 0; i < 10; ++i) {
        results.push_back(channelManager->push(eventType, i));
    }
    
    // 验证结果
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(results[i].toInt(), i + 1);
    }
}

/**
 * @brief 测试EventChannelFuture所有方法
 * 直接测试eventchannel.cpp中EventChannelFuture的实现
 */
TEST_F(EventChannelTest, EventChannelFutureAllMethods)
{
    EventChannel channel;
    channel.setReceiver(receiver, &TestReceiver::addOne);
    
    // 创建一个Future - 这会调用eventchannel.cpp中的方法
    EventChannelFuture future = channel.asyncSend(100);
    
    // 测试所有EventChannelFuture的方法 - 这些都在eventchannel.cpp中实现
    bool started = future.isStarted();
    bool running = future.isRunning();
    bool finished = future.isFinished();
    bool canceled = future.isCanceled();
    
    // 记录初始状态
    EXPECT_TRUE(started || finished);  // 要么开始了要么已经完成
    
    // 等待完成 - 调用waitForFinished()
    future.waitForFinished();
    
    // 检查完成后的状态
    EXPECT_TRUE(future.isFinished());
    EXPECT_FALSE(future.isRunning());
    
    // 获取结果 - 调用result()
    QVariant result = future.result();
    EXPECT_EQ(result.toInt(), 101);
    
    // 测试取消功能（在一个新的future上）
    EventChannelFuture future2 = channel.asyncSend(200);
    future2.cancel();  // 调用cancel()
    
    // 检查取消状态
    bool wasCanceled = future2.isCanceled();
    bool wasFinished = future2.isFinished();
    EXPECT_TRUE(wasCanceled || wasFinished);
}

/**
 * @brief 测试EventChannelManager的disconnect方法
 * 直接测试eventchannel.cpp中disconnect()的实现
 */
TEST_F(EventChannelTest, ChannelManagerDirectDisconnect)
{
    // 注册事件类型
    EventType eventType = registerTestEvent("disconnect_direct");
    
    // 连接处理器
    channelManager->connect(eventType, receiver, &TestReceiver::addOne);
    
    // 验证连接有效
    QVariant result = channelManager->push(eventType, 10);
    EXPECT_EQ(result.toInt(), 11);
    
    // 调用disconnect(EventType) - 这个方法在eventchannel.cpp中实现
    bool disconnected = channelManager->disconnect(eventType);
    EXPECT_TRUE(disconnected);
    
    // 验证断开后无结果
    result = channelManager->push(eventType, 10);
    EXPECT_FALSE(result.isValid());
    
    // 重复断开应该返回false - 再次调用.cpp中的方法
    bool disconnected2 = channelManager->disconnect(eventType);
    EXPECT_FALSE(disconnected2);
    
    // 测试字符串版本的disconnect - 这也会调用到.cpp中的实现
    event->registerEventType(EventStratege::kSlot, "test", "slot_stringdisconnect");
    bool connected = channelManager->connect("test", "slot_stringdisconnect", receiver, &TestReceiver::addOne);
    EXPECT_TRUE(connected);
    
    // 使用字符串接口断开 - 内部会调用.cpp中的disconnect方法
    bool stringDisconnected = channelManager->disconnect("test", "slot_stringdisconnect");
    EXPECT_TRUE(stringDisconnected);
}

#include "test_eventchannel.moc"