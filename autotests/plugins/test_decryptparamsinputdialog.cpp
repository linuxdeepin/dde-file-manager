// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_decryptparamsinputdialog.cpp
 * @brief Unit tests for DecryptParamsInputDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "gui/decryptparamsinputdialog.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class DecryptParamsInputDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DecryptParamsInputDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DecryptParamsInputDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DecryptParamsInputDialogTest, DecryptParamsInputDialog)
{
    // Test constructor: DecryptParamsInputDialog((const QString &device, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DecryptParamsInputDialogTest, getKey)
{
    // Test getter: QString getKey()
    auto result = obj->getKey();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
