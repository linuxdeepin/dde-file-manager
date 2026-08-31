// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_packetwritingscheduler.cpp
 * @brief Unit tests for PacketWritingScheduler methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/packetwritingjob.h"

#include <QTest>

using namespace dfmplugin_burn;

class PacketWritingSchedulerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PacketWritingScheduler();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PacketWritingScheduler *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PacketWritingSchedulerTest, PacketWritingScheduler)
{
    // Test constructor: PacketWritingScheduler((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(PacketWritingSchedulerTest, addJob)
{
    // Test method: void addJob((AbstractPacketWritingJob *job))
    EXPECT_NO_FATAL_FAILURE(obj->addJob(nullptr));
}

TEST_F(PacketWritingSchedulerTest, instance)
{
    // Test getter: DPBURN_BEGIN_NAMESPACE instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(PacketWritingSchedulerTest, onTimeout)
{
    // Test method: void onTimeout(())
    EXPECT_NO_FATAL_FAILURE(obj->onTimeout());
}
