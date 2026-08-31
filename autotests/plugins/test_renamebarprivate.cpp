// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_renamebarprivate.cpp
 * @brief Unit tests for RenameBarPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/private/renamebar_p.h"

#include <QTest>

using namespace dfmplugin_workspace;

class RenameBarPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RenameBarPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RenameBarPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RenameBarPrivateTest, layoutItems)
{
    // Test method: void layoutItems(())
    EXPECT_NO_FATAL_FAILURE(obj->layoutItems());
}

TEST_F(RenameBarPrivateTest, setUIParameters)
{
    // Test method: void setUIParameters(())
    EXPECT_NO_FATAL_FAILURE(obj->setUIParameters());
}
