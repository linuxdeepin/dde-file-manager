// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_removepacketwritingjob_1.cpp
 * @brief Unit tests for RemovePacketWritingJob methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/packetwritingjob.h"

#include <QTest>

using namespace dfmplugin_burn;

class RemovePacketWritingJobTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RemovePacketWritingJob();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RemovePacketWritingJob *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RemovePacketWritingJobTest, getPendingUrls)
{
    // Test getter: QList<QUrl> getPendingUrls()
    auto result = obj->getPendingUrls();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RemovePacketWritingJobTest, setPendingUrls)
{
    // Test setter: void setPendingUrls((const QList<QUrl> &value))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setPendingUrls(_arg0));
}

TEST_F(RemovePacketWritingJobTest, work)
{
    // Test bool getter: work()
    bool result = obj->work();
    EXPECT_FALSE(result);

}
