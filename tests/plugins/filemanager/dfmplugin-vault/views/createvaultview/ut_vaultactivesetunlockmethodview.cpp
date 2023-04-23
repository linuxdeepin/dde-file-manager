// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/createvaultview/vaultactivesetunlockmethodview.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/encryption/vaultconfig.h"
#include "utils/policy/policymanager.h"

#include <gtest/gtest.h>

#include <DPasswordEdit>
#include <DComboBox>
#include <DLabel>

#include <QShowEvent>

DPVAULT_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class UT_VaultActiveSetUnlockMethodView : public testing::Test
{
public:
    void SetUp() override
    {
        stub.set_lamda(&OperatorCenter::createDirAndFile, [] { __DBG_STUB_INVOKE__ return true; });
        view = new VaultActiveSetUnlockMethodView();
    }

    void TearDown() override
    {
        if (view) {
            delete view;
            view = nullptr;
        }
        stub.clear();
    }

    VaultActiveSetUnlockMethodView *view { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_VaultActiveSetUnlockMethodView, clearText)
{
    view->clearText();

    EXPECT_TRUE(view->passwordEdit->lineEdit()->text().isEmpty());
    EXPECT_TRUE(view->repeatPasswordEdit->lineEdit()->text().isEmpty());
    EXPECT_TRUE(view->tipsEdit->lineEdit()->text().isEmpty());
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotPasswordEditing)
{
    view->passwordEdit->setText("Qwer@1234");
    view->repeatPasswordEdit->setText("Qwer@1234");
    view->slotPasswordEditing();
    EXPECT_FALSE(view->nextBtn->isEnabled());
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotPasswordEditFinished_True)
{
    view->passwordEdit->setText("Qwer@1234");
    view->repeatPasswordEdit->setText("Qwer@1234");
    view->slotPasswordEditFinished();
    EXPECT_TRUE(view->nextBtn->isEnabled());
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotPasswordEditFinished_False)
{
    stub.set_lamda(&DPasswordEdit::setAlert, [] { __DBG_STUB_INVOKE__ });
    typedef void (DPasswordEdit::*FunType1)(const QString &, int);
    stub.set_lamda(static_cast<FunType1>(&DPasswordEdit::showAlertMessage), [] { __DBG_STUB_INVOKE__ });
    view->passwordEdit->setText("123456");
    view->slotPasswordEditFinished();
    EXPECT_FALSE(view->nextBtn->isEnabled());
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotPasswordEditFocusChanged)
{
    stub.set_lamda(&DPasswordEdit::setAlert, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&DPasswordEdit::hideAlertMessage, [] { __DBG_STUB_INVOKE__ });
    view->slotPasswordEditFocusChanged(true);
    EXPECT_FALSE(view->passwordEdit->isAlert());
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotRepeatPasswordEditFinished)
{
    bool isSetAlert { false };
    stub.set_lamda(&DPasswordEdit::setAlert, [ &isSetAlert ] { __DBG_STUB_INVOKE__ isSetAlert = true; });
    typedef void (DPasswordEdit::*FunType1)(const QString &, int);
    stub.set_lamda(static_cast<FunType1>(&DPasswordEdit::showAlertMessage), [] { __DBG_STUB_INVOKE__ });

    view->passwordEdit->setText("Qwer@5678");
    view->repeatPasswordEdit->setText("Qwer@1234");
    view->slotRepeatPasswordEditFinished();
    EXPECT_TRUE(isSetAlert);
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotRepeatPasswordEditing_True)
{
    view->passwordEdit->setText("Qwer@1234");
    view->repeatPasswordEdit->setText("Qwer@1234");
    view->slotRepeatPasswordEditing();
    EXPECT_TRUE(view->nextBtn->isEnabled());
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotRepeatPasswordEditing_False)
{
    typedef void (DPasswordEdit::*FunType1)(const QString &, int);
    stub.set_lamda(static_cast<FunType1>(&DPasswordEdit::showAlertMessage), [] { __DBG_STUB_INVOKE__ });

    view->passwordEdit->setText("Qwer@5678");
    view->repeatPasswordEdit->setText("Qwer@1234");
    view->slotRepeatPasswordEditing();
    EXPECT_FALSE(view->nextBtn->isEnabled());
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotRepeatPasswordEditFocusChanged)
{
    stub.set_lamda(&DPasswordEdit::setAlert, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&DPasswordEdit::hideAlertMessage, [] { __DBG_STUB_INVOKE__ });
    view->slotRepeatPasswordEditFocusChanged(true);
    EXPECT_FALSE(view->repeatPasswordEdit->isAlert());
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotGenerateEditChanged_True)
{
    view->slotGenerateEditChanged("Qwer@1234");
    EXPECT_TRUE(view->nextBtn->isEnabled());
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotGenerateEditChanged_False)
{
    view->slotGenerateEditChanged("123456");
    EXPECT_FALSE(view->nextBtn->isEnabled());
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotNextBtnClicked_keyEncrytion)
{
    bool isSavePassword { false };
    stub.set_lamda(&VaultConfig::set, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&OperatorCenter::savePasswordAndPasswordHint, [ &isSavePassword ] { __DBG_STUB_INVOKE__ isSavePassword = true; return true; });
    stub.set_lamda(&OperatorCenter::createKeyNew, [] { __DBG_STUB_INVOKE__ return true; });

    view->typeCombo->setCurrentIndex(0);
    view->slotNextBtnClicked();
    EXPECT_TRUE(isSavePassword);
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotNextBtnClicked_transparentEncrytion1)
{
    bool isGeneratePassword { false };
    stub.set_lamda(&VaultConfig::set, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&OperatorCenter::autoGeneratePassword, [ &isGeneratePassword ] { __DBG_STUB_INVOKE__ isGeneratePassword = true; return ""; });

    view->typeCombo->setCurrentIndex(1);
    view->slotNextBtnClicked();
    EXPECT_TRUE(isGeneratePassword);
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotNextBtnClicked_transparentEncrytion2)
{
    bool isSavePsToKeyring { false };
    stub.set_lamda(&VaultConfig::set, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&OperatorCenter::autoGeneratePassword, [] { __DBG_STUB_INVOKE__ return "123456"; });
    stub.set_lamda(&OperatorCenter::savePasswordToKeyring, [ &isSavePsToKeyring ] { __DBG_STUB_INVOKE__ isSavePsToKeyring = true; return true; });

    view->typeCombo->setCurrentIndex(1);
    view->slotNextBtnClicked();
    EXPECT_TRUE(isSavePsToKeyring);
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotNextBtnClicked_transparentEncrytion3)
{
    bool isSavePsToKeyring { true };
    stub.set_lamda(&VaultConfig::set, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&OperatorCenter::autoGeneratePassword, [] { __DBG_STUB_INVOKE__ return "123456"; });
    stub.set_lamda(&OperatorCenter::savePasswordToKeyring, [ &isSavePsToKeyring ] { __DBG_STUB_INVOKE__ isSavePsToKeyring = false; return false; });

    view->typeCombo->setCurrentIndex(1);
    view->slotNextBtnClicked();
    EXPECT_FALSE(isSavePsToKeyring);
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotTypeChanged_transparentEncryption)
{
    view->slotTypeChanged(1);
    EXPECT_TRUE(view->nextBtn->isEnabled());
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotTypeChanged_keyEncryption_True)
{
    typedef void (DPasswordEdit::*FunType1)(const QString &, int);
    stub.set_lamda(static_cast<FunType1>(&DPasswordEdit::showAlertMessage), [] { __DBG_STUB_INVOKE__ });

    view->passwordEdit->setText("Qwer@1234");
    view->repeatPasswordEdit->setText("Qwer@1234");
    view->slotTypeChanged(0);
    EXPECT_TRUE(view->nextBtn->isEnabled());
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotTypeChanged_keyEncryption_False)
{
    typedef void (DPasswordEdit::*FunType1)(const QString &, int);
    stub.set_lamda(static_cast<FunType1>(&DPasswordEdit::showAlertMessage), [] { __DBG_STUB_INVOKE__ });

    view->passwordEdit->setText("Qwer@5678");
    view->repeatPasswordEdit->setText("Qwer@1234");
    view->slotTypeChanged(0);
    EXPECT_FALSE(view->nextBtn->isEnabled());
}

TEST_F(UT_VaultActiveSetUnlockMethodView, slotLimiPasswordLength)
{
    emit view->passwordEdit->textEdited("123456789012345678901234567890");
    EXPECT_TRUE(view->passwordEdit->text().size() == PASSWORD_LENGHT_MAX);
}

TEST_F(UT_VaultActiveSetUnlockMethodView, showEvent)
{
    bool isSetVaule { false };
    stub.set_lamda(&PolicyManager::setVauleCurrentPageMark, [ &isSetVaule ] { __DBG_STUB_INVOKE__ isSetVaule = true; });

    QShowEvent event;
    view->showEvent(&event);
    EXPECT_TRUE(isSetVaule);
}
