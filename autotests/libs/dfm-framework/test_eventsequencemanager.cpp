// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_eventsequencemanager.cpp
 * @brief Unit tests for EventSequenceManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-framework/event/eventsequence.h"

#include <QTest>

using namespace include;

class EventSequenceManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EventSequenceManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EventSequenceManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EventSequenceManagerTest, unfollow)
{
    // Test method: bool unfollow((EventType type))
    auto result = obj->unfollow(EventType());
    EXPECT_FALSE(result);

}
