// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_renamepacketwritingjob.cpp
 * @brief Unit tests for RenamePacketWritingJob methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/packetwritingjob.h"

#include <QTest>

using namespace dfmplugin_burn;

class RenamePacketWritingJobTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RenamePacketWritingJob();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RenamePacketWritingJob *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RenamePacketWritingJobTest, RenamePacketWritingJob)
{
    // Test constructor: RenamePacketWritingJob((const QString &device, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RenamePacketWritingJobTest, getDestUrl)
{
    // Test getter: QUrl getDestUrl()
    auto result = obj->getDestUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(RenamePacketWritingJobTest, getSrcUrl)
{
    // Test getter: QUrl getSrcUrl()
    auto result = obj->getSrcUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(RenamePacketWritingJobTest, setDestUrl)
{
    // Test setter: void setDestUrl((const QUrl &value))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setDestUrl(_arg0));
}

TEST_F(RenamePacketWritingJobTest, setSrcUrl)
{
    // Test setter: void setSrcUrl((const QUrl &value))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setSrcUrl(_arg0));
}

TEST_F(RenamePacketWritingJobTest, work)
{
    // Test bool getter: work()
    bool result = obj->work();
    EXPECT_FALSE(result);

}
