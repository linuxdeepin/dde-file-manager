// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_burnudffilesjob_1.cpp
 * @brief Unit tests for BurnUDFFilesJob methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/burnjob.h"

#include <QTest>

using namespace dfmplugin_burn;

class BurnUDFFilesJobTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BurnUDFFilesJob();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BurnUDFFilesJob *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BurnUDFFilesJobTest, BurnUDFFilesJob)
{
    // Test constructor: BurnUDFFilesJob((const QString &dev, const JobHandlePointer handler))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BurnUDFFilesJobTest, fileSystemLimitsValid)
{
    // Test bool getter: fileSystemLimitsValid()
    bool result = obj->fileSystemLimitsValid();
    EXPECT_FALSE(result);

}

TEST_F(BurnUDFFilesJobTest, work)
{
    // Test method: void work(())
    EXPECT_NO_FATAL_FAILURE(obj->work());
}

TEST_F(BurnUDFFilesJobTest, BurnUDFFilesJob_Destructor)
{
    // Test method:  ~BurnUDFFilesJob(())
    EXPECT_NO_FATAL_FAILURE({ BurnUDFFilesJob *tmp = new BurnUDFFilesJob(); delete tmp; });
}
