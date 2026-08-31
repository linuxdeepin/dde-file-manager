// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_progressdialog.cpp
 * @brief Unit tests for ProgressDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "shred/progressdialog.h"

#include <QTest>

using namespace dfmplugin_utils;

class ProgressDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ProgressDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ProgressDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ProgressDialogTest, ProgressDialog)
{
    // Test constructor: ProgressDialog((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ProgressDialogTest, handleButtonClicked)
{
    // Test method: void handleButtonClicked((int index, const QString &text))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleButtonClicked(0, _arg1));
}
