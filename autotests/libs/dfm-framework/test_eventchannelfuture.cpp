// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_eventchannelfuture.cpp
 * @brief Unit tests for EventChannelFuture methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-framework/event/eventchannel.h"

#include <QTest>

using namespace src;

class EventChannelFutureTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EventChannelFuture();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EventChannelFuture *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EventChannelFutureTest, cancel)
{
    // Test method: void cancel(())
    EXPECT_NO_FATAL_FAILURE(obj->cancel());
}

TEST_F(EventChannelFutureTest, isCanceled)
{
    // Test bool getter: isCanceled()
    bool result = obj->isCanceled();
    EXPECT_FALSE(result);

}

TEST_F(EventChannelFutureTest, isFinished)
{
    // Test bool getter: isFinished()
    bool result = obj->isFinished();
    EXPECT_FALSE(result);

}

TEST_F(EventChannelFutureTest, isRunning)
{
    // Test bool getter: isRunning()
    bool result = obj->isRunning();
    EXPECT_FALSE(result);

}

TEST_F(EventChannelFutureTest, isStarted)
{
    // Test bool getter: isStarted()
    bool result = obj->isStarted();
    EXPECT_FALSE(result);

}

TEST_F(EventChannelFutureTest, result)
{
    // Test getter: QVariant result()
    auto result = obj->result();
    EXPECT_FALSE(result.isValid());

}

TEST_F(EventChannelFutureTest, waitForFinished)
{
    // Test method: void waitForFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->waitForFinished());
}
