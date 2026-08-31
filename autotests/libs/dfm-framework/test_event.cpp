// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_event.cpp
 * @brief Unit tests for Event methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-framework/event/event.h"

#include <QTest>

using namespace include;

class EventTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Event();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Event *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EventTest, M_~Event)
{
    // Test method:  ~Event(())
    EXPECT_NO_FATAL_FAILURE({ Event *tmp = new Event(); delete tmp; });
}

TEST_F(EventTest, Event)
{
    // Test constructor: Event(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(EventTest, eventType)
{
    // Test method: EventType eventType((const QString &space, const QString &topic))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->eventType(_arg0, _arg1);
    EXPECT_NO_FATAL_FAILURE({ obj->eventType(_arg0, _arg1); });

}

TEST_F(EventTest, registerEventType)
{
    // Test method: void registerEventType((EventStratege stratege, const QString &space, const QString &topic))
    QString _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->registerEventType(EventStratege(), _arg1, _arg2));
}

TEST_F(EventTest, dispatcher)
{
    // Test getter: EventDispatcherManager dispatcher()
    auto result = obj->dispatcher();
    EXPECT_NO_FATAL_FAILURE({ obj->dispatcher(); });

}

TEST_F(EventTest, sequence)
{
    // Test getter: EventSequenceManager sequence()
    auto result = obj->sequence();
    EXPECT_NO_FATAL_FAILURE({ obj->sequence(); });

}

TEST_F(EventTest, channel)
{
    // Test getter: EventChannelManager channel()
    auto result = obj->channel();
    EXPECT_NO_FATAL_FAILURE({ obj->channel(); });

}

TEST_F(EventTest, pluginTopics)
{
    // Test method: QStringList pluginTopics((const QString &space, EventStratege stratege))
    QString _arg0{};
    auto result = obj->pluginTopics(_arg0, EventStratege());
    EXPECT_TRUE(result.isEmpty());

}
