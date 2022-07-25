/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "testqobject.h"
#include "event/dispatcher/eventdispatcher.h"

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>

#define protected public

DPF_USE_NAMESPACE

class UT_EventDispatcher : public testing::Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }
};

TEST_F(UT_EventDispatcher, test_install_global_filter)
{
    TestQObject b;
    // TODO(zhangs)
}

TEST_F(UT_EventDispatcher, test_use_global_filter)
{
    TestQObject b;
    // TODO(zhangs)
}

TEST_F(UT_EventDispatcher, test_empty_push)
{
    TestQObject b;
    EventType eType1 = 2;
    EXPECT_TRUE(dpfSignalDispatcher->subscribe(eType1, &b, &TestQObject::empty1));
    EXPECT_TRUE(dpfSignalDispatcher->publish(eType1));
    EXPECT_TRUE(dpfSignalDispatcher->unsubscribe(eType1));
}

TEST_F(UT_EventDispatcher, test_unsub)
{
    TestQObject b;
    EventType eType1 = 2;
    int v = 0;
    EXPECT_TRUE(dpfSignalDispatcher->subscribe(eType1, &b, &TestQObject::add1));
    EXPECT_TRUE(dpfSignalDispatcher->publish(eType1, &v));
    EXPECT_EQ(v, 1);
    EXPECT_TRUE(dpfSignalDispatcher->publish(eType1, &v));
    EXPECT_EQ(v, 2);

    EXPECT_TRUE(dpfSignalDispatcher->unsubscribe(eType1, &b, &TestQObject::add1));
    EXPECT_TRUE(dpfSignalDispatcher->publish(eType1, &v));
    EXPECT_EQ(v, 2);

    EXPECT_TRUE(dpfSignalDispatcher->subscribe(eType1, &b, &TestQObject::add1));
    EXPECT_TRUE(dpfSignalDispatcher->publish(eType1, &v));
    EXPECT_EQ(v, 3);

    EXPECT_TRUE(dpfSignalDispatcher->unsubscribe(eType1));
}
