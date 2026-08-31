// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_putpacketwritingjob.cpp
 * @brief Unit tests for PutPacketWritingJob methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/packetwritingjob.h"

#include <QTest>

using namespace dfmplugin_burn;

class PutPacketWritingJobTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PutPacketWritingJob();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PutPacketWritingJob *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PutPacketWritingJobTest, PutPacketWritingJob)
{
    // Test constructor: PutPacketWritingJob((const QString &device, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(PutPacketWritingJobTest, getPendingUrls)
{
    // Test getter: QList<QUrl> getPendingUrls()
    auto result = obj->getPendingUrls();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PutPacketWritingJobTest, setPendingUrls)
{
    // Test setter: void setPendingUrls((const QList<QUrl> &value))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setPendingUrls(_arg0));
}

TEST_F(PutPacketWritingJobTest, work)
{
    // Test bool getter: work()
    bool result = obj->work();
    EXPECT_FALSE(result);

}
