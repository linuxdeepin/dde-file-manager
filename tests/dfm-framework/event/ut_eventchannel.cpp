// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testqobject.h"

#include <dfm-framework/dpf.h>
#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>

DPF_USE_NAMESPACE

class UT_EventChannel : public testing::Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }
};

TEST_F(UT_EventChannel, test_sync_send)
{
    TestQObject b;
    EventChannel channel;
    channel.setReceiver(&b, &TestQObject::test1);
    QVariant value = channel.send(8);
    EXPECT_EQ(value.toInt(), 18);
}

TEST_F(UT_EventChannel, test_async_send)
{
    TestQObject b;
    EventChannel Channel;
    Channel.setReceiver(&b, &TestQObject::test1);
    EventChannelFuture future = Channel.asyncSend(8);
    future.waitForFinished();
    QVariant value = future.result();
    EXPECT_EQ(value.toInt(), 18);
}

TEST_F(UT_EventChannel, test_manager_push)
{
    TestQObject b;
    dpfSlotChannel->connect(12345, &b, &TestQObject::test1);
    QVariant value = dpfSlotChannel->push(12345, 10);
    EXPECT_EQ(value.toInt(), 20);
}

TEST_F(UT_EventChannel, test_manager_post)
{
    TestQObject b;
    dpfSlotChannel->connect(12345, &b, &TestQObject::test1);
    EventChannelFuture future = dpfSlotChannel->post(12345, 10);
    future.waitForFinished();
    QVariant value = future.result();
    EXPECT_EQ(value.toInt(), 20);
}
