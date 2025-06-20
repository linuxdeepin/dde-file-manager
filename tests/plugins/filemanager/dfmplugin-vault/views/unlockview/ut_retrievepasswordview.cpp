// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/unlockview/retrievepasswordview.h"
#include "utils/vaultutils.h"
#include "utils/encryption/operatorcenter.h"

#include <gtest/gtest.h>

DPVAULT_USE_NAMESPACE

TEST(UT_RetrievePasswordView, verificationKey_Zero_Exist)
{
    stub_ext::StubExt stub;
    typedef  bool(*FuncType)(const QString &);
    stub.set_lamda(static_cast<FuncType>(&QFile::exists), [] { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&OperatorCenter::verificationRetrievePassword, [] { __DBG_STUB_INVOKE__ return true; });

    RetrievePasswordView view;
    view.savePathTypeComboBox->setCurrentIndex(0);
    view.verificationKey();
    EXPECT_FALSE(view.defaultFilePathEdit->text().isEmpty());
}

TEST(UT_RetrievePasswordView, verificationKey_Zero_NoExist)
{
    stub_ext::StubExt stub;
    typedef  bool(*FuncType)(const QString &);
    stub.set_lamda(static_cast<FuncType>(&QFile::exists), [] { __DBG_STUB_INVOKE__ return false; });
    stub.set_lamda(&OperatorCenter::verificationRetrievePassword, [] { __DBG_STUB_INVOKE__ return false; });

    RetrievePasswordView view;
    view.savePathTypeComboBox->setCurrentIndex(0);
    view.verificationKey();
    EXPECT_TRUE(view.defaultFilePathEdit->text().isEmpty());
}

TEST(UT_RetrievePasswordView, verificationKey_One_Exist)
{
    stub_ext::StubExt stub;
    typedef  bool(*FuncType)(const QString &);
    stub.set_lamda(static_cast<FuncType>(&QFile::exists), [] { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&OperatorCenter::verificationRetrievePassword, [] { __DBG_STUB_INVOKE__ return false; });

    bool enableShow { false };
    RetrievePasswordView view;
    view.savePathTypeComboBox->setCurrentIndex(1);
    QObject::connect(&view, &RetrievePasswordView::sigBtnEnabled, [ &enableShow ](int index, bool enable) { enableShow = enable; });
    view.verificationKey();
    EXPECT_TRUE(enableShow);
}

TEST(UT_RetrievePasswordView, verificationKey_One_NoExist)
{
    stub_ext::StubExt stub;
    typedef  bool(*FuncType)(const QString &);
    stub.set_lamda(static_cast<FuncType>(&QFile::exists), [] { __DBG_STUB_INVOKE__ return false; });
    stub.set_lamda(&OperatorCenter::verificationRetrievePassword, [] { __DBG_STUB_INVOKE__ return false; });

    bool enableShow { true };
    RetrievePasswordView view;
    view.savePathTypeComboBox->setCurrentIndex(1);
    QObject::connect(&view, &RetrievePasswordView::sigBtnEnabled, [ &enableShow ](int index, bool enable) { enableShow = enable; });
    view.verificationKey();
    EXPECT_FALSE(enableShow);
}

TEST(UT_RetrievePasswordView, getUserName)
{
    RetrievePasswordView view;
    EXPECT_FALSE(view.getUserName().isEmpty());
}

TEST(UT_RetrievePasswordView, btnText)
{
    RetrievePasswordView view;
    EXPECT_FALSE(view.btnText().isEmpty());
}

TEST(UT_RetrievePasswordView, titleText)
{
    RetrievePasswordView view;
    EXPECT_FALSE(view.titleText().isEmpty());
}

TEST(UT_RetrievePasswordView, buttonClicked_Zero)
{
    bool isJump { false };
    RetrievePasswordView view;
    QObject::connect(&view, &RetrievePasswordView::signalJump, [ &isJump ] { isJump = true; });
    view.buttonClicked(0, "");
    EXPECT_TRUE(isJump);
}

TEST(UT_RetrievePasswordView, buttonClicked_One)
{
    bool isAuthorization { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultUtils::showAuthorityDialog, [ &isAuthorization ] { __DBG_STUB_INVOKE__ isAuthorization = true; });

    RetrievePasswordView view;
    view.buttonClicked(1, "");
    EXPECT_TRUE(isAuthorization);
}

TEST(UT_RetrievePasswordView, ValidationResults)
{
    RetrievePasswordView view;
    view.validationResults = "123";
    EXPECT_TRUE(view.ValidationResults() == "123");
}

TEST(UT_RetrievePasswordView, onComboBoxIndex_Zero_Exits)
{
    stub_ext::StubExt stub;
    typedef  bool(*FuncType)(const QString &);
    stub.set_lamda(static_cast<FuncType>(&QFile::exists), [] { __DBG_STUB_INVOKE__ return true; });

    RetrievePasswordView view;
    view.onComboBoxIndex(0);
    EXPECT_FALSE(view.defaultFilePathEdit->text().isEmpty());
}

TEST(UT_RetrievePasswordView, onComboBoxIndex_Zero_NoExits)
{
    stub_ext::StubExt stub;
    typedef bool(*FuncType)(const QString &);
    stub.set_lamda(static_cast<FuncType>(&QFile::exists), [] { __DBG_STUB_INVOKE__ return false; });

    RetrievePasswordView view;
    view.onComboBoxIndex(0);
    EXPECT_TRUE(view.defaultFilePathEdit->text().isEmpty());
}

TEST(UT_RetrievePasswordView, onComboBoxIndex_One_Exits)
{
    stub_ext::StubExt stub;
    typedef  bool(*FuncType)(const QString &);
    stub.set_lamda(static_cast<FuncType>(&QFile::exists), [] { __DBG_STUB_INVOKE__ return true; });

    RetrievePasswordView view;
    view.onComboBoxIndex(1);
    EXPECT_TRUE(view.verificationPrompt->text().isEmpty());
}

TEST(UT_RetrievePasswordView, onComboBoxIndex_One_NoExits)
{
    stub_ext::StubExt stub;
    typedef  bool(*FuncType)(const QString &);
    stub.set_lamda(static_cast<FuncType>(&QFile::exists), [] { __DBG_STUB_INVOKE__ return false; });

    RetrievePasswordView view;
    view.onComboBoxIndex(1);
    EXPECT_TRUE(view.verificationPrompt->text().isEmpty());
}

TEST(UT_RetrievePasswordView, onComboBoxIndex_One_NoExits2)
{
    stub_ext::StubExt stub;
    typedef  bool(*FuncType)(const QString &);
    stub.set_lamda(static_cast<FuncType>(&QFile::exists), [] { __DBG_STUB_INVOKE__ return false; });

    RetrievePasswordView view;
    view.filePathEdit->setText("123");
    view.filePathEdit->lineEdit()->setPlaceholderText("");
    view.onComboBoxIndex(1);
    EXPECT_TRUE(view.filePathEdit->text().isEmpty());
}
