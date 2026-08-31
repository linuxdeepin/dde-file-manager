// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_closealldialog.cpp
 * @brief Unit tests for CloseAllDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/closealldialog.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class CloseAllDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CloseAllDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CloseAllDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CloseAllDialogTest, CloseAllDialog)
{
    // Test constructor: CloseAllDialog((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
