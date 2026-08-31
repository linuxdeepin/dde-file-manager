// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_eventchannel.cpp
 * @brief Unit tests for TestReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "libs/dfm-framework/test_eventchannel.h"

#include <QTest>

using namespace autotests;

class TestReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TestReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TestReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TestReceiverTest, public)
{
    // Test getter: Q_OBJECT public()
    EXPECT_NO_FATAL_FAILURE({ obj->public(); });
}

TEST_F(TestReceiverTest, addOne)
{
    // Test getter: int addOne()
    auto result = obj->addOne();
    EXPECT_EQ(result, 0);

}

TEST_F(TestReceiverTest, addTen)
{
    // Test getter: int addTen()
    auto result = obj->addTen();
    EXPECT_EQ(result, 0);

}

TEST_F(TestReceiverTest, combineStrings)
{
    // Test getter: QString combineStrings()
    auto result = obj->combineStrings();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TestReceiverTest, processVariant)
{
    // Test getter: QVariant processVariant()
    auto result = obj->processVariant();
    EXPECT_FALSE(result.isValid());

}

TEST_F(TestReceiverTest, voidFunction)
{
    // Test method: void voidFunction(())
    EXPECT_NO_FATAL_FAILURE(obj->voidFunction());
}

TEST_F(TestReceiverTest, getCallCount)
{
    // Test getter: int getCallCount()
    auto result = obj->getCallCount();
    EXPECT_EQ(result, 0);

}

TEST_F(TestReceiverTest, lastCallCount)
{
    // Test getter: int lastCallCount()
    auto result = obj->lastCallCount();
    EXPECT_EQ(result, 0);

}
