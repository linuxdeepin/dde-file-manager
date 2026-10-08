// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS sev-2 regression tests for OperatorCenter.
// Covers: BUG-172923 (savePasswordAndPasswordHint must store the salt+cipher,
// not the plaintext, unless the use_user_password switch is set),
// BUG-276953 (savePasswordToKeyring must not double-free / corrupt the heap),
// BUG-339741 (verificationRetrievePassword old-version branch must return the
// verified cipher, not the raw decrypt output).
// libsecret is real but the secret service is stubbed; no LUKS container is
// touched (password_container.bin is kept absent).

#include <gtest/gtest.h>

#undef signals
extern "C" {
#include <libsecret/secret.h>
}
#define signals public

#include "utils/encryption/operatorcenter.h"

#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFile>
#include <QDir>
#include <QSettings>

#include "stubext.h"

#include "utils/encryption/vaultconfig.h"
#include "utils/vaultdefine.h"
#include "utils/operator/pbkdf2.h"
#include "utils/operator/rsam.h"

DPVAULT_USE_NAMESPACE

namespace {
const QString kConfigPath = kVaultBasePath + "/" + QString(kVaultConfigFileName);
const QString kContainerPath = kVaultBasePath + "/password_container.bin";
}   // namespace

static GError *makeSecretError()
{
    static GError err = { 0, 1, nullptr };
    err.domain = g_quark_from_static_string("ut-vault");
    err.code = 1;
    if (!err.message)
        err.message = g_strdup("no secret service in ut");
    return &err;
}

class OperatorCenterRegressionTest : public testing::Test
{
protected:
    void SetUp() override
    {
        operatorCenter = OperatorCenter::getInstance();
        QDir().mkpath(kVaultBasePath);
        if (QFile::exists(kConfigPath))
            QFile::copy(kConfigPath, kConfigPath + ".utbak");
        // start clean: drop any user-password switch (the dev machine may carry
        // one in the real vault config); an empty key would still be treated as
        // "user password mode" by the production check
        {
            QSettings s(kConfigPath, QSettings::IniFormat);
            s.remove(QString("/%1/%2").arg(kConfigNodeName).arg(kConfigKeyUseUserPassWord));
            s.sync();
        }
    }

    void TearDown() override
    {
        stub.clear();
        operatorCenter->clearSaltAndPasswordCipher();   // keep singleton state clean
        QFile::remove(kContainerPath);
        QFile::remove(kConfigPath);
        if (QFile::exists(kConfigPath + ".utbak")) {
            QFile::remove(kConfigPath);
            QFile::rename(kConfigPath + ".utbak", kConfigPath);
        }
        QFile::remove(kVaultBasePath + "/" + QString(kPasswordHintFileName));
        QFile::remove(kVaultBasePath + "/" + QString(kRSACiphertextFileName));
    }

    void writeContainer(bool present)
    {
        QFile::remove(kContainerPath);
        if (present) {
            QFile f(kContainerPath);
            f.open(QIODevice::WriteOnly);
            f.close();
        }
    }

    stub_ext::StubExt stub;
    OperatorCenter *operatorCenter = nullptr;
};

// PMS:172923 cipher mode (no use_user_password switch) must keep the plaintext
// password out of strCryfsPassword; user-password mode keeps the plaintext
TEST_F(OperatorCenterRegressionTest, BUG172923_SavePasswordStoresCipherUnlessUserPasswordMode)
{
    writeContainer(false);
    const QString kPassword = QStringLiteral("ut-pwd-12345");

    // default (cipher) mode: strCryfsPassword must be the salt+cipher, never the plaintext
    Result ret = operatorCenter->savePasswordAndPasswordHint(kPassword, "ut-hint");
    ASSERT_TRUE(ret.result);

    VaultConfig config;
    const QString &cipher = config.get(kConfigNodeName, kConfigKeyCipher).toString();
    EXPECT_EQ(cipher.length(), kRandomSaltLength + kPasswordCipherLength);
    // regression: cipher mode must keep the plaintext OUT of strCryfsPassword,
    // storing the salt+cipher (random salt followed by the pbkdf2 digest)
    EXPECT_NE(operatorCenter->strCryfsPassword, kPassword);
    EXPECT_EQ(operatorCenter->strCryfsPassword.length(), kRandomSaltLength + kPasswordCipherLength);
    EXPECT_TRUE(operatorCenter->strCryfsPassword.startsWith(cipher.left(kRandomSaltLength)));

    // hint file is written next to the vault config
    QFile hintFile(kVaultBasePath + "/" + QString(kPasswordHintFileName));
    ASSERT_TRUE(hintFile.open(QIODevice::ReadOnly));
    EXPECT_EQ(QString::fromUtf8(hintFile.readAll()), QString("ut-hint"));
    hintFile.close();

    // explicit user-password mode: the plaintext is the cryfs password
    config.set(kConfigNodeName, kConfigKeyUseUserPassWord, QVariant(QString("Yes")));
    ret = operatorCenter->savePasswordAndPasswordHint(kPassword, "ut-hint");
    ASSERT_TRUE(ret.result);
    EXPECT_EQ(operatorCenter->strCryfsPassword, kPassword);
}

// PMS:276953 savePasswordToKeyring must survive secret-service errors without
// heap corruption (the pre-fix code passed a QByteArray-owned buffer to
// secret_value_new_full, which was later freed) and report failures properly
TEST_F(OperatorCenterRegressionTest, BUG276953_SavePasswordToKeyringErrorPathsNoCrash)
{
    writeContainer(false);

    // 1) secret service unavailable -> clean failure, no crash
    stub.set_lamda(&secret_service_get_sync,
                   [](SecretServiceFlags, GCancellable *, GError **error) -> SecretService * {
                       if (error)
                           *error = makeSecretError();
                       return nullptr;
                   });
    Result ret = operatorCenter->savePasswordToKeyring("ut-keyring-pwd");
    EXPECT_FALSE(ret.result);

    // 2) service ok but store fails -> clean failure, no crash/double-free
    stub.clear();
    stub.set_lamda(&secret_service_get_sync,
                   [](SecretServiceFlags, GCancellable *, GError **) -> SecretService * {
                       return reinterpret_cast<SecretService *>(quintptr(1));
                   });
    stub.set_lamda(&secret_service_store_sync,
                   [](SecretService *, const SecretSchema *, GHashTable *, const gchar *,
                      const gchar *, SecretValue *, GCancellable *, GError **error) -> gboolean {
                       if (error)
                           *error = makeSecretError();
                       return FALSE;
                   });
    ret = operatorCenter->savePasswordToKeyring("ut-keyring-pwd");
    EXPECT_FALSE(ret.result);

    // 3) store succeeds -> success, value cleanup must not corrupt the heap
    stub.clear();
    stub.set_lamda(&secret_service_get_sync,
                   [](SecretServiceFlags, GCancellable *, GError **) -> SecretService * {
                       return reinterpret_cast<SecretService *>(quintptr(1));
                   });
    stub.set_lamda(&secret_service_store_sync,
                   [](SecretService *, const SecretSchema *, GHashTable *, const gchar *,
                      const gchar *, SecretValue *, GCancellable *, GError **) -> gboolean {
                       return TRUE;
                   });
    ret = operatorCenter->savePasswordToKeyring("ut-keyring-pwd");
    EXPECT_TRUE(ret.result);
}

// PMS:339741 old-version verificationRetrievePassword must output the verified
// cipher (checkPassword result), not the raw publicKeyDecrypt output
TEST_F(OperatorCenterRegressionTest, BUG339741_OldVersionVerificationReturnsCheckedPassword)
{
    writeContainer(false);   // absent container -> old vault version branch

    // prepare the RSA cipher file the old branch reads
    const QString rsaCipherPath = kVaultBasePath + "/" + QString(kRSACiphertextFileName);
    {
        QFile f(rsaCipherPath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write("ut-rsa-cipher");
        f.close();
    }
    // key file passed by the caller
    QTemporaryFile keyFile;
    ASSERT_TRUE(keyFile.open());
    keyFile.write("ut-public-key");
    keyFile.close();

    const QString kWrongDecrypt = QStringLiteral("ut-wrong-decrypted");
    const QString kCheckedPassword = QStringLiteral("ut-checked-password");

    stub.set_lamda(&rsam::publicKeyDecrypt,
                   [kWrongDecrypt](const QString &, const QString &) -> QString {
                       return kWrongDecrypt;
                   });
    stub.set_lamda(ADDR(OperatorCenter, checkPassword),
                   [kCheckedPassword](OperatorCenter *, const QString &, QString &cipher) -> bool {
                       cipher = kCheckedPassword;
                       return true;
                   });

    QString password;
    ASSERT_TRUE(operatorCenter->verificationRetrievePassword(keyFile.fileName(), password));
    // the output must be the verified cipher, not the raw decrypt result
    EXPECT_EQ(password, kCheckedPassword);
    EXPECT_NE(password, kWrongDecrypt);
}

// PMS:314353 creation failures (e.g. full disk: unwritable config dir) must
// surface a descriptive errno message instead of a bare false, so the creation
// page can tell the user what went wrong.
TEST_F(OperatorCenterRegressionTest, BUG314353_CreateVaultFailureCarriesErrnoMessage)
{
    QTemporaryDir base;
    ASSERT_TRUE(base.isValid());
    // a regular file blocks the path where the vault config dir would be created,
    // mkpath fails with ENOTDIR/ENOTEMPTY — same code path as a full disk (ENOSPC)
    QFile blocker(base.path() + "/blocker");
    ASSERT_TRUE(blocker.open(QIODevice::WriteOnly));
    blocker.close();

    const QString blockedDir = base.path() + "/blocker";
    stub.set_lamda(ADDR(OperatorCenter, makeVaultLocalPath),
                   [blockedDir](OperatorCenter *, const QString &, const QString &) -> QString {
                       return blockedDir;
                   });

    Result ret = operatorCenter->createDirAndFile();
    EXPECT_FALSE(ret.result);
    EXPECT_FALSE(ret.message.isEmpty());
}
