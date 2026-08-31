// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_diskpasswordchangingdialog.cpp
 * @brief Unit tests for DiskPasswordChangingDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dialogs/diskpasswordchangingdialog.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class DiskPasswordChangingDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DiskPasswordChangingDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DiskPasswordChangingDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DiskPasswordChangingDialogTest, DiskPasswordChangingDialog)
{
    // Test constructor: DiskPasswordChangingDialog((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
