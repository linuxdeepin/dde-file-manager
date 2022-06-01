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
#include "event/dispatcher/eventdispatcher.h"
#include "testqobject.h"

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>

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

TEST_F(UT_EventDispatcher, test_install_filter)
{
    TestQObject b;
    EventType eType1 = 1;

    EXPECT_FALSE(EventDispatcherManager::instance().removeEventFilter(eType1));
    EventDispatcherManager::instance().subscribe(eType1, &b, &TestQObject::test1);
    EXPECT_TRUE(EventDispatcherManager::instance().removeEventFilter(eType1));
    EXPECT_TRUE(EventDispatcherManager::instance().installEventFilter(eType1,
                                                                      [](EventDispatcher::Listener,
                                                                         const QVariantList &) {
                                                                          return true;
                                                                      }));
    EXPECT_FALSE(EventDispatcherManager::instance().installEventFilter(eType1,
                                                                       [](EventDispatcher::Listener,
                                                                          const QVariantList &) {
                                                                           return true;
                                                                       }));
    EXPECT_TRUE(EventDispatcherManager::instance().removeEventFilter(eType1));
    EXPECT_TRUE(EventDispatcherManager::instance().installEventFilter(eType1,
                                                                      [](EventDispatcher::Listener,
                                                                         const QVariantList &) {
                                                                          return true;
                                                                      }));
    EXPECT_TRUE(EventDispatcherManager::instance().removeEventFilter(eType1));
}

TEST_F(UT_EventDispatcher, test_use_filter)
{
    TestQObject b;
    EventType eType1 = 1;

    int called { 0 };
    EventDispatcherManager::instance().subscribe(eType1, &b, &TestQObject::bigger10);
    EventDispatcherManager::instance().publish(eType1, 0, &called);
    EXPECT_EQ(10, called);

    EventDispatcherManager::instance().subscribe(eType1, &b, &TestQObject::bigger15);
    EventDispatcherManager::instance().publish(eType1, 0, &called);
    EXPECT_EQ(15, called);

    EXPECT_TRUE(EventDispatcherManager::instance().installEventFilter(eType1,
                                                                      [](EventDispatcher::Listener listener,
                                                                         const QVariantList &list) {
                                                                          if (listener(list).toBool())
                                                                              return true;
                                                                          return false;
                                                                      }));
    EventDispatcherManager::instance().publish(eType1, 2, &called);
    EXPECT_EQ(15, called);

    called = 1;
    EventDispatcherManager::instance().publish(eType1, 9, &called);
    EXPECT_EQ(15, called);

    EventDispatcherManager::instance().publish(eType1, 11, &called);
    EXPECT_EQ(10, called);
}

TEST_F(UT_EventDispatcher, test_empty_push)
{
    TestQObject b;
    EventType eType1 = 2;
    EXPECT_TRUE(EventDispatcherManager::instance().subscribe(eType1, &b, &TestQObject::empty1));
    EXPECT_TRUE(EventDispatcherManager::instance().publish(eType1));
}
