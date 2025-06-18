// test_eventchannel.cpp - EventChannel类单元测试
// 基于原始ut_eventchannel.cpp，使用新的测试架构

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>

// 包含测试框架
#include "../../framework/dfm-test-base.h"

// 包含待测试的类
#include <dfm-framework/event/event.h>

using namespace dpf;

/**
 * @brief EventChannel类单元测试
 * 
 * 测试范围：
 * 1. 事件管理器基本功能
 * 2. 事件通道管理
 * 3. 事件类型注册
 * 4. 错误处理
 */
class EventChannelTest : public ::testing::Test
{
protected:
    void SetUp() override {
        event = Event::instance();
        ASSERT_NE(event, nullptr);
        channelManager = event->channel();
        ASSERT_NE(channelManager, nullptr);
    }
    
    void TearDown() override {
        // Event是单例，不需要删除
    }
    
    Event *event;
    EventChannelManager *channelManager;
};

/**
 * @brief 测试单例模式
 * 验证Event::instance()返回同一个实例
 */
TEST_F(EventChannelTest, SingletonPattern)
{
    Event *instance1 = Event::instance();
    Event *instance2 = Event::instance();
    
    // 验证是同一个实例
    EXPECT_EQ(instance1, instance2);
    EXPECT_EQ(event, instance1);
    
    // 验证实例不为空
    EXPECT_NE(instance1, nullptr);
}

/**
 * @brief 测试事件管理器基本功能
 * 验证事件管理器的基本功能
 */
TEST_F(EventChannelTest, BasicFunctionality)
{
    // 验证事件管理器存在
    EXPECT_NE(event, nullptr);
    EXPECT_NE(channelManager, nullptr);
    
    // 验证各个管理器都能正常获取
    EXPECT_NE(event->dispatcher(), nullptr);
    EXPECT_NE(event->sequence(), nullptr);
    EXPECT_NE(event->channel(), nullptr);
}

/**
 * @brief 测试事件类型注册
 * 验证事件类型注册功能
 */
TEST_F(EventChannelTest, EventTypeRegistration)
{
    // 注册测试事件类型
    event->registerEventType(EventStratege::kSlot, "test", "sample");
    
    // 获取事件类型
    EventType type = event->eventType("test", "sample");
    
    // 验证事件类型有效
    EXPECT_NE(type, EventType());
}

/**
 * @brief 测试插件主题查询
 * 验证插件主题查询功能
 */
TEST_F(EventChannelTest, PluginTopics)
{
    // 注册一些测试事件类型
    event->registerEventType(EventStratege::kSlot, "testspace", "topic1");
    event->registerEventType(EventStratege::kSignal, "testspace", "topic2");
    event->registerEventType(EventStratege::kHook, "testspace", "topic3");
    
    // 获取所有主题
    QStringList allTopics = event->pluginTopics("testspace");
    EXPECT_GE(allTopics.size(), 3);
    
    // 获取特定策略的主题
    QStringList slotTopics = event->pluginTopics("testspace", EventStratege::kSlot);
    EXPECT_GE(slotTopics.size(), 1);
    EXPECT_TRUE(slotTopics.contains("topic1"));
}

/**
 * @brief 测试错误处理
 * 验证错误情况的处理
 */
TEST_F(EventChannelTest, ErrorHandling)
{
    // 测试空字符串参数
    EventType invalidType = event->eventType("", "");
    EXPECT_EQ(invalidType, EventType());
    
    // 测试不存在的事件类型
    EventType nonExistentType = event->eventType("nonexistent", "topic");
    EXPECT_EQ(nonExistentType, EventType());
    
    // 测试空主题查询
    QStringList emptyTopics = event->pluginTopics("");
    EXPECT_TRUE(emptyTopics.isEmpty());
} 