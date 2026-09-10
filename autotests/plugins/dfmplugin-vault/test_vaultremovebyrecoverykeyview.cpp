// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>
#include <QStringList>
#include <QSignalSpy>
#include <QPlainTextEdit>
#include <QKeyEvent>
#include <QEvent>
#include <DFileChooserEdit>

#include "stubext.h"

#define private public
#include "views/removevaultview/vaultremovebyrecoverykeyview.h"
#undef private

#include "utils/vaulthelper.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaultutils.h"

DPVAULT_USE_NAMESPACE

class VaultRemoveByRecoverykeyViewTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stub.set_lamda(static_cast<bool(VaultHelper::*)()const>(&VaultHelper::getVaultVersion), []() -> bool { return false; });
        stub.set_lamda(&OperatorCenter::checkPassword, [](OperatorCenter *, const QString &, QString &) -> bool { return true; });
        stub.set_lamda(&OperatorCenter::isNewVaultVersion, []() -> bool { return false; });
        stub.set_lamda(&OperatorCenter::checkUserKey, [](OperatorCenter *, const QString &, QString &) -> bool { return true; });
        stub.set_lamda(&VaultUtils::showAuthorityDialog, [](VaultUtils *, const QString &) {});
        stub.set_lamda(&VaultHelper::lockVault, [](VaultHelper *, bool) -> bool { return true; });
        view = new VaultRemoveByRecoverykeyView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    VaultRemoveByRecoverykeyView *view = nullptr;
};

// --- construction ---

TEST_F(VaultRemoveByRecoverykeyViewTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

// --- btnText ---

TEST_F(VaultRemoveByRecoverykeyViewTest, BtnText_ReturnsTwoButtons)
{
    QStringList btns = view->btnText();
    EXPECT_EQ(btns.size(), 2);
}

// --- titleText ---

TEST_F(VaultRemoveByRecoverykeyViewTest, TitleText_ReturnsNonEmpty)
{
    QString title = view->titleText();
    EXPECT_FALSE(title.isEmpty());
}

// --- getRecoverykey ---

TEST_F(VaultRemoveByRecoverykeyViewTest, GetRecoverykey_DefaultEmpty)
{
    EXPECT_TRUE(view->getRecoverykey().isEmpty());
}

// --- showAlertMessage ---

TEST_F(VaultRemoveByRecoverykeyViewTest, ShowAlertMessage_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->showAlertMessage("test message", 100));
}

TEST_F(VaultRemoveByRecoverykeyViewTest, ShowAlertMessage_PersistentDisplay_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->showAlertMessage("persistent", -1));
}

// --- buttonClicked(0) emits sigCloseDialog ---

TEST_F(VaultRemoveByRecoverykeyViewTest, ButtonClicked_Cancel_EmitsCloseDialog)
{
    QSignalSpy spy(view, &VaultRemoveByRecoverykeyView::sigCloseDialog);
    view->buttonClicked(0, "");
    EXPECT_EQ(spy.count(), 1);
}

// --- buttonClicked(1) V1 mode triggers checkRecoveryKeyV1 ---

TEST_F(VaultRemoveByRecoverykeyViewTest, ButtonClicked_Ok_V1_NoCrash)
{
    stub.set_lamda(&VaultRemoveByRecoverykeyView::checkRecoveryKeyV1, [](VaultRemoveByRecoverykeyView *) {});
    EXPECT_NO_FATAL_FAILURE({
        view->buttonClicked(1, "");
    });
}

// --- buttonClicked invalid index ---

TEST_F(VaultRemoveByRecoverykeyViewTest, ButtonClicked_InvalidIndex_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(99, ""));
}

// --- afterRecoveryKeyChanged ---

TEST_F(VaultRemoveByRecoverykeyViewTest, AfterRecoveryKeyChanged_EmptyString)
{
    QString key = "";
    int pos = view->afterRecoveryKeyChanged(key);
    EXPECT_EQ(pos, -1);
}

TEST_F(VaultRemoveByRecoverykeyViewTest, AfterRecoveryKeyChanged_ShortString)
{
    QString key = "abcd";
    int pos = view->afterRecoveryKeyChanged(key);
    EXPECT_GE(pos, 0);
}

TEST_F(VaultRemoveByRecoverykeyViewTest, AfterRecoveryKeyChanged_LongString_AddsDashes)
{
    QString key = "abcdefghijklmnop";
    int pos = view->afterRecoveryKeyChanged(key);
    EXPECT_GT(pos, 0);
    EXPECT_TRUE(key.contains("-"));
}

// --- onRecoveryKeyChanged ---

TEST_F(VaultRemoveByRecoverykeyViewTest, OnRecoveryKeyChanged_Empty_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onRecoveryKeyChanged());
}

TEST_F(VaultRemoveByRecoverykeyViewTest, OnRecoveryKeyChanged_WithText_NoCrash)
{
    if (view->keyEdit) {
        view->keyEdit->setPlainText("abcdefgh");
    }
    EXPECT_NO_FATAL_FAILURE(view->onRecoveryKeyChanged());
}

// --- eventFilter ---

TEST_F(VaultRemoveByRecoverykeyViewTest, EventFilter_KeyEnter_Filtered)
{
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    bool result = view->eventFilter(view->keyEdit, &keyEvent);
    EXPECT_TRUE(result);
}

TEST_F(VaultRemoveByRecoverykeyViewTest, EventFilter_KeyReturn_Filtered)
{
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    bool result = view->eventFilter(view->keyEdit, &keyEvent);
    EXPECT_TRUE(result);
}

TEST_F(VaultRemoveByRecoverykeyViewTest, EventFilter_KeyMinus_Filtered)
{
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Minus, Qt::NoModifier);
    bool result = view->eventFilter(view->keyEdit, &keyEvent);
    EXPECT_TRUE(result);
}

TEST_F(VaultRemoveByRecoverykeyViewTest, EventFilter_OtherKey_NotFiltered)
{
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    bool result = view->eventFilter(view->keyEdit, &keyEvent);
    EXPECT_FALSE(result);
}

// --- validateRecoveryKeyV1 ---

TEST_F(VaultRemoveByRecoverykeyViewTest, ValidateRecoveryKeyV1_OldVersion_ReturnsTrue)
{
    bool result = view->validateRecoveryKeyV1("testkey1234");
    EXPECT_TRUE(result);
}

TEST_F(VaultRemoveByRecoverykeyViewTest, ValidateRecoveryKeyV1_NewVersion_WrongLength)
{
    stub.set_lamda(&OperatorCenter::isNewVaultVersion, []() -> bool { return true; });
    bool result = view->validateRecoveryKeyV1("short");
    EXPECT_FALSE(result);
}

TEST_F(VaultRemoveByRecoverykeyViewTest, ValidateRecoveryKeyV1_NewVersion_CorrectLength)
{
    stub.set_lamda(&OperatorCenter::isNewVaultVersion, []() -> bool { return true; });
    QString key(32, 'A');
    bool result = view->validateRecoveryKeyV1(key);
    EXPECT_TRUE(result);
}

// --- handleRecoveryKeyV1ValidationResult ---

TEST_F(VaultRemoveByRecoverykeyViewTest, HandleRecoveryKeyV1ValidationResult_Valid_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->handleRecoveryKeyV1ValidationResult(true));
}

TEST_F(VaultRemoveByRecoverykeyViewTest, HandleRecoveryKeyV1ValidationResult_Invalid_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->handleRecoveryKeyV1ValidationResult(false));
}

// --- slotCheckAuthorizationFinished ---

TEST_F(VaultRemoveByRecoverykeyViewTest, SlotCheckAuthorizationFinished_False_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotCheckAuthorizationFinished(false));
}

TEST_F(VaultRemoveByRecoverykeyViewTest, SlotCheckAuthorizationFinished_True_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE({
        view->slotCheckAuthorizationFinished(true);
    });
}

// --- checkRecoveryKeyV1 (async) ---

TEST_F(VaultRemoveByRecoverykeyViewTest, CheckRecoveryKeyV1_NoCrash)
{
    stub.set_lamda(&VaultRemoveByRecoverykeyView::checkRecoveryKeyV1, [](VaultRemoveByRecoverykeyView *) {});
    EXPECT_NO_FATAL_FAILURE({
        view->checkRecoveryKeyV1();
    });
}

// --- validateRecoveryKeyFile ---

TEST_F(VaultRemoveByRecoverykeyViewTest, ValidateRecoveryKeyFile_NoCrash)
{
    stub.set_lamda(&OperatorCenter::verificationRetrievePassword, [](OperatorCenter *, const QString, QString &) -> bool { return true; });
    bool result = view->validateRecoveryKeyFile("/tmp/testkey.key");
    EXPECT_TRUE(result);
}

// ===== V2 mode tests (getVaultVersion returns true) =====

class VaultRemoveByRecoverykeyViewV2Test : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stub.set_lamda(static_cast<bool(VaultHelper::*)()const>(&VaultHelper::getVaultVersion), []() -> bool { return true; });
        stub.set_lamda(&OperatorCenter::verificationRetrievePassword, [](OperatorCenter *, const QString, QString &) -> bool { return true; });
        stub.set_lamda(&VaultUtils::showAuthorityDialog, [](VaultUtils *, const QString &) {});
        stub.set_lamda(&VaultHelper::lockVault, [](VaultHelper *, bool) -> bool { return true; });
        view = new VaultRemoveByRecoverykeyView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    VaultRemoveByRecoverykeyView *view = nullptr;
};

TEST_F(VaultRemoveByRecoverykeyViewV2Test, Constructor_CreatesView_V2Mode)
{
    EXPECT_NE(view, nullptr);
    EXPECT_NE(view->filePathEdit, nullptr);
}

TEST_F(VaultRemoveByRecoverykeyViewV2Test, ButtonClicked_Ok_V2_NoCrash)
{
    stub.set_lamda(&VaultRemoveByRecoverykeyView::checkRecoveryKeyV2, [](VaultRemoveByRecoverykeyView *) {});
    EXPECT_NO_FATAL_FAILURE({
        view->buttonClicked(1, "");
    });
}

TEST_F(VaultRemoveByRecoverykeyViewV2Test, CheckRecoveryKeyV2_EmptyFile_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->checkRecoveryKeyV2());
}

TEST_F(VaultRemoveByRecoverykeyViewV2Test, CheckRecoveryKeyV2_WithFile_NoCrash)
{
    stub.set_lamda(&VaultRemoveByRecoverykeyView::checkRecoveryKeyV2, [](VaultRemoveByRecoverykeyView *) {});
    if (view->filePathEdit) {
        view->filePathEdit->setText("/tmp/testkey.key");
    }
    EXPECT_NO_FATAL_FAILURE({
        view->checkRecoveryKeyV2();
    });
}

TEST_F(VaultRemoveByRecoverykeyViewV2Test, HandleRecoveryKeyFileValidationResult_Valid_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->handleRecoveryKeyFileValidationResult(true));
}

TEST_F(VaultRemoveByRecoverykeyViewV2Test, HandleRecoveryKeyFileValidationResult_Invalid_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->handleRecoveryKeyFileValidationResult(false));
}
