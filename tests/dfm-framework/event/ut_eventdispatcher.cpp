// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testqobject.h"

#include <dfm-framework/dpf.h>
#include <dfm-framework/event/event.h>

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
