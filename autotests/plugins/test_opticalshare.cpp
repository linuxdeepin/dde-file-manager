// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticalshare.cpp
 * @brief Unit tests for OpticalShare methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "opticalshare.h"

#include <QTest>

using namespace opticalshare;

class OpticalShareTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpticalShare();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpticalShare *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalShareTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(OpticalShareTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(OpticalShareTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}
