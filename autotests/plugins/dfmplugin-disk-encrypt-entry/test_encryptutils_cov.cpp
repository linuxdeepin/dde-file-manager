// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Covered functions (utils/encryptutils.cpp, complementing test_encryptutils.cpp):
//   tpm_utils::checkTPM / checkTPMLockoutStatus / getRandomByTPM / isSupportAlgoByTPM /
//   encryptByTPM / decryptByTPM / ownerAuthStatus;
//   device_utils::encKeyType / cacheToken;
//   tpm_passphrase_utils::genPassphraseFromTPM / getPassphraseFromTPM /
//   getPassphraseFromTPM_NonBlock / genPassphraseFromTPM_NonBlock;
//   dialog_utils::showDialog / showTPMError / showConfirmEncryptionDialog /
//   showConfirmDecryptionDialog;
//   config_utils::useOverlayDMMode / tpmAlgoFromDConfig
// Branch notes (from get_code_snippet):
//   checkTPM: invalid iface / valid reply (ret/-1 auth-failed);
//   isSupportAlgoByTPM: 2-arg reply writes *support;
//   encKeyType: iface invalid / empty token / pin 1 / pin 0;
//   cacheToken: empty token removes dir / full token writes 5 files;
//   getPassphraseFromTPM: missing token / token missing keys / decrypt success;
//   genPassphraseFromTPM: no random / full success (pin or pcr);
//   showTPMError: all enum cases (dialog only when message non-empty).

#include "stubext.h"

#include "utils/encryptutils.h"
#include "dfmplugin_disk_encrypt_global.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QDialog>
#include <QDBusAbstractInterface>
#include <QDBusMessage>
#include <QJsonDocument>
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

#include <DDialog>
#include <dconfig.h>

using namespace dfmplugin_diskenc;
DWIDGET_USE_NAMESPACE

class UT_EncryptUtilsCov : public testing::Test
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

    void stubIfaceInvalid()
    {
        stub.set_lamda(&QDBusAbstractInterface::isValid, [](QDBusAbstractInterface *) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
    }

    void stubIfaceCall(const std::function<QVariantList(const QString &)> &maker)
    {
        stub.set_lamda(&QDBusAbstractInterface::isValid, [](QDBusAbstractInterface *) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        using CallFunc = QDBusMessage (QDBusAbstractInterface::*)(const QString &);
        stub.set_lamda(static_cast<CallFunc>(&QDBusAbstractInterface::call),
                       [maker](QDBusAbstractInterface *, const QString &method) -> QDBusMessage {
                           __DBG_STUB_INVOKE__
                           QDBusMessage msg = QDBusMessage::createMethodCall("s", "/p", "i", method);
                           return msg.createReply(QVariantList {} << maker(method));
                       });
        using CallArgsFunc = QDBusMessage (QDBusAbstractInterface::*)(QDBus::CallMode, const QString &, const QVariant *, size_t);
        stub.set_lamda(static_cast<CallArgsFunc>(&QDBusAbstractInterface::doCall),
                       [maker](QDBusAbstractInterface *, QDBus::CallMode, const QString &method, const QVariant *, size_t) -> QDBusMessage {
                           __DBG_STUB_INVOKE__
                           QDBusMessage msg = QDBusMessage::createMethodCall("s", "/p", "i", method);
                           return msg.createReply(QVariantList {} << maker(method));
                       });
    }

    void stubDConfigValue(const std::function<QVariant(const QString &)> &valueOf)
    {
        auto createFunc = static_cast<Dtk::Core::DConfig *(*)(const QString &, const QString &, const QString &, QObject *)>(&Dtk::Core::DConfig::create);
        auto *fakeCfg = new Dtk::Core::DConfig("", QString(), nullptr);
        stub.set_lamda(createFunc, [fakeCfg](const QString &, const QString &, const QString &, QObject *) -> Dtk::Core::DConfig * {
            __DBG_STUB_INVOKE__
            return fakeCfg;
        });
        stub.set_lamda(&Dtk::Core::DConfig::value, [valueOf](Dtk::Core::DConfig *, const QString &key, const QVariant &def) -> QVariant {
            __DBG_STUB_INVOKE__
            return valueOf(key);
        });
    }

    stub_ext::StubExt stub;
};

// ========== tpm_utils DBus wrappers ==========

TEST_F(UT_EncryptUtilsCov, CheckTPM_InvalidInterface_ReturnsMinusOne)
{
    // Arrange
    stubIfaceInvalid();
    bool authFailed = true;

    // Act
    int ret = tpm_utils::checkTPM(&authFailed);

    // Assert
    EXPECT_EQ(ret, -1);
    EXPECT_FALSE(authFailed);
}

TEST_F(UT_EncryptUtilsCov, CheckTPM_ValidReply_MapsAuthFailed)
{
    // Arrange
    stubIfaceCall([](const QString &method) -> QVariantList {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(method, QString("IsTPMAvailable"));
        return { -1 };
    });
    bool authFailed = false;

    // Act
    int ret = tpm_utils::checkTPM(&authFailed);

    // Assert
    EXPECT_EQ(ret, -1);
    EXPECT_TRUE(authFailed);
}

TEST_F(UT_EncryptUtilsCov, CheckTPMLockoutStatus_Branches)
{
    // Arrange: invalid
    stubIfaceInvalid();
    EXPECT_EQ(tpm_utils::checkTPMLockoutStatus(), -1);

    // valid reply
    stubIfaceCall([](const QString &) -> QVariantList { return { 1 }; });
    // Act
    // Assert
    EXPECT_EQ(tpm_utils::checkTPMLockoutStatus(), 1);
}

TEST_F(UT_EncryptUtilsCov, GetRandomByTPM_InvalidInterface_ReturnsMinusOne)
{
    // Arrange
    stubIfaceInvalid();
    QString out;

    // Act
    int ret = tpm_utils::getRandomByTPM(32, &out);

    // Assert
    EXPECT_EQ(ret, -1);
    EXPECT_TRUE(out.isEmpty());
}

TEST_F(UT_EncryptUtilsCov, IsSupportAlgoByTPM_ValidReply_SetsSupport)
{
    // Arrange
    stubIfaceCall([](const QString &) -> QVariantList { return { 0, true }; });
    bool support = false;

    // Act
    int ret = tpm_utils::isSupportAlgoByTPM("rsa", &support);

    // Assert
    EXPECT_EQ(ret, 0);
    EXPECT_TRUE(support);
}

TEST_F(UT_EncryptUtilsCov, IsSupportAlgoByTPM_InvalidInterface_ReturnsMinusOne)
{
    // Arrange
    stubIfaceInvalid();
    bool support = true;

    // Act

    // Assert
    EXPECT_EQ(tpm_utils::isSupportAlgoByTPM("rsa", &support), -1);
    EXPECT_NE(tpm_utils::isSupportAlgoByTPM("rsa", &support), 0);   // complementary bound
}

TEST_F(UT_EncryptUtilsCov, EncryptByTPM_InvalidInterface_ReturnsMinusOne)
{
    // Arrange
    stubIfaceInvalid();

    // Act

    // Assert
    EXPECT_EQ(tpm_utils::encryptByTPM(QVariantMap {}), -1);
    EXPECT_NE(tpm_utils::encryptByTPM(QVariantMap {}), 0);   // complementary bound
}

TEST_F(UT_EncryptUtilsCov, DecryptByTPM_InvalidInterface_ReturnsMinusOne)
{
    // Arrange
    stubIfaceInvalid();
    QString out;

    // Act

    // Assert
    EXPECT_EQ(tpm_utils::decryptByTPM(QVariantMap {}, &out), -1);
    EXPECT_NE(tpm_utils::decryptByTPM(QVariantMap {}, &out), 0);   // complementary bound
}

TEST_F(UT_EncryptUtilsCov, OwnerAuthStatus_Branches)
{
    // Arrange: invalid
    stubIfaceInvalid();
    EXPECT_EQ(tpm_utils::ownerAuthStatus(), -1);

    // valid
    stubIfaceCall([](const QString &) -> QVariantList { return { 1 }; });
    // Act
    // Assert
    EXPECT_EQ(tpm_utils::ownerAuthStatus(), 1);
}

// ========== device_utils ==========

TEST_F(UT_EncryptUtilsCov, EncKeyType_Branches)
{
    // Arrange
    stubIfaceInvalid();
    stub.set_lamda(&device_utils::cacheToken, [](const QString &, const QVariantMap &) {
        __DBG_STUB_INVOKE__
    });

    // Act
    int noIface = device_utils::encKeyType("/dev/sdb1");
    stubIfaceCall([](const QString &) -> QVariantList { return { QString("") }; });
    int emptyToken = device_utils::encKeyType("/dev/sdb1");
    stubIfaceCall([](const QString &) -> QVariantList { return { QString("{\"pin\":\"1\"}") }; });
    int pinOne = device_utils::encKeyType("/dev/sdb1");
    stubIfaceCall([](const QString &) -> QVariantList { return { QString("{\"pin\":\"0\"}") }; });
    int pinZero = device_utils::encKeyType("/dev/sdb1");

    // Assert
    EXPECT_EQ(noIface, 0);
    EXPECT_EQ(emptyToken, 0);
    EXPECT_EQ(pinOne, 1);
    EXPECT_EQ(pinZero, 2);
}

TEST_F(UT_EncryptUtilsCov, CacheToken_EmptyToken_RemovesDeviceDir)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString devDir = dir.path() + "/dev/sdb1";
    QDir().mkpath(devDir);
    stub.set_lamda(&tpm_passphrase_utils::getGlobalTPMConfigPath, [&dir]() -> QString {
        __DBG_STUB_INVOKE__
        return dir.path();
    });

    // Act
    device_utils::cacheToken("/dev/sdb1", QVariantMap {});

    // Assert
    EXPECT_EQ(QDir(devDir).exists(), false);
    EXPECT_NE(QDir(devDir).exists(), true);   // double-check stable state
}

TEST_F(UT_EncryptUtilsCov, CacheToken_FullToken_WritesFiveFiles)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    stub.set_lamda(&tpm_passphrase_utils::getGlobalTPMConfigPath, [&dir]() -> QString {
        __DBG_STUB_INVOKE__
        return dir.path();
    });
    QVariantMap token {
        { "iv", QString(QByteArray("iv-data").toBase64()) },
        { "kek-priv", QString(QByteArray("priv").toBase64()) },
        { "kek-pub", QString(QByteArray("pub").toBase64()) },
        { "enc", QString(QByteArray("cipher").toBase64()) },
    };

    // Act
    device_utils::cacheToken("/dev/sdb1", token);

    // Assert
    QString devDir = dir.path() + "/dev/sdb1";
    EXPECT_TRUE(QFile::exists(devDir + "/token.json"));
    EXPECT_TRUE(QFile::exists(devDir + "/iv.bin"));
    EXPECT_TRUE(QFile::exists(devDir + "/key.priv"));
    EXPECT_TRUE(QFile::exists(devDir + "/key.pub"));
    EXPECT_TRUE(QFile::exists(devDir + "/cipher.out"));
    QFile iv(devDir + "/iv.bin");
    ASSERT_TRUE(iv.open(QIODevice::ReadOnly));
    EXPECT_EQ(iv.readAll(), QByteArray("iv-data"));
}

// ========== tpm_passphrase_utils ==========

TEST_F(UT_EncryptUtilsCov, GenPassphraseFromTPM_NoRandom_Fails)
{
    // Arrange
    stub.set_lamda(&tpm_utils::getRandomByTPM, [](int, QString *) -> int {
        __DBG_STUB_INVOKE__
        return -1;
    });
    QString passphrase;

    // Act
    int err = tpm_passphrase_utils::genPassphraseFromTPM("/dev/sdb1", "", &passphrase);

    // Assert
    EXPECT_EQ(err, tpm_passphrase_utils::kTPMNoRandomNumber);
    EXPECT_NE(err, tpm_passphrase_utils::kTPMNoError);
}

TEST_F(UT_EncryptUtilsCov, GenPassphraseFromTPM_FullFlow_Succeeds)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    stub.set_lamda(&tpm_utils::getRandomByTPM, [](int, QString *out) -> int {
        __DBG_STUB_INVOKE__
        *out = "random-key-123";
        return 0;
    });
    stub.set_lamda(&tpm_passphrase_utils::getGlobalTPMConfigPath, [&dir]() -> QString {
        __DBG_STUB_INVOKE__
        return dir.path();
    });
    stub.set_lamda(&tpm_passphrase_utils::getAlgorithm,
                   [](QString *s, QString *k, QString *ph, QString *pk, QString *mh, QString *mk, QString *pcr, QString *bank) -> bool {
                       __DBG_STUB_INVOKE__
                       *s = "sha256"; *k = "aes"; *ph = "sha256"; *pk = "rsa";
                       *mh = "sha256"; *mk = "aes"; *pcr = "7"; *bank = "sha256";
                       return true;
                   });
    stub.set_lamda(&tpm_utils::encryptByTPM, [](const QVariantMap &) -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });
    QString passphrase;

    // Act (no pin -> TPM+PCR branch)
    int err = tpm_passphrase_utils::genPassphraseFromTPM("/dev/sdb1", "", &passphrase);
    int errPin = tpm_passphrase_utils::genPassphraseFromTPM("/dev/sdb1", "1234", &passphrase);

    // Assert
    EXPECT_EQ(err, tpm_passphrase_utils::kTPMNoError);
    EXPECT_EQ(errPin, tpm_passphrase_utils::kTPMNoError);
}

TEST_F(UT_EncryptUtilsCov, GetPassphraseFromTPM_MissingToken_ReturnsEmpty)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    stub.set_lamda(&tpm_passphrase_utils::getGlobalTPMConfigPath, [&dir]() -> QString {
        __DBG_STUB_INVOKE__
        return dir.path();
    });

    // Act
    QString pwd = tpm_passphrase_utils::getPassphraseFromTPM("/dev/sdb1", "");

    // Assert
    EXPECT_EQ(pwd.isEmpty(), true);
    EXPECT_NE(pwd.isEmpty(), false);   // double-check stable state
}

TEST_F(UT_EncryptUtilsCov, GetPassphraseFromTPM_TokenWithoutKeys_ReturnsEmpty)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    stub.set_lamda(&tpm_passphrase_utils::getGlobalTPMConfigPath, [&dir]() -> QString {
        __DBG_STUB_INVOKE__
        return dir.path();
    });
    QDir().mkpath(dir.path() + "/dev/sdb1");
    QFile f(dir.path() + "/dev/sdb1/token.json");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("{\"foo\":\"bar\"}");
    f.close();

    // Act
    QString pwd = tpm_passphrase_utils::getPassphraseFromTPM("/dev/sdb1", "");

    // Assert
    EXPECT_EQ(pwd.isEmpty(), true);
    EXPECT_NE(pwd.isEmpty(), false);   // double-check stable state
}

TEST_F(UT_EncryptUtilsCov, GetPassphraseFromTPM_ValidToken_Decrypts)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    stub.set_lamda(&tpm_passphrase_utils::getGlobalTPMConfigPath, [&dir]() -> QString {
        __DBG_STUB_INVOKE__
        return dir.path();
    });
    QDir().mkpath(dir.path() + "/dev/sdb1");
    QFile f(dir.path() + "/dev/sdb1/token.json");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("{\"session-hash-alg\":\"sha256\",\"session-key-alg\":\"aes\","
            "\"primary-hash-alg\":\"sha256\",\"primary-key-alg\":\"rsa\","
            "\"pcr\":\"7\",\"pcr-bank\":\"sha256\"}");
    f.close();
    stub.set_lamda(&tpm_utils::decryptByTPM, [](const QVariantMap &, QString *psw) -> int {
        __DBG_STUB_INVOKE__
        *psw = "decrypted-secret";
        return 0;
    });

    // Act
    QString pwd = tpm_passphrase_utils::getPassphraseFromTPM("/dev/sdb1", "");

    // Assert
    EXPECT_EQ(pwd, QString("decrypted-secret"));
    EXPECT_EQ(pwd.length(), 16);
}

TEST_F(UT_EncryptUtilsCov, GetPassphraseFromTPM_NonBlock_ReturnsFutureResult)
{
    // Arrange
    stub.set_lamda(&tpm_passphrase_utils::getPassphraseFromTPM,
                   [](const QString &, const QString &) -> QString {
                       __DBG_STUB_INVOKE__
                       return QString("async-secret");
                   });

    // Act
    QString pwd = tpm_passphrase_utils::getPassphraseFromTPM_NonBlock("/dev/sdb1", "1234");

    // Assert
    EXPECT_EQ(pwd, QString("async-secret"));
    EXPECT_EQ(pwd.length(), 12);
}

TEST_F(UT_EncryptUtilsCov, GenPassphraseFromTPM_NonBlock_ReturnsFutureResult)
{
    // Arrange
    stub.set_lamda(&tpm_passphrase_utils::genPassphraseFromTPM,
                   [](const QString &, const QString &, QString *passphrase) -> int {
                       __DBG_STUB_INVOKE__
                       *passphrase = "generated-key";
                       return tpm_passphrase_utils::kTPMNoError;
                   });
    QString passphrase;

    // Act
    int err = tpm_passphrase_utils::genPassphraseFromTPM_NonBlock("/dev/sdb1", "1234", &passphrase);

    // Assert
    EXPECT_EQ(err, tpm_passphrase_utils::kTPMNoError);
    EXPECT_EQ(passphrase, QString("generated-key"));
}

// ========== dialog_utils / config_utils ==========

TEST_F(UT_EncryptUtilsCov, ShowDialog_ExecStubbed_ReturnsButtonCode)
{
    // Arrange: dialog auto-finished as confirmed
    autoFinishDialog(1);

    // Act
    int ret = dialog_utils::showDialog("T", "M");

    // Assert
    EXPECT_EQ(ret, 1);
    EXPECT_NE(ret, 2);   // complementary bound
}

TEST_F(UT_EncryptUtilsCov, ShowTPMError_KnownErrors_ShowDialogWithMessage)
{
    // Arrange
    QString capturedMsg;
    stub.set_lamda(&dialog_utils::showDialog, [&capturedMsg](const QString &, const QString &msg) -> int {
        __DBG_STUB_INVOKE__
        capturedMsg = msg;
        return 0;
    });

    // Act: known errors show a dialog

    // Assert
    dialog_utils::showTPMError("err", tpm_passphrase_utils::kTPMNoRandomNumber);
    EXPECT_EQ(capturedMsg, QString("Cannot generate random number by TPM"));
    dialog_utils::showTPMError("err", tpm_passphrase_utils::kTPMLocked);
    EXPECT_EQ(capturedMsg, QString("TPM is locked."));

    // unknown error -> no dialog at all
    capturedMsg.clear();
    dialog_utils::showTPMError("err", static_cast<tpm_passphrase_utils::TPMError>(999));
    EXPECT_TRUE(capturedMsg.isEmpty());
}

TEST_F(UT_EncryptUtilsCov, ShowConfirmEncryptionDialog_ExecStubbed_ReturnsAccepted)
{
    // Arrange: dialog auto-finished as confirmed
    autoFinishDialog(1);

    // Act (one auto-finish per exec)

    // Assert
    EXPECT_EQ(dialog_utils::showConfirmEncryptionDialog("data(sdb1)", false), 1);
    autoFinishDialog(1);
    EXPECT_EQ(dialog_utils::showConfirmEncryptionDialog("data(sdb1)", true), 1);
}

TEST_F(UT_EncryptUtilsCov, ShowConfirmDecryptionDialog_ExecStubbed_ReturnsAccepted)
{
    // Arrange: dialog auto-finished as confirmed
    autoFinishDialog(1);

    // Act

    // Assert
    int code = dialog_utils::showConfirmDecryptionDialog("data(sdb1)", false);
    EXPECT_EQ(code, 1);
    EXPECT_NE(code, 2);   // complementary bound
}

TEST_F(UT_EncryptUtilsCov, UseOverlayDMMode_ReadsDConfigValue)
{
    // Arrange
    stubDConfigValue([](const QString &key) -> QVariant {
        __DBG_STUB_INVOKE__
        if (key == "useOverlayDMMode")
            return true;
        return QVariant();
    });

    // Act

    // Assert
    EXPECT_EQ(config_utils::useOverlayDMMode(), true);
    EXPECT_NE(config_utils::useOverlayDMMode(), false);   // double-check stable state
}

TEST_F(UT_EncryptUtilsCov, TpmAlgoFromDConfig_CompleteAndIncomplete)
{
    // Arrange: complete config
    stubDConfigValue([](const QString &key) -> QVariant {
        __DBG_STUB_INVOKE__
        return QString("val-") + key;
    });
    QString s1, s2, s3, s4, s5, s6, s7, s8;

    // Act
    bool complete = config_utils::tpmAlgoFromDConfig(&s1, &s2, &s3, &s4, &s5, &s6, &s7, &s8);

    // Assert
    EXPECT_TRUE(complete);
    EXPECT_EQ(s1, QString("val-tpmSessionHashAlgoName"));
    EXPECT_EQ(s8, QString("val-tpmPcrBank"));

    // incomplete config -> false
    stubDConfigValue([](const QString &) -> QVariant {
        __DBG_STUB_INVOKE__
        return QString();
    });
    EXPECT_FALSE(config_utils::tpmAlgoFromDConfig(&s1, &s2, &s3, &s4, &s5, &s6, &s7, &s8));
}
