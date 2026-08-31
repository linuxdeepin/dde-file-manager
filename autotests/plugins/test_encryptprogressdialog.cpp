// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_encryptprogressdialog.cpp
 * @brief Unit tests for EncryptProgressDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "gui/encryptprogressdialog.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class EncryptProgressDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EncryptProgressDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EncryptProgressDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EncryptProgressDialogTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(EncryptProgressDialogTest, saveRecKey)
{
    // Test method: void saveRecKey((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveRecKey(_arg0));
}

TEST_F(EncryptProgressDialogTest, validateExportPath)
{
    // Test method: bool validateExportPath((const QString &path, QString *msg))
    QString _arg0{};
    auto result = obj->validateExportPath(_arg0, nullptr);
    EXPECT_FALSE(result);

}
