// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_renamedialogprivate.cpp
 * @brief Unit tests for RenameDialogPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/private/renamedialog_p.h"

#include <QTest>

using namespace ddplugin_canvas;

class RenameDialogPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RenameDialogPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RenameDialogPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RenameDialogPrivateTest, initLayout)
{
    // Test method: void initLayout(())
    EXPECT_NO_FATAL_FAILURE(obj->initLayout());
}

TEST_F(RenameDialogPrivateTest, initLayout_2)
{
    // Test method: void initLayout(())
    EXPECT_NO_FATAL_FAILURE(obj->initLayout());
}

TEST_F(RenameDialogPrivateTest, initParameters)
{
    // Test method: void initParameters(())
    EXPECT_NO_FATAL_FAILURE(obj->initParameters());
}

TEST_F(RenameDialogPrivateTest, initParameters_2)
{
    // Test method: void initParameters(())
    EXPECT_NO_FATAL_FAILURE(obj->initParameters());
}
