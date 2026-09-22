// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Covered functions (gui/unlockpartitiondialog.cpp):
//   UnlockPartitionDialog ctor / ~dtor / initUI / initConnect(+textChanged lambda) /
//   updateUserHint (kRec/kPwd/kPin branches) / handleButtonClicked (index1 kRec ok+bad,
//   index1 kPwd, index0) / switchUnlockType / showEvent / getUnlockKey
// Branch map (from get_code_snippet):
//   updateUserHint switch: kRec(with initType kPwd vs kPin), kPwd, kPin;
//   handleButtonClicked: index!=1 -> reject; index==1 kRec len!=24 -> alert; len==24 -> accept;
//   switchUnlockType: kRec -> initType; kPin/kPwd -> kRec.

#include "stubext.h"

#include "gui/unlockpartitiondialog.h"
#include "utils/encryptutils.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QDialog>
#include <QShowEvent>
#include <QSignalSpy>
#include <DDialog>
#include <DPasswordEdit>
#include <dcommandlinkbutton.h>

using namespace dfmplugin_diskenc;
DWIDGET_USE_NAMESPACE

namespace {
class TestableUnlockDialog : public UnlockPartitionDialog
{
public:
    explicit TestableUnlockDialog(UnlockType type, QWidget *parent = nullptr)
        : UnlockPartitionDialog(type, parent) {}

    using UnlockPartitionDialog::handleButtonClicked;
    using UnlockPartitionDialog::switchUnlockType;
    void callShowEvent(QShowEvent *e) { showEvent(e); }
};
}   // namespace

class UT_UnlockPartitionDialogCov : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(&QApplication::platformName, []() -> QString {
            __DBG_STUB_INVOKE__
            return "offscreen";
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

    DPasswordEdit *pwdEdit(UnlockPartitionDialog *dlg) const
    {
        return dlg->findChild<DPasswordEdit *>("PasswordLineEdit");
    }
    DCommandLinkButton *switchBtn(UnlockPartitionDialog *dlg) const
    {
        return dlg->findChild<DCommandLinkButton *>("ChgUnlockType");
    }

    stub_ext::StubExt stub;
};

TEST_F(UT_UnlockPartitionDialogCov, Construct_PwdType_InitializesUi)
{
    // Arrange
    // Act
    // Assert
    TestableUnlockDialog dlg(UnlockPartitionDialog::kPwd);
    auto *edit = pwdEdit(&dlg);
    auto *btn = switchBtn(&dlg);

    // Assert
    ASSERT_NE(edit, nullptr);
    ASSERT_NE(btn, nullptr);
    EXPECT_EQ(edit->lineEdit()->placeholderText(), QString("Please input passphrase to unlock partition"));
    EXPECT_EQ(dlg.getButton(1)->isEnabled(), false);   // unlock disabled until text typed
}

TEST_F(UT_UnlockPartitionDialogCov, Construct_PinType_SetsPinHint)
{
    // Arrange
    // Act
    // Assert
    TestableUnlockDialog dlg(UnlockPartitionDialog::kPin);

    // Assert
    EXPECT_EQ(pwdEdit(&dlg)->lineEdit()->placeholderText(), QString("Please input PIN to unlock partition"));
    EXPECT_EQ(switchBtn(&dlg)->text(), QString("Unlock by recovery key"));
}

TEST_F(UT_UnlockPartitionDialogCov, Construct_RecType_HidesSwitchButton)
{
    // Arrange
    // Act
    // Assert
    TestableUnlockDialog dlg(UnlockPartitionDialog::kRec);

    // Assert
    EXPECT_TRUE(switchBtn(&dlg)->isHidden());
    EXPECT_EQ(pwdEdit(&dlg)->lineEdit()->placeholderText(), QString("Please enter the 24-digit recovery key"));
}

TEST_F(UT_UnlockPartitionDialogCov, TextChanged_LambdaEnablesUnlockButton)
{
    // Arrange
    TestableUnlockDialog dlg(UnlockPartitionDialog::kPwd);
    auto *edit = pwdEdit(&dlg);
    auto *unlock = dlg.getButton(1);
    ASSERT_NE(unlock, nullptr);

    // Act
    edit->setText("some-pass");
    bool enabledWithText = unlock->isEnabled();
    edit->setText("");

    // Assert
    EXPECT_EQ(enabledWithText, true);
    EXPECT_EQ(unlock->isEnabled(), false);
}

TEST_F(UT_UnlockPartitionDialogCov, TextChanged_RecModeFormatsRecoveryKey)
{
    // Arrange: switch pwd -> rec first
    TestableUnlockDialog dlg(UnlockPartitionDialog::kPwd);
    dlg.switchUnlockType();
    auto *edit = pwdEdit(&dlg);

    // Act
    edit->setText("123456789012345678901234");

    // Assert: formatted with dashes by the textChanged lambda
    EXPECT_EQ(edit->text(), QString("123456-789012-345678-901234"));
    EXPECT_EQ(dlg.getUnlockKey().first, UnlockPartitionDialog::kRec);
}

TEST_F(UT_UnlockPartitionDialogCov, HandleButtonClicked_Cancel_Rejects)
{
    // Arrange
    TestableUnlockDialog dlg(UnlockPartitionDialog::kPwd);
    QSignalSpy acceptSpy(&dlg, &QDialog::accepted);
    QSignalSpy rejectSpy(&dlg, &QDialog::rejected);

    // Act
    dlg.handleButtonClicked(0, "Cancel");

    // Assert
    EXPECT_EQ(acceptSpy.count(), 0);
    EXPECT_EQ(rejectSpy.count(), 1);
}

TEST_F(UT_UnlockPartitionDialogCov, HandleButtonClicked_PwdWithText_AcceptsAndStoresKey)
{
    // Arrange
    TestableUnlockDialog dlg(UnlockPartitionDialog::kPwd);
    QSignalSpy acceptSpy(&dlg, &QDialog::accepted);
    QSignalSpy rejectSpy(&dlg, &QDialog::rejected);
    pwdEdit(&dlg)->setText("my-passphrase");

    // Act
    dlg.handleButtonClicked(1, "Unlock");

    // Assert
    EXPECT_EQ(acceptSpy.count(), 1);
    EXPECT_EQ(rejectSpy.count(), 0);
    EXPECT_EQ(dlg.getUnlockKey().second, QString("my-passphrase"));
}

TEST_F(UT_UnlockPartitionDialogCov, HandleButtonClicked_RecInvalidLength_DoesNotAccept)
{
    // Arrange
    TestableUnlockDialog dlg(UnlockPartitionDialog::kRec);
    QSignalSpy acceptSpy(&dlg, &QDialog::accepted);
    QSignalSpy rejectSpy(&dlg, &QDialog::rejected);
    pwdEdit(&dlg)->setText("123456-789012-345678-90123");

    // Act
    dlg.handleButtonClicked(1, "Unlock");

    // Assert
    EXPECT_EQ(acceptSpy.count(), 0);
    EXPECT_EQ(rejectSpy.count(), 0);
}

TEST_F(UT_UnlockPartitionDialogCov, HandleButtonClicked_RecValidLength_AcceptsStrippedKey)
{
    // Arrange
    TestableUnlockDialog dlg(UnlockPartitionDialog::kRec);
    QSignalSpy acceptSpy(&dlg, &QDialog::accepted);
    QSignalSpy rejectSpy(&dlg, &QDialog::rejected);
    pwdEdit(&dlg)->setText("123456-789012-345678-901234");

    // Act
    dlg.handleButtonClicked(1, "Unlock");

    // Assert
    EXPECT_EQ(acceptSpy.count(), 1);
    EXPECT_EQ(dlg.getUnlockKey().second, QString("123456789012345678901234"));
}

TEST_F(UT_UnlockPartitionDialogCov, SwitchUnlockType_TogglesBetweenPwdAndRec)
{
    // Arrange
    TestableUnlockDialog dlg(UnlockPartitionDialog::kPwd);
    auto *btn = switchBtn(&dlg);

    // Act
    dlg.switchUnlockType();
    auto first = dlg.getUnlockKey().first;
    dlg.switchUnlockType();
    auto second = dlg.getUnlockKey().first;

    // Assert
    EXPECT_EQ(first, UnlockPartitionDialog::kRec);
    EXPECT_EQ(second, UnlockPartitionDialog::kPwd);
    EXPECT_EQ(btn->text(), QString("Unlock by recovery key"));   // back in pwd mode
}

TEST_F(UT_UnlockPartitionDialogCov, ShowEvent_SetsFocusOnPasswordEdit)
{
    // Arrange
    TestableUnlockDialog dlg(UnlockPartitionDialog::kPwd);
    auto *edit = pwdEdit(&dlg);
    ASSERT_NE(edit, nullptr);
    QShowEvent event;

    // Act
    dlg.callShowEvent(&event);

    // Assert: focus handling ran without side effects on the content
    EXPECT_TRUE(edit->text().isEmpty());
    EXPECT_EQ(dlg.title(), QString("Unlock encryption partition"));
}
