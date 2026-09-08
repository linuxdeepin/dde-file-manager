// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// OperatorCenter deep-coverage tests. All LUKS-touching flows stop on empty
// container files or failing password checks, so no real container is ever
// formatted. libsecret calls are stubbed to avoid the real secret service.

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
#include <QFileInfo>

#include "stubext.h"

#include "utils/encryption/interfaceactivevault.h"
#include "utils/encryption/masterkeymanager.h"
#include "utils/encryption/vaultconfig.h"
#include "utils/vaultdefine.h"
#include "utils/operator/pbkdf2.h"
#include "utils/operator/rsam.h"

#include <QPointer>
#include <QRegularExpression>

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

class OperatorCenterMoreTest : public testing::Test
{
protected:
    void SetUp() override
    {
        operatorCenter = OperatorCenter::getInstance();
        QDir().mkpath(kVaultBasePath);
        if (QFile::exists(kConfigPath))
            QFile::copy(kConfigPath, kConfigPath + ".utbak");
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
        QFile::remove(kVaultBasePath + "/" + QString(kRSACiphertextFileName));
        QFile::remove(kVaultBasePath + "/" + QString(kRSAPUBKeyFileName));
        QFile::remove(kVaultBasePath + "/" + QString(kPasswordHintFileName));
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

TEST_F(OperatorCenterMoreTest, RunCmd_TrueCommand_Succeeds)
{
    EXPECT_TRUE(operatorCenter->runCmd("true"));
    EXPECT_FALSE(operatorCenter->runCmd("/no/such/command_ut"));
}

TEST_F(OperatorCenterMoreTest, ExecuteProcess_NonSudo_DelegatesToRunCmd)
{
    EXPECT_TRUE(operatorCenter->executeProcess("true"));
    EXPECT_FALSE(operatorCenter->executeProcess("/no/such/command_ut"));
}

TEST_F(OperatorCenterMoreTest, CreateKeyNew_GeneratesPubKeyAndCipherFile)
{
    Result ret = operatorCenter->createKeyNew("testpassword");
    ASSERT_TRUE(ret.result);
    EXPECT_GE(operatorCenter->getPubKey().length(), 2 * kUserKeyInterceptIndex + 32);
    EXPECT_TRUE(QFile::exists(kVaultBasePath + "/" + QString(kRSACiphertextFileName)));
}

TEST_F(OperatorCenterMoreTest, SaveKey_EmptyKey_ReturnsFailure)
{
    Result ret = operatorCenter->saveKey("", "/tmp/ut_vault_key");
    EXPECT_FALSE(ret.result);
    EXPECT_FALSE(QFile::exists("/tmp/ut_vault_key"));
}

TEST_F(OperatorCenterMoreTest, SaveKey_ValidKey_WritesFileWithExactContent)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString path = dir.path() + "/pub.key";
    Result ret = operatorCenter->saveKey("ABCDEF1234", path);
    ASSERT_TRUE(ret.result);
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    EXPECT_EQ(QString::fromUtf8(f.readAll()), QString("ABCDEF1234"));
}

TEST_F(OperatorCenterMoreTest, RecoveryKey_RoundTrip_ReturnsSameValue)
{
    operatorCenter->setRecoveryKey("Rk_0123456789");
    EXPECT_EQ(operatorCenter->getRecoveryKey(), QString("Rk_0123456789"));
    operatorCenter->setRecoveryKey("");
    EXPECT_TRUE(operatorCenter->getRecoveryKey().isEmpty());
}

TEST_F(OperatorCenterMoreTest, PendingMigrationPassword_RoundTrip_ReturnsSameValue)
{
    operatorCenter->setPendingOldPasswordSchemeMigrationPassword("pending-pwd");
    EXPECT_EQ(operatorCenter->getPendingOldPasswordSchemeMigrationPassword(), QString("pending-pwd"));
}

TEST_F(OperatorCenterMoreTest, GenerateRecoveryKeyForNewVault_Returns32CharKey)
{
    QString key = operatorCenter->generateRecoveryKeyForNewVault();
    EXPECT_EQ(key.length(), 32);
    EXPECT_EQ(operatorCenter->getRecoveryKey(), key);
}

TEST_F(OperatorCenterMoreTest, VerificationRetrievePassword_NewVersionBadLength_ReturnsFailure)
{
    writeContainer(true);
    QTemporaryFile keyFile;
    ASSERT_TRUE(keyFile.open());
    keyFile.write("shortkey");
    keyFile.close();

    QString password;
    EXPECT_FALSE(operatorCenter->verificationRetrievePassword(keyFile.fileName(), password));
    EXPECT_TRUE(password.isEmpty());
}

TEST_F(OperatorCenterMoreTest, VerificationRetrievePassword_NewVersionBadCharset_ReturnsFailure)
{
    writeContainer(true);
    QTemporaryFile keyFile;
    ASSERT_TRUE(keyFile.open());
    keyFile.write("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");   // 32 chars, wrong charset
    keyFile.close();

    QString password;
    EXPECT_FALSE(operatorCenter->verificationRetrievePassword(keyFile.fileName(), password));
}

TEST_F(OperatorCenterMoreTest, VerificationRetrievePassword_NewVersionValid_ReturnsPassword)
{
    writeContainer(true);
    QTemporaryFile keyFile;
    ASSERT_TRUE(keyFile.open());
    keyFile.write("AbCdEf1234567890AbCdEf1234567890");
    keyFile.close();

    stub.set_lamda(&OperatorCenter::checkPassword,
                   [](OperatorCenter *, const QString &, QString &cipher) -> bool {
                       cipher = "decrypted-cipher";
                       return true;
                   });

    QString password;
    ASSERT_TRUE(operatorCenter->verificationRetrievePassword(keyFile.fileName(), password));
    EXPECT_EQ(password, QString("decrypted-cipher"));
}

TEST_F(OperatorCenterMoreTest, VerificationRetrievePassword_OldVersionNoFiles_ReturnsFailure)
{
    writeContainer(false);
    QString password;
    EXPECT_FALSE(operatorCenter->verificationRetrievePassword("/no/such/keyfile_ut", password));
    EXPECT_TRUE(password.isEmpty());
}

TEST_F(OperatorCenterMoreTest, CreateDirAndFile_OldVersion_CreatesLegacyFiles)
{
    writeContainer(false);
    Result ret = operatorCenter->createDirAndFile();
    ASSERT_TRUE(ret.result);
    EXPECT_TRUE(QFile::exists(kVaultBasePath + "/" + QString(kRSAPUBKeyFileName)));
    EXPECT_TRUE(QFile::exists(kVaultBasePath + "/" + QString(kRSACiphertextFileName)));
    EXPECT_TRUE(QFile::exists(kVaultBasePath + "/" + QString(kPasswordHintFileName)));
}

TEST_F(OperatorCenterMoreTest, CreateDirAndFile_NewVersion_SkipsRsaFiles)
{
    writeContainer(true);
    Result ret = operatorCenter->createDirAndFile();
    ASSERT_TRUE(ret.result);
    EXPECT_FALSE(QFile::exists(kVaultBasePath + "/" + QString(kRSAPUBKeyFileName)));
    EXPECT_TRUE(QFile::exists(kVaultBasePath + "/" + QString(kPasswordHintFileName)));
}

TEST_F(OperatorCenterMoreTest, SavePasswordAndPasswordHint_WritesCipherAndHint)
{
    Result ret = operatorCenter->savePasswordAndPasswordHint("my-password", "my-hint");
    ASSERT_TRUE(ret.result);

    QFile hintFile(kVaultBasePath + "/" + QString(kPasswordHintFileName));
    ASSERT_TRUE(hintFile.open(QIODevice::ReadOnly));
    EXPECT_EQ(QString::fromUtf8(hintFile.readAll()), QString("my-hint"));

    VaultConfig config;
    QString cipher = config.get(kConfigNodeName, kConfigKeyCipher).toString();
    EXPECT_EQ(cipher.length(), kRandomSaltLength + kPasswordCipherLength);
}

TEST_F(OperatorCenterMoreTest, SavePasswordToKeyring_ServiceUnavailable_ReturnsFailure)
{
    stub.set_lamda(&secret_service_get_sync,
                   [](SecretServiceFlags, GCancellable *, GError **error) -> SecretService * {
                       if (error)
                           *error = makeSecretError();
                       return nullptr;
                   });
    Result ret = operatorCenter->savePasswordToKeyring("pwd");
    EXPECT_FALSE(ret.result);
}

TEST_F(OperatorCenterMoreTest, SavePasswordToKeyring_StoreSucceeds_ReturnsSuccess)
{
    static gboolean storeResult = TRUE;
    stub.set_lamda(&secret_service_get_sync,
                   [](SecretServiceFlags, GCancellable *, GError **) -> SecretService * {
                       return reinterpret_cast<SecretService *>(quintptr(1));
                   });
    stub.set_lamda(&secret_service_store_sync,
                   [](SecretService *, const SecretSchema *, GHashTable *, const gchar *,
                      const gchar *, SecretValue *, GCancellable *, GError **) -> gboolean {
                       return storeResult;
                   });
    Result ret = operatorCenter->savePasswordToKeyring("pwd");
    EXPECT_TRUE(ret.result);
}

TEST_F(OperatorCenterMoreTest, PasswordFromKeyring_NothingStored_ReturnsEmpty)
{
    stub.set_lamda(&secret_service_get_sync,
                   [](SecretServiceFlags, GCancellable *, GError **) -> SecretService * {
                       return reinterpret_cast<SecretService *>(quintptr(1));
                   });
    stub.set_lamda(&secret_service_lookup_sync,
                   [](SecretService *, const SecretSchema *, GHashTable *, GCancellable *,
                      GError **) -> SecretValue * { return nullptr; });
    stub.set_lamda(&secret_value_get,
                   [](SecretValue *, gsize *length) -> const gchar * {
                       if (length)
                           *length = 0;
                       return "";
                   });
    stub.set_lamda(&secret_value_unref, [](gpointer) {});
    EXPECT_TRUE(operatorCenter->passwordFromKeyring().isEmpty());
}

TEST_F(OperatorCenterMoreTest, PasswordFromKeyring_ValueStored_ReturnsPassword)
{
    static const char *kStored = "stored-password";
    stub.set_lamda(&secret_service_get_sync,
                   [](SecretServiceFlags, GCancellable *, GError **) -> SecretService * {
                       return reinterpret_cast<SecretService *>(quintptr(1));
                   });
    stub.set_lamda(&secret_service_lookup_sync,
                   [](SecretService *, const SecretSchema *, GHashTable *, GCancellable *,
                      GError **) -> SecretValue * {
                       return reinterpret_cast<SecretValue *>(quintptr(2));
                   });
    stub.set_lamda(&secret_value_get,
                   [](SecretValue *, gsize *length) -> const gchar * {
                       if (length)
                           *length = 15;
                       return kStored;
                   });
    stub.set_lamda(&secret_value_unref, [](gpointer) {});
    EXPECT_EQ(operatorCenter->passwordFromKeyring(), QString("stored-password"));
}

TEST_F(OperatorCenterMoreTest, ResetPasswordByOldPassword_OldVersion_ReturnsFailure)
{
    writeContainer(false);
    EXPECT_FALSE(operatorCenter->resetPasswordByOldPassword("old", "new", "hint"));
    EXPECT_FALSE(QFile::exists(kVaultBasePath + "/" + QString(kPasswordHintFileName)));
}

TEST_F(OperatorCenterMoreTest, ResetPasswordByOldPassword_NewVersionEmptyContainer_ReturnsFailure)
{
    writeContainer(true);
    EXPECT_FALSE(operatorCenter->resetPasswordByOldPassword("old", "new", "hint"));
}

TEST_F(OperatorCenterMoreTest, ResetPasswordByRecoveryKey_BadKeyLength_ReturnsFailure)
{
    writeContainer(true);
    EXPECT_FALSE(operatorCenter->resetPasswordByRecoveryKey("short", "new", "hint"));
}

TEST_F(OperatorCenterMoreTest, ResetPasswordByRecoveryKey_EmptyContainer_ReturnsFailure)
{
    writeContainer(true);
    EXPECT_FALSE(operatorCenter->resetPasswordByRecoveryKey(
            "AbCdEf1234567890AbCdEf1234567890", "new", "hint"));
}

TEST_F(OperatorCenterMoreTest, MigrateOldVaultByPassword_NewVersionVault_ReturnsFailure)
{
    writeContainer(true);
    QString outKey;
    EXPECT_FALSE(operatorCenter->migrateOldVaultByPassword("old", "new", outKey));
    EXPECT_TRUE(outKey.isEmpty());
}

TEST_F(OperatorCenterMoreTest, MigrateOldVaultByPassword_BadOldPassword_ReturnsFailure)
{
    writeContainer(false);
    QString outKey;
    EXPECT_FALSE(operatorCenter->migrateOldVaultByPassword("wrong-old", "new", outKey));
    EXPECT_TRUE(outKey.isEmpty());
}

TEST_F(OperatorCenterMoreTest, MigrateOldVaultByRecoveryKey_EmptyKey_ReturnsFailure)
{
    writeContainer(false);
    QString outKey;
    EXPECT_FALSE(operatorCenter->migrateOldVaultByRecoveryKey("", "new", outKey));
    EXPECT_TRUE(outKey.isEmpty());
}

TEST_F(OperatorCenterMoreTest, MigrateOldVaultByRecoveryKey_BadKey_ReturnsFailure)
{
    writeContainer(false);
    QString outKey;
    EXPECT_FALSE(operatorCenter->migrateOldVaultByRecoveryKey("bad-key", "new", outKey));
    EXPECT_TRUE(outKey.isEmpty());
}

TEST_F(OperatorCenterMoreTest, MigrateOldVaultByRecoveryKey_NewVersionVault_ReturnsFailure)
{
    writeContainer(true);
    QString outKey;
    EXPECT_FALSE(operatorCenter->migrateOldVaultByRecoveryKey(
            "AbCdEf1234567890AbCdEf1234567890", "new", outKey));
}

TEST_F(OperatorCenterMoreTest, UpgradeOldVaultByPassword_NewVersionVault_ReturnsFailure)
{
    writeContainer(true);
    QString outKey;
    EXPECT_FALSE(operatorCenter->upgradeOldVaultByPassword("old", outKey));
    EXPECT_TRUE(outKey.isEmpty());
}

TEST_F(OperatorCenterMoreTest, UpgradeOldVaultByPassword_BadPassword_ReturnsFailure)
{
    writeContainer(false);
    QString outKey;
    EXPECT_FALSE(operatorCenter->upgradeOldVaultByPassword("wrong", outKey));
    EXPECT_TRUE(outKey.isEmpty());
}

// --- InterfaceActiveVault delegation ---

TEST_F(OperatorCenterMoreTest, InterfaceActiveVault_ParentOwned_DestructorRuns)
{
    QObject *owner = new QObject();
    QPointer<InterfaceActiveVault> vault = new InterfaceActiveVault(owner);
    ASSERT_FALSE(vault.isNull());
    delete owner;
    EXPECT_TRUE(vault.isNull());
}

TEST_F(OperatorCenterMoreTest, InterfaceActiveVault_CheckPassword_NoSetup_ReturnsFailure)
{
    InterfaceActiveVault vault;
    QString cipher;
    EXPECT_FALSE(vault.checkPassword("pwd", cipher));
    EXPECT_FALSE(vault.checkUserKey(QString(kUserKeyLength, 'A'), cipher));
}

TEST_F(OperatorCenterMoreTest, InterfaceActiveVault_GetPasswordHint_NoFile_ReturnsFailure)
{
    InterfaceActiveVault vault;
    QString hint;
    EXPECT_FALSE(vault.getPasswordHint(hint));
    EXPECT_TRUE(hint.isEmpty());
}

// --- MasterKeyManager ---

TEST_F(OperatorCenterMoreTest, MasterKeyManager_GenerateMasterKey_Returns64Bytes)
{
    QByteArray key = MasterKeyManager::generateMasterKey();
    EXPECT_EQ(key.size(), 64);
}

TEST_F(OperatorCenterMoreTest, MasterKeyManager_GenerateFromPassword_PadsTo64Bytes)
{
    QByteArray key = MasterKeyManager::generateMasterKeyFromPassword("abc");
    ASSERT_EQ(key.size(), 64);
    EXPECT_TRUE(key.startsWith("abc"));
}

TEST_F(OperatorCenterMoreTest, MasterKeyManager_ContainerPath_IsUnderVaultBase)
{
    EXPECT_EQ(MasterKeyManager::getContainerPath(),
              kVaultBasePath + QString("/password_container.bin"));
}

// --- pbkdf2 / rsam helpers ---

TEST_F(OperatorCenterMoreTest, CreateRandomSalt_GivenBytes_ReturnsHexString)
{
    QString salt = pbkdf2::createRandomSalt(kRandomSaltLength);
    EXPECT_FALSE(salt.isEmpty());
    EXPECT_TRUE(salt.contains(QRegularExpression("^[0-9a-fA-F]+$")));
}

TEST_F(OperatorCenterMoreTest, PublicKeyDecrypt_EmptyInputs_ReturnsEmpty)
{
    EXPECT_TRUE(rsam::publicKeyDecrypt("", "").isEmpty());
}
