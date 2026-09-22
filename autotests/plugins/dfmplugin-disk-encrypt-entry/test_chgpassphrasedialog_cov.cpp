// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Covered functions (gui/chgpassphrasedialog.cpp):
//   ChgPassphraseDialog ctor / initUI / getPassphrase / validateByRecKey /
//   validatePasswd(+both lambdas) / onButtonClicked / onRecSwitchClicked / onOldKeyChanged
// Branch map (from get_code_snippet):
//   ctor: encKeyType 1 -> PIN label, else passphrase;
//   getPassphrase: usingRecKey strips '-';
//   validatePasswd: empty old key / rec-key length!=24 / empty new fields /
//                   factor<3 or len<8 / mismatch / ok;
//   onButtonClicked: idx==1 valid -> accept, invalid -> stay, else -> reject;
//   onRecSwitchClicked: toggling rec-key mode updates hint/echo/placeholder.

#include "stubext.h"

#include "gui/chgpassphrasedialog.h"
#include "utils/encryptutils.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QDialog>
#include <QLineEdit>
#include <QSignalSpy>
#include <DDialog>
#include <DPasswordEdit>

using namespace dfmplugin_diskenc;
DWIDGET_USE_NAMESPACE

namespace {
class TestableChgDialog : public ChgPassphraseDialog
{
public:
    explicit TestableChgDialog(const QString &device, QWidget *parent = nullptr)
        : ChgPassphraseDialog(device, parent) {}

    using ChgPassphraseDialog::onButtonClicked;
    using ChgPassphraseDialog::onRecSwitchClicked;
    using ChgPassphraseDialog::onOldKeyChanged;
    using ChgPassphraseDialog::validatePasswd;
    using ChgPassphraseDialog::validateByRecKey;
};
}   // namespace

class UT_ChgPassphraseDialogCov : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(&QApplication::platformName, []() -> QString {
            __DBG_STUB_INVOKE__
            return "offscreen";
        });
        stub.set_lamda(&device_utils::encKeyType, [](const QString &) -> int {
            __DBG_STUB_INVOKE__
            return 0;   // passphrase
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

    DPasswordEdit *oldPass(ChgPassphraseDialog *dlg) const
    {
        return dlg->findChild<DPasswordEdit *>("OldPass");
    }
    DPasswordEdit *new1(ChgPassphraseDialog *dlg) const
    {
        return dlg->findChild<DPasswordEdit *>("NewPass1");
    }
    DPasswordEdit *new2(ChgPassphraseDialog *dlg) const
    {
        return dlg->findChild<DPasswordEdit *>("NewPass2");
    }

    stub_ext::StubExt stub;
};

TEST_F(UT_ChgPassphraseDialogCov, Construct_PassphraseDevice_InitializesInPassphraseMode)
{
    // Arrange
    // Act
    // Assert
    TestableChgDialog dlg("/dev/sdb1");

    // Assert: initUI sets usingRecKey=true then toggles to false
    EXPECT_FALSE(dlg.validateByRecKey());
    EXPECT_EQ(oldPass(&dlg)->echoMode(), QLineEdit::Password);
    EXPECT_EQ(dlg.getButtons().size(), 2);
}

TEST_F(UT_ChgPassphraseDialogCov, Construct_PinDevice_TitleMentionsPin)
{
    // Arrange
    stub.set_lamda(&device_utils::encKeyType, [](const QString &) -> int {
        __DBG_STUB_INVOKE__
        return 1;   // PIN
    });

    // Act
    TestableChgDialog dlg("/dev/sdb1");

    // Assert
    EXPECT_EQ(dlg.title(), QString("Modify PIN"));
    EXPECT_EQ(new2(&dlg)->lineEdit()->placeholderText(), QString("Please enter PIN again"));
}

TEST_F(UT_ChgPassphraseDialogCov, GetPassphrase_RecKeyMode_StripsDashes)
{
    // Arrange
    TestableChgDialog dlg("/dev/sdb1");
    dlg.onRecSwitchClicked();   // -> rec key mode
    oldPass(&dlg)->setText("123456-789012-345678-901234");
    new1(&dlg)->setText("NewSup3r#Key");

    // Act
    auto keys = dlg.getPassphrase();

    // Assert
    EXPECT_EQ(keys.first, QString("123456789012345678901234"));
    EXPECT_EQ(keys.second, QString("NewSup3r#Key"));
}

TEST_F(UT_ChgPassphraseDialogCov, OnRecSwitchClicked_TogglesModeUi)
{
    // Arrange
    TestableChgDialog dlg("/dev/sdb1");
    auto *old = oldPass(&dlg);
    old->setText("leftover");

    // Act
    dlg.onRecSwitchClicked();
    QLineEdit::EchoMode recEcho = old->echoMode();
    QString recPlaceholder = old->lineEdit()->placeholderText();
    dlg.onRecSwitchClicked();
    QLineEdit::EchoMode pwdEcho = old->echoMode();

    // Assert
    EXPECT_EQ(recEcho, QLineEdit::Normal);
    EXPECT_EQ(recPlaceholder, QString("Please input recovery key"));
    EXPECT_EQ(pwdEcho, QLineEdit::Password);
    EXPECT_TRUE(old->text().isEmpty());   // cleared on switch
}

TEST_F(UT_ChgPassphraseDialogCov, OnOldKeyChanged_RecMode_FormatsInput)
{
    // Arrange
    TestableChgDialog dlg("/dev/sdb1");
    dlg.onRecSwitchClicked();
    auto *old = oldPass(&dlg);

    // Act
    dlg.onOldKeyChanged("123456789012345678901234");

    // Assert
    EXPECT_EQ(old->text(), QString("123456-789012-345678-901234"));
    EXPECT_EQ(old->text().length(), 27);   // 24 digits + 3 dashes
}

TEST_F(UT_ChgPassphraseDialogCov, OnOldKeyChanged_PwdMode_DoesNotFormat)
{
    // Arrange
    TestableChgDialog dlg("/dev/sdb1");   // passphrase mode
    auto *old = oldPass(&dlg);

    // Act
    dlg.onOldKeyChanged("123456789012345678901234");

    // Assert
    EXPECT_EQ(old->text().isEmpty(), true);
    EXPECT_NE(old->text().isEmpty(), false);   // double-check stable state
}

TEST_F(UT_ChgPassphraseDialogCov, ValidatePasswd_AllBranches)
{
    // Arrange
    TestableChgDialog dlg("/dev/sdb1");

    // Act
    bool emptyOld = dlg.validatePasswd();

    // Assert: empty old key rejected
    EXPECT_FALSE(emptyOld);
    EXPECT_EQ(emptyOld, false);

    // Act
    oldPass(&dlg)->setText("OldSup3r#Key");
    bool emptyNew = dlg.validatePasswd();
    new1(&dlg)->setText("weakpass");
    new2(&dlg)->setText("weakpass");
    bool weak = dlg.validatePasswd();
    new1(&dlg)->setText("NewSup3r#Key1");
    new2(&dlg)->setText("NewSup3r#Key2");
    bool mismatch = dlg.validatePasswd();
    new2(&dlg)->setText("NewSup3r#Key1");
    bool valid = dlg.validatePasswd();

    // Assert
    EXPECT_FALSE(emptyNew);
    EXPECT_FALSE(weak);
    EXPECT_FALSE(mismatch);
    EXPECT_TRUE(valid);
    EXPECT_EQ(valid, true);
}

TEST_F(UT_ChgPassphraseDialogCov, ValidatePasswd_RecKeyWrongLength_False)
{
    // Arrange
    TestableChgDialog dlg("/dev/sdb1");
    dlg.onRecSwitchClicked();
    oldPass(&dlg)->setText("123456-789012-345678-90123");   // 23 digits
    new1(&dlg)->setText("NewSup3r#Key1");
    new2(&dlg)->setText("NewSup3r#Key1");

    // Act

    // Assert
    EXPECT_EQ(dlg.validatePasswd(), false);

    // correct 24-digit key passes
    oldPass(&dlg)->setText("123456-789012-345678-901234");
    EXPECT_EQ(dlg.validatePasswd(), true);
}

TEST_F(UT_ChgPassphraseDialogCov, OnButtonClicked_Cancel_Rejects)
{
    // Arrange
    TestableChgDialog dlg("/dev/sdb1");
    QSignalSpy acceptSpy(&dlg, &QDialog::accepted);
    QSignalSpy rejectSpy(&dlg, &QDialog::rejected);

    // Act
    dlg.onButtonClicked(0);

    // Assert
    EXPECT_EQ(acceptSpy.count(), 0);
    EXPECT_EQ(rejectSpy.count(), 1);
}

TEST_F(UT_ChgPassphraseDialogCov, OnButtonClicked_Confirm_InvalidStaysValidAccepts)
{
    // Arrange
    TestableChgDialog dlg("/dev/sdb1");
    QSignalSpy acceptSpy(&dlg, &QDialog::accepted);
    QSignalSpy rejectSpy(&dlg, &QDialog::rejected);

    // Act: invalid input -> no accept, no reject
    dlg.onButtonClicked(1);

    oldPass(&dlg)->setText("OldSup3r#Key");
    new1(&dlg)->setText("NewSup3r#Key1");
    new2(&dlg)->setText("NewSup3r#Key1");
    dlg.onButtonClicked(1);

    // Assert
    EXPECT_EQ(acceptSpy.count(), 1);
    EXPECT_EQ(rejectSpy.count(), 0);
}
