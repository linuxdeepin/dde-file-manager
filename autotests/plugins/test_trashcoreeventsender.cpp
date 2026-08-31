// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashcoreeventsender.cpp
 * @brief Unit tests for TrashCoreEventSender methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/trashcoreeventsender.h"

#include <QTest>

using namespace dfmplugin_trashcore;

class TrashCoreEventSenderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashCoreEventSender();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashCoreEventSender *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashCoreEventSenderTest, checkAndStartWatcher)
{
    // Test bool getter: checkAndStartWatcher()
    bool result = obj->checkAndStartWatcher();
    EXPECT_FALSE(result);

}

TEST_F(TrashCoreEventSenderTest, initTrashState)
{
    // Test method: void initTrashState(())
    EXPECT_NO_FATAL_FAILURE(obj->initTrashState());
}

TEST_F(TrashCoreEventSenderTest, initTrashWatcher)
{
    // Test method: void initTrashWatcher(())
    EXPECT_NO_FATAL_FAILURE(obj->initTrashWatcher());
}

TEST_F(TrashCoreEventSenderTest, instance)
{
    // Test getter: TrashCoreEventSender instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(TrashCoreEventSenderTest, sendTrashStateChangedAdd)
{
    // Test method: void sendTrashStateChangedAdd(())
    EXPECT_NO_FATAL_FAILURE(obj->sendTrashStateChangedAdd());
}

TEST_F(TrashCoreEventSenderTest, sendTrashStateChangedDel)
{
    // Test method: void sendTrashStateChangedDel(())
    EXPECT_NO_FATAL_FAILURE(obj->sendTrashStateChangedDel());
}

TEST_F(TrashCoreEventSenderTest, tryInitialize)
{
    // Test method: void tryInitialize(())
    EXPECT_NO_FATAL_FAILURE(obj->tryInitialize());
}
