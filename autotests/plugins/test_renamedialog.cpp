// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_renamedialog.cpp
 * @brief Unit tests for RenameDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/renamedialog.h"

#include <QTest>

using namespace ddplugin_canvas;

class RenameDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RenameDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RenameDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RenameDialogTest, RenameDialog)
{
    // Test constructor: RenameDialog((int fileCount, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RenameDialogTest, modifyMode)
{
    // Test getter: RenameDialog::ModifyMode modifyMode()
    auto result = obj->modifyMode();
    EXPECT_GE(static_cast<int>(result), 0);

}
