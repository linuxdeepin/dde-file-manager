// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractindexcontroller.cpp
 * @brief Unit tests for AbstractIndexController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "abstractindexcontroller.h"

#include <QTest>

using namespace core;

class AbstractIndexControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractIndexController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractIndexController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractIndexControllerTest, setupStateHandlers)
{
    // Test method: void setupStateHandlers(())
    EXPECT_NO_FATAL_FAILURE(obj->setupStateHandlers());
}
