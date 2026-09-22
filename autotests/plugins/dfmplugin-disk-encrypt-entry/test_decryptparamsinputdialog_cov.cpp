// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Covered functions (gui/decryptparamsinputdialog.cpp):
//   DecryptParamsInputDialog ctor / initUI / getKey / setInputPIN / usingRecKey /
//   onRecSwitchClicked / onKeyChanged / onButtonClicked / updateUserHints
// Branch map (from get_code_snippet):
//   ctor/updateUserHints: set useRecKey=true then toggle -> passphrase mode;
//   onRecSwitchClicked: rec mode (with requestPIN on/off) vs passphrase mode;
//   onButtonClicked: idx!=0 -> reject; empty key -> alert; rec len!=24 -> alert; ok -> accept.

#include "stubext.h"

#include "gui/decryptparamsinputdialog.h"
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
class TestableDecryptDialog : public DecryptParamsInputDialog
{
public:
    explicit TestableDecryptDialog(const QString &device, QWidget *parent = nullptr)
        : DecryptParamsInputDialog(device, parent) {}

    using DecryptParamsInputDialog::onRecSwitchClicked;
    using DecryptParamsInputDialog::onKeyChanged;
    using DecryptParamsInputDialog::onButtonClicked;
    using DecryptParamsInputDialog::updateUserHints;
};
}   // namespace

class UT_DecryptParamsInputDialogCov : public testing::Test
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

    DPasswordEdit *editor(DecryptParamsInputDialog *dlg) const
    {
        return dlg->findChild<DPasswordEdit *>("Editor_2");
    }

    stub_ext::StubExt stub;
};

TEST_F(UT_DecryptParamsInputDialogCov, Construct_StartsInPassphraseMode)
{
    // Arrange
    // Act: updateUserHints() sets useRecKey=true then toggles once
    // Assert
    TestableDecryptDialog dlg("/dev/sdb1");
    auto *edit = editor(&dlg);

    // Assert: construction ends in passphrase mode
    ASSERT_NE(edit, nullptr);
    EXPECT_FALSE(dlg.usingRecKey());
    EXPECT_EQ(edit->echoMode(), QLineEdit::Password);
    EXPECT_EQ(dlg.getButtons().size(), 1);
}

TEST_F(UT_DecryptParamsInputDialogCov, GetKey_RecMode_StripsDashes)
{
    // Arrange: switch from passphrase mode into recovery key mode
    TestableDecryptDialog dlg("/dev/sdb1");
    dlg.onRecSwitchClicked();
    editor(&dlg)->setText("123456-789012-345678-901234");

    // Act
    QString key = dlg.getKey();

    // Assert
    EXPECT_EQ(key, QString("123456789012345678901234"));
    EXPECT_EQ(key.length(), 24);
}

TEST_F(UT_DecryptParamsInputDialogCov, OnRecSwitchClicked_TogglesToPassphraseAndBack)
{
    // Arrange: construction already leaves the dialog in passphrase mode
    TestableDecryptDialog dlg("/dev/sdb1");
    auto *edit = editor(&dlg);

    // Act
    bool pwdFlag = dlg.usingRecKey();
    dlg.onRecSwitchClicked();   // -> recovery key mode
    bool recFlag = dlg.usingRecKey();
    QLineEdit::EchoMode recEcho = edit->echoMode();
    dlg.onRecSwitchClicked();   // -> passphrase mode again

    // Assert
    EXPECT_FALSE(pwdFlag);
    EXPECT_TRUE(recFlag);
    EXPECT_EQ(recEcho, QLineEdit::Normal);
    EXPECT_EQ(edit->echoMode(), QLineEdit::Password);
}

TEST_F(UT_DecryptParamsInputDialogCov, SetInputPIN_True_UpdatesHintsToPassphraseWording)
{
    // Arrange: switch to recovery key mode first so setInputPIN toggles back
    TestableDecryptDialog dlg("/dev/sdb1");
    dlg.onRecSwitchClicked();

    // Act
    dlg.setInputPIN(true);

    // Assert: requestPIN changes the mode text; still passphrase mode
    EXPECT_EQ(dlg.usingRecKey(), false);
    EXPECT_EQ(editor(&dlg)->lineEdit()->placeholderText().isEmpty(), false);
}

TEST_F(UT_DecryptParamsInputDialogCov, OnKeyChanged_StoresPassphrase)
{
    // Arrange
    TestableDecryptDialog dlg("/dev/sdb1");
    dlg.onRecSwitchClicked();   // passphrase mode, no dash stripping

    // Act
    dlg.onKeyChanged("My#Passw0rd");

    // Assert
    EXPECT_EQ(dlg.getKey(), QString("My#Passw0rd"));
    EXPECT_EQ(dlg.getKey().length(), 11);
}

TEST_F(UT_DecryptParamsInputDialogCov, OnButtonClicked_NonZero_Rejects)
{
    // Arrange
    TestableDecryptDialog dlg("/dev/sdb1");
    QSignalSpy acceptSpy(&dlg, &QDialog::accepted);
    QSignalSpy rejectSpy(&dlg, &QDialog::rejected);

    // Act
    dlg.onButtonClicked(3);

    // Assert
    EXPECT_EQ(acceptSpy.count(), 0);
    EXPECT_EQ(rejectSpy.count(), 1);
}

TEST_F(UT_DecryptParamsInputDialogCov, OnButtonClicked_EmptyKey_DoesNotAccept)
{
    // Arrange
    TestableDecryptDialog dlg("/dev/sdb1");
    QSignalSpy acceptSpy(&dlg, &QDialog::accepted);
    QSignalSpy rejectSpy(&dlg, &QDialog::rejected);

    // Act: key field empty
    dlg.onButtonClicked(0);

    // Assert
    EXPECT_EQ(acceptSpy.count(), 0);
    EXPECT_EQ(rejectSpy.count(), 0);
}

TEST_F(UT_DecryptParamsInputDialogCov, OnButtonClicked_RecKeyInvalidLength_DoesNotAccept)
{
    // Arrange: switch into rec mode, then a 23-digit key
    TestableDecryptDialog dlg("/dev/sdb1");
    dlg.onRecSwitchClicked();
    editor(&dlg)->setText("123456-789012-345678-90123");
    QSignalSpy acceptSpy(&dlg, &QDialog::accepted);
    QSignalSpy rejectSpy(&dlg, &QDialog::rejected);

    // Act
    dlg.onButtonClicked(0);

    // Assert
    EXPECT_EQ(acceptSpy.count(), 0);
    EXPECT_EQ(rejectSpy.count(), 0);
}

TEST_F(UT_DecryptParamsInputDialogCov, OnButtonClicked_ValidInputs_Accepts)
{
    // Arrange: construction leaves the dialog in passphrase mode
    TestableDecryptDialog dlg("/dev/sdb1");
    editor(&dlg)->setText("My#Passw0rd");
    QSignalSpy acceptSpy(&dlg, &QDialog::accepted);
    QSignalSpy rejectSpy(&dlg, &QDialog::rejected);

    // Act
    dlg.onButtonClicked(0);

    // Assert
    EXPECT_EQ(acceptSpy.count(), 1);
    EXPECT_EQ(rejectSpy.count(), 0);
}

TEST_F(UT_DecryptParamsInputDialogCov, UpdateUserHints_EndsInPassphraseMode)
{
    // Arrange: enter recovery key mode first
    TestableDecryptDialog dlg("/dev/sdb1");
    dlg.onRecSwitchClicked();
    ASSERT_TRUE(dlg.usingRecKey());

    // Act
    dlg.updateUserHints();

    // Assert: sets useRecKey=true then toggles once -> passphrase mode
    EXPECT_FALSE(dlg.usingRecKey());
    EXPECT_EQ(editor(&dlg)->echoMode(), QLineEdit::Password);
}
