/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "event/channel/eventchannel.h"
#include "testqobject.h"
#include "framework.h"

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
    EventChannel Channel;
    Channel.setReceiver(&b, &TestQObject::test1);
    QVariant value = Channel.send(8);
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
    EventChannelManager &manager = dpfInstance.eventChannel();
    manager.connect(123456, &b, &TestQObject::test1);
    QVariant value = manager.push(123456, 10);
    EXPECT_EQ(value.toInt(), 20);
}

TEST_F(UT_EventChannel, test_manager_post)
{
    TestQObject b;
    EventChannelManager &manager = dpfInstance.eventChannel();
    manager.connect(123456, &b, &TestQObject::test1);
    EventChannelFuture future = manager.post(123456, 10);
    future.waitForFinished();
    QVariant value = future.result();
    EXPECT_EQ(value.toInt(), 20);
}
