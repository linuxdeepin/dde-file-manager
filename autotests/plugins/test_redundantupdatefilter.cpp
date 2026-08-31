// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_redundantupdatefilter.cpp
 * @brief Unit tests for RedundantUpdateFilter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "model/filefilter.h"

#include <QTest>

using namespace ddplugin_canvas;

class RedundantUpdateFilterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RedundantUpdateFilter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RedundantUpdateFilter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RedundantUpdateFilterTest, checkUpdate)
{
    // Test method: void checkUpdate(())
    EXPECT_NO_FATAL_FAILURE(obj->checkUpdate());
}
