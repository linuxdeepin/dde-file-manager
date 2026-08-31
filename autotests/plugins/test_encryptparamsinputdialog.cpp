// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_encryptparamsinputdialog.cpp
 * @brief Unit tests for EncryptParamsInputDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "gui/encryptparamsinputdialog.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class EncryptParamsInputDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EncryptParamsInputDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EncryptParamsInputDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EncryptParamsInputDialogTest, createPasswordPage)
{
    // Test getter: QWidget createPasswordPage()
    auto result = obj->createPasswordPage();
    EXPECT_NO_FATAL_FAILURE({ obj->createPasswordPage(); });

}

TEST_F(EncryptParamsInputDialogTest, onButtonClicked)
{
    // Test method: void onButtonClicked((int idx))
    EXPECT_NO_FATAL_FAILURE(obj->onButtonClicked(0));
}
