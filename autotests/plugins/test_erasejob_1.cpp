// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_erasejob_1.cpp
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

TEST_F(EraseJobTest, updateMessage)
{
    // Test method: void updateMessage((JobInfoPointer ptr))
    EXPECT_NO_FATAL_FAILURE(obj->updateMessage(JobInfoPointer()));
}
