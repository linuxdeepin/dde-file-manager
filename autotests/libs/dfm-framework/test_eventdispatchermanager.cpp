// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_eventdispatchermanager.cpp
 * @brief Unit tests for EventDispatcherManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-framework/event/eventdispatcher.h"

#include <QTest>

using namespace include;

class EventDispatcherManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EventDispatcherManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EventDispatcherManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EventDispatcherManagerTest, removeGlobalEventFilter)
{
    // Test method: bool removeGlobalEventFilter((QObject *obj))
    auto result = obj->removeGlobalEventFilter(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(EventDispatcherManagerTest, unsubscribe)
{
    // Test method: bool unsubscribe((EventType type))
    auto result = obj->unsubscribe(EventType());
    EXPECT_FALSE(result);

}
