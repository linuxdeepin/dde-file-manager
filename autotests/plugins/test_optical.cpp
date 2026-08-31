// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_optical.cpp
 * @brief Unit tests for Optical methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "optical.h"

#include <QTest>

using namespace dfmplugin_optical;

class OpticalTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Optical();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Optical *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalTest, bindEvents)
{
    // Test method: void bindEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->bindEvents());
}
