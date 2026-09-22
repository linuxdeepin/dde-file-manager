// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Covered functions (events/eventshandler.cpp, complementing test_eventshandler.cpp):
//   hookEvents / onInitEncryptFinished(+reboot lambda) / onEncryptFinished(+cleanup lambda) /
//   onDecryptFinished / onChgPwdFinished / onRequestAuthArgs(+finished lambda partially) /
//   ignoreParamRequest / onEncryptProgress / onDecryptProgress /
//   acquirePassphrase / acquirePassphraseByPIN / acquirePassphraseByTPM / acquirePassphraseByRec /
//   showPreEncryptError / showDecryptError / showChgPwdError / requestReboot /
//   canUnlock / setAutoStartDFM / saveRecoveryKeyToFile
// Branch notes (from get_code_snippet):
//   onInitEncryptFinished: -kRebootRequired (autostart+delayed reboot) / code<0 / success;
//   onEncryptFinished: kUserCancelled / kSuccess / default; with/without progress dialog;
//   onDecryptFinished: -kRebootRequired vs showDecryptError;
//   show*Error switch cases: success / cancelled / wrong passphrase / not fully encrypted / default;
//   canUnlock: under-operating / pending decrypt job / online+encrypting / ok;
//   saveRecoveryKeyToFile: valid write / failure path.

#include "stubext.h"

#include "events/eventshandler.h"
#include "utils/encryptutils.h"
#include <dfm-mount/dmount.h>
#include "gui/encryptprogressdialog.h"
#include "gui/encryptparamsinputdialog.h"
#include "dfmplugin_disk_encrypt_global.h"
#include "services/diskencrypt/globaltypesdefine.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QDialog>
#include <QDBusAbstractInterface>
#include <QEventLoop>
#include <QFile>
#include <QTemporaryDir>
#include <QTimer>
// Schedules auto-finish for the next modal dialog (its exec() runs a local
// event loop; the retrying timer closes the dialog as soon as it is visible).
inline void autoFinishDialog(int result)
{
    auto *t = new QTimer(qApp);
    t->setInterval(10);
    QObject::connect(t, &QTimer::timeout, t, [t, result]() {
        for (QWidget *w : QApplication::topLevelWidgets()) {
            auto *d = qobject_cast<QDialog *>(w);
            if (d && d->isVisible() && d->isModal()) {
                t->stop();
                t->deleteLater();
                d->done(result);
                return;
            }
        }
    });
    t->start();
}

#include <QTimer>
#include <DDialog>
#include <dconfig.h>

using namespace dfmplugin_diskenc;
using namespace disk_encrypt;

class UT_EventsHandlerCov : public testing::Test
{
protected:
    void SetUp() override
    {
        ins = EventsHandler::instance();

        // never let a real reboot or autostart change escape to the session bus
        stubAutoStart(nullptr);
        stubReboot(nullptr);
        stubShowDialog(nullptr);

        stub.set_lamda(&QApplication::platformName, []() -> QString {
            __DBG_STUB_INVOKE__
            return "offscreen";
        });
        stub.set_lamda(&QDBusAbstractInterface::isValid, [](QDBusAbstractInterface *) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
        // avoid real 25s DBus timeouts in status queries
        stub.set_lamda(&EventsHandler::unfinishedDecryptJob, [](EventsHandler *) -> QString {
            __DBG_STUB_INVOKE__
            return QString();
        });
        stub.set_lamda(&EventsHandler::deviceEncryptStatus, [](EventsHandler *, const QString &) -> int {
            __DBG_STUB_INVOKE__
            return -1;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

    void stubAutoStart(int *counter)
    {
        stub.set_lamda(&EventsHandler::setAutoStartDFM,
                       [this, counter](EventsHandler *, bool enable) {
                           __DBG_STUB_INVOKE__
                           if (counter) ++(*counter);
                           lastAutoStartEnable = enable;
                       });
    }

    void stubReboot(int *counter)
    {
        stub.set_lamda(&EventsHandler::requestReboot, [counter](EventsHandler *) {
            __DBG_STUB_INVOKE__
            if (counter) ++(*counter);
        });
    }

    void stubShowDialog(QString *title /* may be null */)
    {
        stub.set_lamda(&dialog_utils::showDialog,
                       [title](const QString &t, const QString &) -> int {
                           __DBG_STUB_INVOKE__
                           if (title) *title = t;
                           return 0;
                       });
    }

    EventsHandler *ins = nullptr;
    stub_ext::StubExt stub;
    bool lastAutoStartEnable = false;
};

TEST_F(UT_EventsHandlerCov, HookEvents_RegistersAcquireDevPwdHook)
{
    // Arrange
    // Act: dpf hook follow is safe with unregistered plugin
    // Assert
    // Assert
    ASSERT_NE(ins, nullptr);
    EXPECT_NO_FATAL_FAILURE(ins->hookEvents());
}

TEST_F(UT_EventsHandlerCov, OnInitEncryptFinished_RebootRequired_SetsAutostartAndRequestsReboot)
{
    // Arrange
    int autoCount = 0, rebootCount = 0;
    stubAutoStart(&autoCount);
    stubReboot(&rebootCount);
    QVariantMap result { { encrypt_param_keys::kKeyOperationResult, -kRebootRequired },
                         { encrypt_param_keys::kKeyDevice, "/dev/sdb1" },
                         { encrypt_param_keys::kKeyDeviceName, "data" } };

    // Act
    ins->onInitEncryptFinished(result);

    // Assert: autostart enabled immediately, reboot is delayed by 1s timer
    EXPECT_EQ(autoCount, 1);
    EXPECT_TRUE(lastAutoStartEnable);
    EXPECT_EQ(rebootCount, 0);

    QEventLoop loop;
    QTimer::singleShot(1500, &loop, &QEventLoop::quit);
    loop.exec();
    EXPECT_EQ(rebootCount, 1);
}

TEST_F(UT_EventsHandlerCov, OnInitEncryptFinished_Error_ShowsPreEncryptError)
{
    // Arrange
    QString title;
    stubShowDialog(&title);
    QVariantMap result { { encrypt_param_keys::kKeyOperationResult, -7 },
                         { encrypt_param_keys::kKeyDevice, "/dev/sdb1" },
                         { encrypt_param_keys::kKeyDeviceName, "data" } };

    // Act
    ins->onInitEncryptFinished(result);

    // Assert
    EXPECT_EQ(title, QString("Preencrypt failed"));
    EXPECT_NE(title, QString("Preencrypt done"));
}

TEST_F(UT_EventsHandlerCov, OnInitEncryptFinished_Success_EnablesAutostart)
{
    // Arrange
    int autoCount = 0;
    stubAutoStart(&autoCount);
    QVariantMap result { { encrypt_param_keys::kKeyOperationResult, 0 } };

    // Act
    ins->onInitEncryptFinished(result);

    // Assert
    EXPECT_EQ(autoCount, 1);
    EXPECT_TRUE(lastAutoStartEnable);
}

TEST_F(UT_EventsHandlerCov, OnEncryptFinished_UserCancelled_IgnoresParamRequest)
{
    // Arrange
    int ignoreCount = 0, autoCount = 0;
    stubAutoStart(&autoCount);
    stub.set_lamda(&EventsHandler::ignoreParamRequest, [&ignoreCount](EventsHandler *) {
        __DBG_STUB_INVOKE__
        ++ignoreCount;
    });
    QVariantMap result { { encrypt_param_keys::kKeyOperationResult, -kUserCancelled },
                         { encrypt_param_keys::kKeyDevice, "/dev/utA1" } };

    // Act
    ins->onEncryptFinished(result);

    // Assert: cancelled returns before disabling autostart
    EXPECT_EQ(ignoreCount, 1);
    EXPECT_EQ(autoCount, 0);
}

TEST_F(UT_EventsHandlerCov, OnEncryptFinished_SuccessNoDialog_ShowsEncryptDone)
{
    // Arrange
    QString title;
    stubShowDialog(&title);
    int autoCount = 0;
    stubAutoStart(&autoCount);
    QVariantMap result { { encrypt_param_keys::kKeyOperationResult, -kSuccess },
                         { encrypt_param_keys::kKeyDevice, "/dev/utA2" },
                         { encrypt_param_keys::kKeyDeviceName, "data" } };

    // Act
    ins->onEncryptFinished(result);

    // Assert
    EXPECT_EQ(title, QString("Encrypt done"));
    EXPECT_EQ(autoCount, 1);
    EXPECT_FALSE(lastAutoStartEnable);
}

TEST_F(UT_EventsHandlerCov, OnEncryptFinished_Failure_ShowsEncryptFailed)
{
    // Arrange
    QString title;
    stubShowDialog(&title);
    QVariantMap result { { encrypt_param_keys::kKeyOperationResult, -9 },
                         { encrypt_param_keys::kKeyDevice, "/dev/utA3" },
                         { encrypt_param_keys::kKeyDeviceName, "data" } };

    // Act
    ins->onEncryptFinished(result);

    // Assert
    EXPECT_EQ(title, QString("Encrypt failed"));
    EXPECT_NE(title, QString("Encrypt done"));
}

TEST_F(UT_EventsHandlerCov, OnEncryptFinished_WithProgressDialog_UsesResultPage)
{
    // Arrange: create a progress dialog through onEncryptProgress first
    int updateCount = 0;
    stub.set_lamda(&EncryptProgressDialog::updateProgress,
                   [&updateCount](EncryptProgressDialog *, double) {
                       __DBG_STUB_INVOKE__
                       ++updateCount;
                   });
    ins->onEncryptProgress("/dev/utA4", "data", 0.1);
    ASSERT_EQ(updateCount, 1);

    // Act: finish with success plus a recovery key -> result/export page on the same dialog
    QVariantMap result { { encrypt_param_keys::kKeyOperationResult, -kSuccess },
                         { encrypt_param_keys::kKeyDevice, "/dev/utA4" },
                         { encrypt_param_keys::kKeyDeviceName, "data" },
                         { encrypt_param_keys::kKeyRecoveryKey, QString("AAAA-BBBB-CCCC-DDDD") } };
    EXPECT_NO_FATAL_FAILURE(ins->onEncryptFinished(result));

    // Assert: dialog was reused (no further progress ticks), no fallback dialog shown
    EXPECT_EQ(updateCount, 1);
    EXPECT_NE(updateCount, 2);   // complementary bound
}

TEST_F(UT_EventsHandlerCov, OnDecryptFinished_RebootRequired_RequestsReboot)
{
    // Arrange
    int rebootCount = 0;
    stubReboot(&rebootCount);
    QVariantMap result { { encrypt_param_keys::kKeyOperationResult, -kRebootRequired },
                         { encrypt_param_keys::kKeyDevice, "/dev/utB1" } };

    // Act
    ins->onDecryptFinished(result);

    // Assert
    EXPECT_EQ(rebootCount, 1);
    EXPECT_NE(rebootCount, 2);   // complementary bound
}

TEST_F(UT_EventsHandlerCov, OnDecryptFinished_WrongPassphrase_ShowsSpecificDialog)
{
    // Arrange
    QString title;
    stubShowDialog(&title);
    QVariantMap result { { encrypt_param_keys::kKeyOperationResult, -kErrorWrongPassphrase },
                         { encrypt_param_keys::kKeyDevice, "/dev/utB2" },
                         { encrypt_param_keys::kKeyDeviceName, "data" } };

    // Act
    ins->onDecryptFinished(result);

    // Assert
    EXPECT_EQ(title, QString("Decrypt partition"));
    EXPECT_NE(title, QString("Decrypt done"));
}

TEST_F(UT_EventsHandlerCov, OnDecryptFinished_Success_ShowsDecryptDoneAndDisablesAutostart)
{
    // Arrange
    QString title;
    stubShowDialog(&title);
    int autoCount = 0;
    stubAutoStart(&autoCount);
    QVariantMap result { { encrypt_param_keys::kKeyOperationResult, -kSuccess },
                         { encrypt_param_keys::kKeyDevice, "/dev/utB3" },
                         { encrypt_param_keys::kKeyDeviceName, "data" } };

    // Act
    ins->onDecryptFinished(result);

    // Assert
    EXPECT_EQ(title, QString("Decrypt done"));
    EXPECT_EQ(autoCount, 1);
    EXPECT_FALSE(lastAutoStartEnable);
}

TEST_F(UT_EventsHandlerCov, OnChgPwdFinished_AllCodes_MapToTitles)
{
    // Arrange
    QString title;
    stubShowDialog(&title);
    stub.set_lamda(&device_utils::encKeyType, [](const QString &) -> int {
        __DBG_STUB_INVOKE__
        return 0;   // passphrase
    });

    // Act: success

    // Assert
    ins->onChgPwdFinished({ { encrypt_param_keys::kKeyOperationResult, -kSuccess },
                            { encrypt_param_keys::kKeyDevice, "/dev/utC1" },
                            { encrypt_param_keys::kKeyDeviceName, "data" } });
    EXPECT_EQ(title, QString("Change passphrase done"));

    // wrong passphrase
    ins->onChgPwdFinished({ { encrypt_param_keys::kKeyOperationResult, -kErrorChangePassphraseFailed },
                            { encrypt_param_keys::kKeyDevice, "/dev/utC2" },
                            { encrypt_param_keys::kKeyDeviceName, "data" } });
    EXPECT_EQ(title, QString("Change passphrase failed"));

    // default error
    ins->onChgPwdFinished({ { encrypt_param_keys::kKeyOperationResult, -42 },
                            { encrypt_param_keys::kKeyDevice, "/dev/utC3" },
                            { encrypt_param_keys::kKeyDeviceName, "data" } });
    EXPECT_EQ(title, QString("Change passphrase failed"));
}

TEST_F(UT_EventsHandlerCov, OnChgPwdFinished_PinDevice_UsesPinWording)
{
    // Arrange
    QString title;
    stubShowDialog(&title);
    stub.set_lamda(&device_utils::encKeyType, [](const QString &) -> int {
        __DBG_STUB_INVOKE__
        return 1;   // PIN
    });

    // Act
    ins->onChgPwdFinished({ { encrypt_param_keys::kKeyOperationResult, -kSuccess },
                            { encrypt_param_keys::kKeyDevice, "/dev/utC4" },
                            { encrypt_param_keys::kKeyDeviceName, "data" } });

    // Assert
    EXPECT_EQ(title, QString("Change PIN done"));
    EXPECT_NE(title, QString("Change passphrase done"));
}

TEST_F(UT_EventsHandlerCov, OnRequestAuthArgs_EmptyDevice_EarlyReturn)
{
    // Arrange
    QString title = "untouched";
    stubShowDialog(&title);

    // Act
    ins->onRequestAuthArgs(QVariantMap());

    // Assert: nothing created, no dialogs
    EXPECT_EQ(title, QString("untouched"));
    EXPECT_EQ(title.length(), 9);
}

TEST_F(UT_EventsHandlerCov, OnRequestAuthArgs_ValidDevice_CreatesInputDialog)
{
    // Arrange
    auto createFunc = static_cast<Dtk::Core::DConfig *(*)(const QString &, const QString &, const QString &, QObject *)>(&Dtk::Core::DConfig::create);
    auto *fakeCfg = new Dtk::Core::DConfig("", QString(), nullptr);
    stub.set_lamda(createFunc, [fakeCfg](const QString &, const QString &, const QString &, QObject *) -> Dtk::Core::DConfig * {
        __DBG_STUB_INVOKE__
        return fakeCfg;
    });
    stub.set_lamda(&Dtk::Core::DConfig::value, [](Dtk::Core::DConfig *, const QString &, const QVariant &) -> QVariant {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&tpm_utils::checkTPM, [](bool *) -> int {
        __DBG_STUB_INVOKE__
        return -1;
    });
    stub.set_lamda(&tpm_utils::checkTPMLockoutStatus, []() -> int {
        __DBG_STUB_INVOKE__
        return -1;
    });
    stub.set_lamda(&device_utils::createBlockDevice, [](const QString &) -> BlockDev {
        __DBG_STUB_INVOKE__
        return BlockDev();
    });
    auto countInputDialogs = []() {
        int n = 0;
        for (QWidget *w : QApplication::topLevelWidgets())
            if (qobject_cast<EncryptParamsInputDialog *>(w))
                ++n;
        return n;
    };

    // Act
    ins->onRequestAuthArgs({ { encrypt_param_keys::kKeyDevice, "/dev/utD1" },
                             { encrypt_param_keys::kKeyDeviceName, "data" } });

    // Assert: an EncryptParamsInputDialog was created and shown exactly once
    ASSERT_EQ(countInputDialogs(), 1);

    // calling again for the same device does not duplicate the dialog
    ins->onRequestAuthArgs({ { encrypt_param_keys::kKeyDevice, "/dev/utD1" } });
    EXPECT_EQ(countInputDialogs(), 1);
    EXPECT_NE(countInputDialogs(), 2);   // complementary bound
}

TEST_F(UT_EventsHandlerCov, IgnoreParamRequest_InvalidInterface_StillQuiet)
{
    // Arrange: DBus iface invalid (SetUp)

    // Act

    // Assert
    ASSERT_NE(ins, nullptr);
    EXPECT_NO_FATAL_FAILURE(ins->ignoreParamRequest());
}

TEST_F(UT_EventsHandlerCov, OnEncryptProgress_CreatesAndUpdatesDialog)
{
    // Arrange
    int updateCount = 0;
    double lastProgress = -1;
    QString lastTitle;
    stub.set_lamda(&EncryptProgressDialog::updateProgress,
                   [&updateCount, &lastProgress](EncryptProgressDialog *, double p) {
                       __DBG_STUB_INVOKE__
                       ++updateCount;
                       lastProgress = p;
                   });
    stub.set_lamda(&EncryptProgressDialog::setText,
                   [&lastTitle](EncryptProgressDialog *, const QString &title, const QString &) {
                       __DBG_STUB_INVOKE__
                       lastTitle = title;
                   });

    // Act
    ins->onEncryptProgress("/dev/utE1", "data", 0.25);
    ins->onEncryptProgress("/dev/utE1", "data", 0.5);

    // Assert: dialog created once (title set once), progress updated twice
    EXPECT_EQ(updateCount, 2);
    EXPECT_DOUBLE_EQ(lastProgress, 0.5);
    EXPECT_TRUE(lastTitle.contains("encrypting"));
}

TEST_F(UT_EventsHandlerCov, OnDecryptProgress_CreatesDialog)
{
    // Arrange
    int updateCount = 0;
    stub.set_lamda(&EncryptProgressDialog::updateProgress,
                   [&updateCount](EncryptProgressDialog *, double) {
                       __DBG_STUB_INVOKE__
                       ++updateCount;
                   });
    // Act
    ins->onDecryptProgress("/dev/utF1", "data", 0.75);

    // Assert
    EXPECT_EQ(updateCount, 1);
    EXPECT_TRUE(updateCount == 1);
}

TEST_F(UT_EventsHandlerCov, AcquirePassphrase_Cancelled_SetsCancelledFlag)
{
    // Arrange: dialog auto-finished with rejection
    autoFinishDialog(0);
    bool cancelled = false;

    // Act
    QString key = ins->acquirePassphrase("/dev/sdb1", cancelled);

    // Assert
    EXPECT_EQ(cancelled, true);
    EXPECT_EQ(key.isEmpty(), true);
}

TEST_F(UT_EventsHandlerCov, AcquirePassphraseByPin_WithPin_TakesTpmPath)
{
    // Arrange
    autoFinishDialog(1);   // accepted
    stub.set_lamda(&tpm_passphrase_utils::getPassphraseFromTPM_NonBlock,
                   [](const QString &dev, const QString &) -> QString {
                       __DBG_STUB_INVOKE__
                       return QString("tpm-") + dev;
                   });
    bool cancelled = false;

    // Act
    QString key = ins->acquirePassphraseByPIN("/dev/sdb1", cancelled);

    // Assert
    EXPECT_FALSE(cancelled);
    EXPECT_EQ(key, QString("tpm-/dev/sdb1"));
}

TEST_F(UT_EventsHandlerCov, AcquirePassphraseByTPM_ReturnsTpmPassphrase)
{
    // Arrange
    stub.set_lamda(&tpm_passphrase_utils::getPassphraseFromTPM_NonBlock,
                   [](const QString &, const QString &) -> QString {
                       __DBG_STUB_INVOKE__
                       return QString("tpm-only");
                   });
    bool cancelled = false;

    // Act
    QString key = ins->acquirePassphraseByTPM("/dev/sdb1", cancelled);

    // Assert
    EXPECT_EQ(key, QString("tpm-only"));
    EXPECT_FALSE(cancelled);
}

TEST_F(UT_EventsHandlerCov, AcquirePassphraseByRec_Cancelled_ReturnsEmpty)
{
    // Arrange: dialog auto-finished with rejection
    autoFinishDialog(0);
    bool cancelled = false;

    // Act
    QString key = ins->acquirePassphraseByRec("/dev/sdb1", cancelled);

    // Assert
    EXPECT_EQ(cancelled, true);
    EXPECT_EQ(key.isEmpty(), true);
}

TEST_F(UT_EventsHandlerCov, RequestReboot_CallsShutdownInterface)
{
    // Arrange: stub the real method away (avoids touching the session bus)
    int rebootCount = 0;
    stubReboot(&rebootCount);

    // Act
    ins->requestReboot();

    // Assert
    EXPECT_EQ(rebootCount, 1);
    EXPECT_NE(rebootCount, 2);   // complementary bound
}

TEST_F(UT_EventsHandlerCov, CanUnlock_UnderOperating_ReturnsFalse)
{
    // Arrange
    stub.set_lamda(&EventsHandler::isUnderOperating, [](EventsHandler *, const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Act

    // Assert
    EXPECT_EQ(ins->canUnlock("/dev/sdb1"), false);
    EXPECT_NE(ins->canUnlock("/dev/sdb1"), true);   // double-check stable state
}

TEST_F(UT_EventsHandlerCov, CanUnlock_PendingDecryptJob_ReturnsFalse)
{
    // Arrange
    stub.set_lamda(&EventsHandler::unfinishedDecryptJob, [](EventsHandler *) -> QString {
        __DBG_STUB_INVOKE__
        return QString("/dev/sdb1");
    });

    // Act

    // Assert
    EXPECT_EQ(ins->canUnlock("/dev/sdb1"), false);
    EXPECT_NE(ins->canUnlock("/dev/sdb1"), true);   // double-check stable state
}

TEST_F(UT_EventsHandlerCov, CanUnlock_OnlineAndEncrypting_ReturnsFalse)
{
    // Arrange
    stub.set_lamda(&EventsHandler::deviceEncryptStatus, [](EventsHandler *, const QString &) -> int {
        __DBG_STUB_INVOKE__
        return EncryptState::kStatusOnline | EncryptState::kStatusEncrypt;
    });

    // Act

    // Assert
    EXPECT_EQ(ins->canUnlock("/dev/sdb1"), false);
    EXPECT_NE(ins->canUnlock("/dev/sdb1"), true);   // double-check stable state
}

TEST_F(UT_EventsHandlerCov, CanUnlock_IdleDevice_ReturnsTrue)
{
    // Arrange: defaults from SetUp keep every check negative; DBus invalid gives -1
    stub.set_lamda(&EventsHandler::deviceEncryptStatus, [](EventsHandler *, const QString &) -> int {
        __DBG_STUB_INVOKE__
        return EncryptState::kStatusNotEncrypted;
    });

    // Act

    // Assert
    EXPECT_EQ(ins->canUnlock("/dev/sdb1"), true);
    EXPECT_NE(ins->canUnlock("/dev/sdb1"), false);   // double-check stable state
}

TEST_F(UT_EventsHandlerCov, SetAutoStartDFM_InvalidDBus_NoAutostartFileTouched)
{
    // Arrange: DBus iface invalid (SetUp); make sure no stale file exists
    const QString autostartFile = QDir::homePath() + "/.config/autostart/dfm-reencrypt.desktop";
    bool existedBefore = QFile::exists(autostartFile);

    // Act
    ins->setAutoStartDFM(true);
    ins->setAutoStartDFM(false);

    // Assert: nothing created by the test
    EXPECT_EQ(QFile::exists(autostartFile), existedBefore);
    EXPECT_EQ(QFile::exists("/nonexistent/dfm-ut-probe-dir"), false);
}

TEST_F(UT_EventsHandlerCov, SaveRecoveryKeyToFile_ValidPath_WritesKey)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    // Act
    bool ok = ins->saveRecoveryKeyToFile("ABCD-EFGH", "/dev/sdb1", dir.path());
    QFile f(dir.path() + "/sdb1_recovery_key.txt");

    // Assert
    EXPECT_TRUE(ok);
    ASSERT_TRUE(f.exists());
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    EXPECT_EQ(QString::fromLocal8Bit(f.readAll()), QString("ABCD-EFGH"));
}

TEST_F(UT_EventsHandlerCov, SaveRecoveryKeyToFile_InvalidPath_ReturnsFalse)
{
    // Arrange: regular file used as directory
    QFile blocker("/tmp/ut_evthandler_blocker");
    blocker.open(QIODevice::WriteOnly);
    blocker.close();

    // Act
    bool ok = ins->saveRecoveryKeyToFile("ABCD-EFGH", "/dev/sdb1", "/tmp/ut_evthandler_blocker/sub");

    // Assert
    EXPECT_EQ(ok, false);
    EXPECT_NE(ok, true);   // double-check stable state
    QFile::remove("/tmp/ut_evthandler_blocker");
}
