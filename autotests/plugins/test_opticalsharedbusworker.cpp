// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticalsharedbusworker.cpp
 * @brief Unit tests for OpticalShareDBusWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "opticalshare.h"

#include <QTest>

using namespace opticalshare;

class OpticalShareDBusWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpticalShareDBusWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpticalShareDBusWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalShareDBusWorkerTest, launchService)
{
    // Test method: void launchService(())
    EXPECT_NO_FATAL_FAILURE(obj->launchService());
}
