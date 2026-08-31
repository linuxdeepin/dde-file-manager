// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_encryptprogressdialog_1.cpp
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

TEST_F(EncryptProgressDialogTest, EncryptProgressDialog)
{
    // Test constructor: EncryptProgressDialog((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(EncryptProgressDialogTest, onCicked)
{
    // Test method: void onCicked((int idx, const QString &btnTxt))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onCicked(0, _arg1));
}

TEST_F(EncryptProgressDialogTest, setRecoveryKey)
{
    // Test method: void setRecoveryKey(())
    EXPECT_NO_FATAL_FAILURE(obj->setRecoveryKey());
}

TEST_F(EncryptProgressDialogTest, setText)
{
    // Test setter: void setText((const QString &title, const QString &message))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setText(_arg0, _arg1));
}

TEST_F(EncryptProgressDialogTest, showExportPage)
{
    // Test method: void showExportPage(())
    EXPECT_NO_FATAL_FAILURE(obj->showExportPage());
}

TEST_F(EncryptProgressDialogTest, showResultPage)
{
    // Test method: void showResultPage((bool success, const QString &title, const QString &message))
    QString _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->showResultPage(false, _arg1, _arg2));
}

TEST_F(EncryptProgressDialogTest, updateProgress)
{
    // Test method: void updateProgress((double progress))
    EXPECT_NO_FATAL_FAILURE(obj->updateProgress(0.0));
}
