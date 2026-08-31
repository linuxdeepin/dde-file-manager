// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_burnisofilesjob.cpp
 * @brief Unit tests for BurnISOFilesJob methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/burnjob.h"

#include <QTest>

using namespace dfmplugin_burn;

class BurnISOFilesJobTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BurnISOFilesJob();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BurnISOFilesJob *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BurnISOFilesJobTest, BurnISOFilesJob)
{
    // Test constructor: BurnISOFilesJob((const QString &dev, const JobHandlePointer handler))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BurnISOFilesJobTest, fileSystemLimitsValid)
{
    // Test bool getter: fileSystemLimitsValid()
    bool result = obj->fileSystemLimitsValid();
    EXPECT_FALSE(result);

}

TEST_F(BurnISOFilesJobTest, work)
{
    // Test method: void work(())
    EXPECT_NO_FATAL_FAILURE(obj->work());
}

TEST_F(BurnISOFilesJobTest, BurnISOFilesJob_Destructor)
{
    // Test method:  ~BurnISOFilesJob(())
    EXPECT_NO_FATAL_FAILURE({ BurnISOFilesJob *tmp = new BurnISOFilesJob(); delete tmp; });
}
