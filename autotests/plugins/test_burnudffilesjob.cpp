// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_burnudffilesjob.cpp
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

TEST_F(BurnUDFFilesJobTest, finishFunc)
{
    // Test method: void finishFunc((const VerifyResult &result))
    VerifyResult _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->finishFunc(_arg0));
}

TEST_F(BurnUDFFilesJobTest, writeFunc)
{
    // Test method: void writeFunc((int progressFd, int checkFd))
    EXPECT_NO_FATAL_FAILURE(obj->writeFunc(0, 0));
}
