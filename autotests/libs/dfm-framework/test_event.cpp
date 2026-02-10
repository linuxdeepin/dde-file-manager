// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>

using namespace dpf;

class EventTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup, if needed
    }

    void TearDown() override {
        // Test cleanup, if needed
    }
};

TEST_F(EventTest, Singleton)
{
    auto *instance1 = Event::instance();
    auto *instance2 = Event::instance();
    EXPECT_EQ(instance1, instance2);
}

TEST_F(EventTest, RegisterAndRetrieveEventType)
{
    auto *event = Event::instance();
    const QString space = "test_space";
    const QString signalTopic = "signal_test_topic";
    const QString slotTopic = "slot_test_topic";
    const QString hookTopic = "hook_test_topic";

    event->registerEventType(EventStratege::kSignal, space, signalTopic);
    event->registerEventType(EventStratege::kSlot, space, slotTopic);
    event->registerEventType(EventStratege::kHook, space, hookTopic);
    
    // Test retrieving existing events
    EventType signalEventType = event->eventType(space, signalTopic);
    EXPECT_NE(signalEventType, EventTypeScope::kInValid);

    EventType slotEventType = event->eventType(space, slotTopic);
    EXPECT_NE(slotEventType, EventTypeScope::kInValid);

    EventType hookEventType = event->eventType(space, hookTopic);
    EXPECT_NE(hookEventType, EventTypeScope::kInValid);
    
    // Test retrieving a non-existent event
    EventType invalidEventType = event->eventType(space, "non_existent_topic");
    EXPECT_EQ(invalidEventType, EventTypeScope::kInValid);
}

TEST_F(EventTest, RegisterRepeatEvent)
{
    auto *event = Event::instance();
    const QString space = "test_space";
    const QString topic = "signal_repeat_topic";

    event->registerEventType(EventStratege::kSignal, space, topic);
    EventType eventType1 = event->eventType(space, topic);

    // Registering the same event again should be handled gracefully
    event->registerEventType(EventStratege::kSignal, space, topic);
    EventType eventType2 = event->eventType(space, topic);

    EXPECT_EQ(eventType1, eventType2);
}

TEST_F(EventTest, PluginTopics)
{
    auto *event = Event::instance();
    const QString space1 = "plugin1";
    const QString space2 = "plugin2";

    event->registerEventType(EventStratege::kSignal, space1, "signal_topic1");
    event->registerEventType(EventStratege::kSlot, space1, "slot_topic2");
    event->registerEventType(EventStratege::kHook, space2, "hook_topic3");

    QStringList topics1 = event->pluginTopics(space1);
    EXPECT_EQ(topics1.size(), 2);
    EXPECT_TRUE(topics1.contains("signal_topic1"));
    EXPECT_TRUE(topics1.contains("slot_topic2"));

    QStringList topics2 = event->pluginTopics(space2);
    EXPECT_EQ(topics2.size(), 1);
    EXPECT_TRUE(topics2.contains("hook_topic3"));

    QStringList topicsSignal1 = event->pluginTopics(space1, EventStratege::kSignal);
    EXPECT_EQ(topicsSignal1.size(), 1);
    EXPECT_TRUE(topicsSignal1.contains("signal_topic1"));
}
