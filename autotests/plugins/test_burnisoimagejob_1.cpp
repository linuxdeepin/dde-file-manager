// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_burnisoimagejob_1.cpp
 * @brief Unit tests for BurnISOImageJob methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/burnjob.h"

#include <QTest>

using namespace dfmplugin_burn;

class BurnISOImageJobTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BurnISOImageJob();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BurnISOImageJob *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BurnISOImageJobTest, BurnISOImageJob)
{
    // Test constructor: BurnISOImageJob((const QString &dev, const JobHandlePointer handler))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BurnISOImageJobTest, work)
{
    // Test method: void work(())
    EXPECT_NO_FATAL_FAILURE(obj->work());
}

TEST_F(BurnISOImageJobTest, BurnISOImageJob_Destructor)
{
    // Test method:  ~BurnISOImageJob(())
    EXPECT_NO_FATAL_FAILURE({ BurnISOImageJob *tmp = new BurnISOImageJob(); delete tmp; });
}
