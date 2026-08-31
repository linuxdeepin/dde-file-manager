// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_eventsequence.cpp
 * @brief Unit tests for TestEventHandler methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "libs/dfm-framework/test_eventsequence.h"

#include <QTest>

using namespace autotests;

class TestEventHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TestEventHandler();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TestEventHandler *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TestEventHandlerTest, reset)
{
    // Test method: void reset(())
    EXPECT_NO_FATAL_FAILURE(obj->reset());
}

TEST_F(TestEventHandlerTest, handleEvent)
{
    // Test bool getter: handleEvent()
    bool result = obj->handleEvent();
    EXPECT_FALSE(result);

}

TEST_F(TestEventHandlerTest, public)
{
    // Test getter: Q_OBJECT public()
    EXPECT_NO_FATAL_FAILURE({ obj->public(); });
}

TEST_F(TestEventHandlerTest, handlerCalled)
{
    // Test bool getter: handlerCalled()
    bool result = obj->handlerCalled();
    EXPECT_FALSE(result);

}

TEST_F(TestEventHandlerTest, lastEventData)
{
    // Test getter: QString lastEventData()
    auto result = obj->lastEventData();
    EXPECT_TRUE(result.isEmpty());

}
