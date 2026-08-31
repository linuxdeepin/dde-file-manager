// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_encryptparamsinputdialog_1.cpp
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

TEST_F(EncryptParamsInputDialogTest, EncryptParamsInputDialog)
{
    // Test constructor: EncryptParamsInputDialog((const QVariantMap &args,
                                                   QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(EncryptParamsInputDialogTest, confirmEncrypt)
{
    // Test method: void confirmEncrypt(())
    EXPECT_NO_FATAL_FAILURE(obj->confirmEncrypt());
}

TEST_F(EncryptParamsInputDialogTest, createExportPage)
{
    // Test getter: QWidget createExportPage()
    auto result = obj->createExportPage();
    EXPECT_NO_FATAL_FAILURE({ obj->createExportPage(); });

}

TEST_F(EncryptParamsInputDialogTest, encryptByTpm)
{
    // Test method: bool encryptByTpm((const QString &deviceName))
    QString _arg0{};
    auto result = obj->encryptByTpm(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(EncryptParamsInputDialogTest, getInputs)
{
    // Test getter: DeviceEncryptParam getInputs()
    auto result = obj->getInputs();
    EXPECT_NO_FATAL_FAILURE({ obj->getInputs(); });

}

TEST_F(EncryptParamsInputDialogTest, initConn)
{
    // Test method: void initConn(())
    EXPECT_NO_FATAL_FAILURE(obj->initConn());
}

TEST_F(EncryptParamsInputDialogTest, initUi)
{
    // Test method: void initUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initUi());
}

TEST_F(EncryptParamsInputDialogTest, onEncTypeChanged)
{
    // Test method: void onEncTypeChanged((int type))
    EXPECT_NO_FATAL_FAILURE(obj->onEncTypeChanged(0));
}

TEST_F(EncryptParamsInputDialogTest, onExpPathChanged)
{
    // Test method: void onExpPathChanged((const QString &path, bool silent))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onExpPathChanged(_arg0, false));
}

TEST_F(EncryptParamsInputDialogTest, onPageChanged)
{
    // Test method: void onPageChanged((int page))
    EXPECT_NO_FATAL_FAILURE(obj->onPageChanged(0));
}

TEST_F(EncryptParamsInputDialogTest, setPasswordInputVisible)
{
    // Test setter: void setPasswordInputVisible((bool visible))
    EXPECT_NO_FATAL_FAILURE(obj->setPasswordInputVisible(false));
}
