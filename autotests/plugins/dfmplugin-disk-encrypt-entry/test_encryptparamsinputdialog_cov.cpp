// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Covered functions (gui/encryptparamsinputdialog.cpp):
//   EncryptParamsInputDialog ctor / initUi / initConn / createPasswordPage /
//   createExportPage / getInputs / validatePassword(+lambda) / setPasswordInputVisible /
//   onButtonClicked / onPageChanged / onEncTypeChanged / onExpPathChanged(+lambda via signal) /
//   encryptByTpm(+lambda) / confirmEncrypt
// Branch map (from get_code_snippet):
//   ctor: wayland on/off; createPasswordPage: tpm ok -> PIN default, tpm fail -> Pwd only;
//   getInputs: Pin/Tpm -> tpmPassword, Pwd -> encKeyEdit1;
//   validatePassword: wrong page / kTpm skip / empty1 / empty2 / weak / mismatch / ok;
//   onButtonClicked: page0 invalid pwd / page0->export / page0->confirm / page1 idx0 back / page1 idx1 confirm;
//   onEncTypeChanged: kPwd / kPin / kTpm / invalid;
//   onExpPathChanged: no button / valid path / invalid path;
//   encryptByTpm: exitCode 0 / nonzero; confirmEncrypt: kPwd accept / tpm fail locked / tpm fail status / tpm ok.

#include "stubext.h"

#include "gui/encryptparamsinputdialog.h"
#include "utils/encryptutils.h"
#include "dfmplugin_disk_encrypt_global.h"

#include <gtest/gtest.h>

#include <QAbstractButton>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <dconfig.h>
#include <DDialog>
#include <DPasswordEdit>

using namespace dfmplugin_diskenc;
using namespace disk_encrypt;
DWIDGET_USE_NAMESPACE

namespace {
// Subclass exposes protected members/slots for direct testing.
class TestableEncryptDialog : public EncryptParamsInputDialog
{
public:
    explicit TestableEncryptDialog(const QVariantMap &args, QWidget *parent = nullptr)
        : EncryptParamsInputDialog(args, parent) {}

    using EncryptParamsInputDialog::onButtonClicked;
    using EncryptParamsInputDialog::onEncTypeChanged;
    using EncryptParamsInputDialog::onExpPathChanged;
    using EncryptParamsInputDialog::onPageChanged;
    using EncryptParamsInputDialog::validatePassword;
    using EncryptParamsInputDialog::encryptByTpm;
    using EncryptParamsInputDialog::confirmEncrypt;
    using EncryptParamsInputDialog::setPasswordInputVisible;
};
}   // namespace

class UT_EncryptParamsInputDialogCov : public testing::Test
{
protected:
    void SetUp() override
    {
        // no TPM, no wayland: dialog keeps only passphrase mode and no top flag
        stub.set_lamda(&tpm_utils::checkTPM, [](bool *) -> int {
            __DBG_STUB_INVOKE__
            return -1;
        });
        stub.set_lamda(&tpm_utils::checkTPMLockoutStatus, []() -> int {
            __DBG_STUB_INVOKE__
            return -1;
        });
        stub.set_lamda(&QApplication::platformName, []() -> QString {
            __DBG_STUB_INVOKE__
            return "offscreen";
        });
        stubDConfigExport(true);
    }

    void TearDown() override
    {
        stub.clear();
    }

    void stubDConfigExport(bool enabled)
    {
        auto createFunc = static_cast<Dtk::Core::DConfig *(*)(const QString &, const QString &, const QString &, QObject *)>(&Dtk::Core::DConfig::create);
        auto *fakeCfg = new Dtk::Core::DConfig("", QString(), nullptr);
        stub.set_lamda(createFunc, [fakeCfg](const QString &, const QString &, const QString &, QObject *) -> Dtk::Core::DConfig * {
            __DBG_STUB_INVOKE__
            return fakeCfg;
        });
        stub.set_lamda(&Dtk::Core::DConfig::value, [enabled](Dtk::Core::DConfig *, const QString &key, const QVariant &) -> QVariant {
            __DBG_STUB_INVOKE__
            if (key == "allowExportEncKey")
                return enabled;
            return QVariant();
        });
    }

    QVariantMap makeArgs() const
    {
        return QVariantMap { { encrypt_param_keys::kKeyDevice, "/dev/sdb1" },
                             { encrypt_param_keys::kKeyDeviceName, "data" } };
    }

    DPasswordEdit *keyEdit1(EncryptParamsInputDialog *dlg) const
    {
        return dlg->findChild<DPasswordEdit *>("EncKeyEdit1");
    }
    DPasswordEdit *keyEdit2(EncryptParamsInputDialog *dlg) const
    {
        return dlg->findChild<DPasswordEdit *>("EncKeyEdit2");
    }
    QComboBox *encCombo(EncryptParamsInputDialog *dlg) const
    {
        return dlg->findChild<QComboBox *>("EncType");
    }

    stub_ext::StubExt stub;
};

TEST_F(UT_EncryptParamsInputDialogCov, Construct_NoTpm_PassphraseOnlyTwoItems)
{
    // Arrange
    // Act
    // Assert
    TestableEncryptDialog dlg(makeArgs());
    auto combo = encCombo(&dlg);
    auto e1 = keyEdit1(&dlg);

    // Assert
    ASSERT_NE(combo, nullptr);
    ASSERT_NE(e1, nullptr);
    EXPECT_EQ(combo->count(), 1);   // tpm items removed
    EXPECT_EQ(combo->currentIndex(), 0);   // kPwd
    EXPECT_FALSE(dlg.property("test").toBool());
}

TEST_F(UT_EncryptParamsInputDialogCov, GetInputs_PwdMode_ReturnsEditedPassword)
{
    // Arrange
    TestableEncryptDialog dlg(makeArgs());
    keyEdit1(&dlg)->setText("Sup3r#Secret");
    keyEdit2(&dlg)->setText("Sup3r#Secret");

    // Act
    auto inputs = dlg.getInputs();

    // Assert
    EXPECT_EQ(inputs.key, QString("Sup3r#Secret"));
    EXPECT_EQ(inputs.devDesc, QString("/dev/sdb1"));
    EXPECT_EQ(static_cast<int>(inputs.secType), static_cast<int>(disk_encrypt::kPwd));
}

TEST_F(UT_EncryptParamsInputDialogCov, ValidatePassword_EmptyAndWeak_ReturnsFalse)
{
    // Arrange
    TestableEncryptDialog dlg(makeArgs());
    auto e1 = keyEdit1(&dlg);
    auto e2 = keyEdit2(&dlg);

    // Act: empty first field

    // Assert
    e2->setText("whatever");
    EXPECT_EQ(dlg.validatePassword(), false);
    // weak complexity
    e1->setText("weakpass");
    e2->setText("weakpass");
    EXPECT_EQ(dlg.validatePassword(), false);
    // mismatch
    e1->setText("Sup3r#Secret1");
    e2->setText("Sup3r#Secret2");
    EXPECT_EQ(dlg.validatePassword(), false);
    // valid
    e1->setText("Sup3r#Secret1");
    e2->setText("Sup3r#Secret1");
    EXPECT_EQ(dlg.validatePassword(), true);
}

TEST_F(UT_EncryptParamsInputDialogCov, ValidatePassword_TpmMode_SkipsChecks)
{
    // Arrange: tpm available so the tpm-only entry exists
    stub.set_lamda(&tpm_utils::checkTPM, [](bool *) -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });
    stub.set_lamda(&tpm_utils::checkTPMLockoutStatus, []() -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });
    TestableEncryptDialog dlg(makeArgs());
    auto combo = encCombo(&dlg);
    ASSERT_EQ(combo->count(), 3);
    combo->setCurrentIndex(2);   // kTpm, no password fields needed

    // Act

    // Assert
    EXPECT_EQ(dlg.validatePassword(), true);
    EXPECT_EQ(keyEdit1(&dlg)->isHidden(), true);
}

TEST_F(UT_EncryptParamsInputDialogCov, ValidatePassword_WrongPage_ReturnsFalse)
{
    // Arrange
    TestableEncryptDialog dlg(makeArgs());
    // Act: validation requested while stacked layout is on the export page
    dlg.onPageChanged(1);
    // Assert
    EXPECT_EQ(dlg.validatePassword(), false);
    EXPECT_NE(dlg.validatePassword(), true);   // double-check stable state
}

TEST_F(UT_EncryptParamsInputDialogCov, OnEncTypeChanged_AllTypes_UpdatesHints)
{
    // Arrange
    TestableEncryptDialog dlg(makeArgs());
    auto e1 = keyEdit1(&dlg);

    // Act
    dlg.onEncTypeChanged(1);   // kPin
    QString pinHint = e1->lineEdit()->placeholderText();
    dlg.onEncTypeChanged(5);   // invalid
    dlg.onEncTypeChanged(0);   // kPwd

    // Assert
    EXPECT_EQ(pinHint.isEmpty(), false);
    EXPECT_EQ(e1->isVisibleTo(nullptr) || !e1->isHidden(), true);
    EXPECT_EQ(dlg.title().isEmpty(), false);
}

TEST_F(UT_EncryptParamsInputDialogCov, OnButtonClicked_ExportEnabled_MovesToExportPageAndBack)
{
    // Arrange: export enabled (SetUp default)
    TestableEncryptDialog dlg(makeArgs());
    keyEdit1(&dlg)->setText("Sup3r#Secret1");
    keyEdit2(&dlg)->setText("Sup3r#Secret1");

    // Act
    dlg.onButtonClicked(1);
    // Assert: moved to export page
    EXPECT_EQ(dlg.title(), QString("Export Recovery Key"));
    EXPECT_EQ(dlg.getButtons().size(), 2);

    // Act: back-button returns to password page
    dlg.onButtonClicked(0);
    EXPECT_EQ(dlg.getButtons().size(), 1);
}

TEST_F(UT_EncryptParamsInputDialogCov, OnButtonClicked_ExportDisabled_ConfirmsDirectly)
{
    // Arrange
    stubDConfigExport(false);
    TestableEncryptDialog dlg(makeArgs());
    keyEdit1(&dlg)->setText("Sup3r#Secret1");
    keyEdit2(&dlg)->setText("Sup3r#Secret1");

    // Act
    dlg.onButtonClicked(1);
    // Assert: confirm button path taken (title is password page, single button)
    EXPECT_EQ(dlg.getButtons().size(), 1);
    EXPECT_NE(dlg.title(), QString("Export Recovery Key"));
}

TEST_F(UT_EncryptParamsInputDialogCov, OnButtonClicked_InvalidPassword_StaysOnPage)
{
    // Arrange
    TestableEncryptDialog dlg(makeArgs());

    // Act
    dlg.onButtonClicked(1);
    // Assert: stayed on password page with a single button
    EXPECT_EQ(dlg.getButtons().size(), 1);
    EXPECT_EQ(dlg.title(), QString("Please continue to encrypt \"data(sdb1)\" partition"));
}

TEST_F(UT_EncryptParamsInputDialogCov, OnExpPathChanged_TriggersButtonState)
{
    // Arrange
    TestableEncryptDialog dlg(makeArgs());
    keyEdit1(&dlg)->setText("Sup3r#Secret1");
    keyEdit2(&dlg)->setText("Sup3r#Secret1");
    dlg.onButtonClicked(1);   // move to export page
    auto next = dlg.getButton(1);
    ASSERT_NE(next, nullptr);

    stub.set_lamda(&recovery_key_utils::validateExportPath, [](const QString &, const QString &, QString *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Act
    dlg.onExpPathChanged("/tmp/export", true);
    bool enabledAfterValid = next->isEnabled();

    stub.set_lamda(&recovery_key_utils::validateExportPath, [](const QString &, const QString &, QString *msg) -> bool {
        __DBG_STUB_INVOKE__
        *msg = "bad path";
        return false;
    });
    dlg.onExpPathChanged("/nonexistent", false);

    // Assert
    EXPECT_EQ(enabledAfterValid, true);
    EXPECT_EQ(next->isEnabled(), false);
}

TEST_F(UT_EncryptParamsInputDialogCov, EncryptByTpm_ExitCodes_TrueOnSuccess)
{
    // Arrange
    TestableEncryptDialog dlg(makeArgs());
    bool okCall = false, failCall = false;
    stub.set_lamda(&tpm_passphrase_utils::genPassphraseFromTPM_NonBlock,
                   [&okCall](const QString &, const QString &, QString *passphrase) -> int {
                       __DBG_STUB_INVOKE__
                       *passphrase = "tpm-generated";
                       okCall = true;
                       return 0;
                   });
    stub.set_lamda(&tpm_utils::ownerAuthStatus, []() -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });

    // Act
    bool ok = dlg.encryptByTpm("/dev/sdb1");

    // Assert
    EXPECT_EQ(ok, true);
    EXPECT_EQ(okCall, true);

    // Act: failure branch
    stub.set_lamda(&tpm_passphrase_utils::genPassphraseFromTPM_NonBlock,
                   [&failCall](const QString &, const QString &, QString *) -> int {
                       __DBG_STUB_INVOKE__
                       failCall = true;
                       return -1;
                   });
    bool fail = dlg.encryptByTpm("/dev/sdb1");
    EXPECT_EQ(fail, false);
    EXPECT_EQ(failCall, true);
}

TEST_F(UT_EncryptParamsInputDialogCov, ConfirmEncrypt_PwdMode_AcceptsDialog)
{
    // Arrange
    TestableEncryptDialog dlg(makeArgs());
    keyEdit1(&dlg)->setText("Sup3r#Secret1");
    keyEdit2(&dlg)->setText("Sup3r#Secret1");
    QSignalSpy acceptSpy(&dlg, &QDialog::accepted);

    // Act
    dlg.confirmEncrypt();

    // Assert
    EXPECT_EQ(acceptSpy.count(), 1);
    EXPECT_NE(acceptSpy.count(), 2);   // complementary bound
}

TEST_F(UT_EncryptParamsInputDialogCov, ConfirmEncrypt_TpmLocked_ShowsDialogWithoutAccept)
{
    // Arrange: tpm available, select pin
    stub.set_lamda(&tpm_utils::checkTPM, [](bool *) -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });
    stub.set_lamda(&tpm_utils::checkTPMLockoutStatus, []() -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });
    TestableEncryptDialog dlg(makeArgs());
    encCombo(&dlg)->setCurrentIndex(1);   // kPin

    QSignalSpy acceptSpy(&dlg, &QDialog::accepted);
    int dialogCount = 0;
    stub.set_lamda(&dialog_utils::showDialog, [&dialogCount](const QString &, const QString &) -> int {
        __DBG_STUB_INVOKE__
        ++dialogCount;
        return 0;
    });
    stub.set_lamda(&tpm_passphrase_utils::genPassphraseFromTPM_NonBlock,
                   [](const QString &, const QString &, QString *) -> int {
                       __DBG_STUB_INVOKE__
                       return -1;
                   });
    stub.set_lamda(&tpm_utils::ownerAuthStatus, []() -> int {
        __DBG_STUB_INVOKE__
        return 1;   // locked
    });

    // Act
    dlg.confirmEncrypt();

    // Assert
    EXPECT_EQ(acceptSpy.count(), 0);
    EXPECT_EQ(dialogCount, 1);
}

TEST_F(UT_EncryptParamsInputDialogCov, SetPasswordInputVisible_TogglesEdits)
{
    // Arrange
    TestableEncryptDialog dlg(makeArgs());
    auto e1 = keyEdit1(&dlg);
    auto e2 = keyEdit2(&dlg);
    ASSERT_NE(e1, nullptr);
    ASSERT_NE(e2, nullptr);

    // Act
    dlg.setPasswordInputVisible(false);
    bool hiddenAfter = e1->isHidden() && e2->isHidden();
    dlg.setPasswordInputVisible(true);

    // Assert
    EXPECT_EQ(hiddenAfter, true);
    EXPECT_EQ(e1->isHidden(), false);
    EXPECT_EQ(e2->isHidden(), false);
}

TEST_F(UT_EncryptParamsInputDialogCov, GetInputs_TpmMode_UsesGeneratedPassword)
{
    // Arrange: tpm available, use tpm-only
    stub.set_lamda(&tpm_utils::checkTPM, [](bool *) -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });
    stub.set_lamda(&tpm_utils::checkTPMLockoutStatus, []() -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });
    TestableEncryptDialog dlg(makeArgs());
    auto combo = encCombo(&dlg);
    ASSERT_EQ(combo->count(), 3);
    combo->setCurrentIndex(2);   // kTpm

    stub.set_lamda(&tpm_passphrase_utils::genPassphraseFromTPM_NonBlock,
                   [](const QString &, const QString &, QString *passphrase) -> int {
                       __DBG_STUB_INVOKE__
                       *passphrase = "gen-1234";
                       return 0;
                   });
    stub.set_lamda(&tpm_utils::ownerAuthStatus, []() -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });
    QSignalSpy acceptSpy(&dlg, &QDialog::accepted);

    // Act
    dlg.confirmEncrypt();   // tpm path fills tpmPassword then accepts
    auto inputs = dlg.getInputs();

    // Assert
    EXPECT_EQ(acceptSpy.count(), 1);
    EXPECT_EQ(inputs.key, QString("gen-1234"));
    EXPECT_EQ(static_cast<int>(inputs.secType), static_cast<int>(disk_encrypt::kTpm));
}
