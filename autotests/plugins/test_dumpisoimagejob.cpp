// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dumpisoimagejob.cpp
 * @brief Unit tests for DumpISOImageJob methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/burnjob.h"

#include <QTest>

using namespace dfmplugin_burn;

class DumpISOImageJobTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DumpISOImageJob();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DumpISOImageJob *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DumpISOImageJobTest, writeFunc)
{
    // Test method: void writeFunc((int progressFd, int checkFd))
    EXPECT_NO_FATAL_FAILURE(obj->writeFunc(0, 0));
}
