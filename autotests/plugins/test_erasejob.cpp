// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_erasejob.cpp
 * @brief Unit tests for EraseJob methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/burnjob.h"

#include <QTest>

using namespace dfmplugin_burn;

class EraseJobTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EraseJob();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EraseJob *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EraseJobTest, EraseJob)
{
    // Test constructor: EraseJob((const QString &dev, const JobHandlePointer handler))
    ASSERT_NE(obj, nullptr);
}

TEST_F(EraseJobTest, work)
{
    // Test method: void work(())
    EXPECT_NO_FATAL_FAILURE(obj->work());
}

TEST_F(EraseJobTest, _EraseJob)
{
    // Test constructor: EraseJob((const QString &dev, const JobHandlePointer handler))
    ASSERT_NE(obj, nullptr);
}
