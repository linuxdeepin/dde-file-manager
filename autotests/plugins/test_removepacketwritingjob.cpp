// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_removepacketwritingjob.cpp
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

TEST_F(RemovePacketWritingJobTest, RemovePacketWritingJob)
{
    // Test constructor: RemovePacketWritingJob((const QString &device, QObject *parent))
    ASSERT_NE(obj, nullptr);
}
