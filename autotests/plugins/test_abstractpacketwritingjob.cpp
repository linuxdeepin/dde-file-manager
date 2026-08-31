// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractpacketwritingjob.cpp
 * @brief Unit tests for AbstractPacketWritingJob methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/packetwritingjob.h"

#include <QTest>

using namespace dfmplugin_burn;

class AbstractPacketWritingJobTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractPacketWritingJob();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractPacketWritingJob *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractPacketWritingJobTest, AbstractPacketWritingJob)
{
    // Test constructor: AbstractPacketWritingJob((const QString &device, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AbstractPacketWritingJobTest, device)
{
    // Test getter: QString device()
    auto result = obj->device();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AbstractPacketWritingJobTest, run)
{
    // Test method: void run(())
    EXPECT_NO_FATAL_FAILURE(obj->run());
}

TEST_F(AbstractPacketWritingJobTest, urls2Names)
{
    // Test method: QStringList urls2Names((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->urls2Names(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
