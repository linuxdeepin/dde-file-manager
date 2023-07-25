// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/encryption/vaultconfig.h"
#include "utils/operator/pbkdf2.h"
#include "utils/operator/rsam.h"

#include <gtest/gtest.h>

#include <QProcess>

DPVAULT_USE_NAMESPACE

TEST(UT_OperatorCenter, makeVaultLocalPath)
{
    QString path = OperatorCenter::getInstance()->makeVaultLocalPath("UT_TEST", "UT_TEST");

    EXPECT_TRUE(path == (kVaultBasePath + "/UT_TEST/UT_TEST"));
}

TEST(UT_OperatorCenter, runCmd_one)
{
    stub_ext::StubExt stub;
    typedef void(QProcess::*FuncType)(QProcess::OpenMode);
    stub.set_lamda(static_cast<FuncType>(&QProcess::start), []{});
    stub.set_lamda(&QProcess::readAllStandardOutput, []{
        return QByteArray("");
    });
    stub.set_lamda(&QProcess::exitCode, []{
        return 127;
    });

    bool isOk = OperatorCenter::getInstance()->runCmd(kRootProxy);

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, runCmd_two)
{
    stub_ext::StubExt stub;
    typedef void(QProcess::*FuncType)(QProcess::OpenMode);
    stub.set_lamda(static_cast<FuncType>(&QProcess::start), []{});
    stub.set_lamda(&QProcess::readAllStandardOutput, []{
        return QByteArray("");
    });
    stub.set_lamda(&QProcess::exitCode, []{
        return 0;
    });

    bool isOk = OperatorCenter::getInstance()->runCmd(kRootProxy);

    EXPECT_TRUE(isOk);
}

TEST(UT_OperatorCenter, executeProcess)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::runCmd, [ &isOk ]{
        isOk = true;
        return true;
    });

    OperatorCenter::getInstance()->executeProcess("UT_TEST");

    OperatorCenter::getInstance()->standOutput = "root";
    OperatorCenter::getInstance()->executeProcess("sudo UT_TEST");

    OperatorCenter::getInstance()->standOutput = "";
    OperatorCenter::getInstance()->executeProcess("sudo UT_TEST");

    EXPECT_TRUE(isOk);
}

TEST(UT_OperatorCenter, secondSaveSaltAndCiphertext_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&pbkdf2::pbkdf2EncrypyPassword, []{
        return "";
    });

    bool isOk = OperatorCenter::getInstance()->secondSaveSaltAndCiphertext("TEST1", "TEST2", "TEST3");

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, secondSaveSaltAndCiphertext_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&pbkdf2::pbkdf2EncrypyPassword, []{
        return "UT_TEST";
    });
    stub.set_lamda(&VaultConfig::set, [ &isOk ]{
        isOk = true;
    });

    OperatorCenter::getInstance()->secondSaveSaltAndCiphertext("TEST1", "TEST2", "TEST3");

    EXPECT_TRUE(isOk);
}

TEST(UT_OperatorCenter, createKeyNew_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&rsam::createPublicAndPrivateKey, [](QString &publicKey, QString privateKey){
        publicKey = "UT";
        return true;
    });
    stub.set_lamda(&rsam::privateKeyEncrypt, []{
        return "UT_TEST";
    });

    bool isOk = OperatorCenter::getInstance()->createKeyNew("UT_TEST");

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, createKeyNew_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&rsam::createPublicAndPrivateKey, [](QString &publicKey, QString privateKey){
        Q_UNUSED(privateKey)
        publicKey = "UT_TESTUT_TESTUT_TESTUT_TESTUT_TESTUT_TESTUT_TESTUT_TESTUT_TESTUT_TESTUT_TESTUT_TESTUT_TESTUT_TESTUT_TESTUT_TESTUT_TESTUT_TESTUT_TES";
        return true;
    });
    stub.set_lamda(&rsam::privateKeyEncrypt, []{
        return "UT_TEST";
    });
    stub.set_lamda(&OperatorCenter::makeVaultLocalPath, []{
        return "/UT_TEST";
    });

    bool isOk = OperatorCenter::getInstance()->createKeyNew("UT_TEST");

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, saveKey_one)
{
    bool isOk = OperatorCenter::getInstance()->saveKey("UT_TEST", "UT_TEST");

    EXPECT_TRUE(isOk);
}

TEST(UT_OperatorCenter, saveKey_two)
{
    stub_ext::StubExt stub;
    typedef bool(*FuncType)(QIODevice::OpenMode);
    stub.set_lamda((FuncType)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return true;
    });

    bool isOk = OperatorCenter::getInstance()->saveKey("UT_TEST", "/UT_TEST");

    EXPECT_TRUE(isOk);
}

TEST(UT_OperatorCenter, getPubKey)
{
    OperatorCenter::getInstance()->strPubKey = "UT_TEST";
    QString re = OperatorCenter::getInstance()->getPubKey();

    EXPECT_TRUE(re == "UT_TEST");
}

TEST(UT_OperatorCenter, verificationRetrievePassword_one)
{
    QString passwd;
    bool isOk = OperatorCenter::getInstance()->verificationRetrievePassword("/UT_TEST", passwd);

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, verificationRetrievePassword_two)
{
    stub_ext::StubExt stub;
    typedef bool(*FuncType)(QIODevice::OpenMode);
    stub.set_lamda((FuncType)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return true;
    });
    stub.set_lamda(&OperatorCenter::makeVaultLocalPath, []{
        return "/UT_TEST";
    });
    stub.set_lamda(&rsam::publicKeyDecrypt, []{
        return "UT_TEST";
    });
    stub.set_lamda(&OperatorCenter::checkPassword, []{
        return false;
    });

    QString passwd;
    bool isOk = OperatorCenter::getInstance()->verificationRetrievePassword("/UT_TEST", passwd);

    EXPECT_FALSE(isOk);
}


TEST(UT_OperatorCenter, verificationRetrievePassword_three)
{
    stub_ext::StubExt stub;
    typedef bool(*FuncType)(QIODevice::OpenMode);
    stub.set_lamda((FuncType)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return true;
    });
    stub.set_lamda(&OperatorCenter::makeVaultLocalPath, []{
        return "/UT_TEST";
    });
    stub.set_lamda(&rsam::publicKeyDecrypt, []{
        return "UT_TEST";
    });
    stub.set_lamda(&OperatorCenter::checkPassword, []{
        return true;
    });

    QString passwd;
    bool isOk = OperatorCenter::getInstance()->verificationRetrievePassword("/UT_TEST", passwd);

    EXPECT_TRUE(isOk);
}

TEST(UT_OperatorCenter, createDirAndFile_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::makeVaultLocalPath, []{
        return "/UT_TEST";
    });
    stub.set_lamda(&QDir::mkpath, []{
        return false;
    });

    bool isOk = OperatorCenter::getInstance()->createDirAndFile();

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, createDirAndFile_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::makeVaultLocalPath, []{
        return "/UT_TEST";
    });
    typedef bool(QDir::*FuncType1)()const;
    stub.set_lamda(static_cast<FuncType1>(&QDir::exists), []{
        return false;
    });
    stub.set_lamda(&QDir::mkpath, []{
        return true;
    });
    typedef bool(*FuncType2)(QIODevice::OpenMode);
    stub.set_lamda((FuncType2)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return false;
    });

    bool isOk = OperatorCenter::getInstance()->createDirAndFile();

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, createDirAndFile_three)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::makeVaultLocalPath, []{
        return "/UT_TEST";
    });
    typedef bool(QDir::*FuncType1)()const;
    stub.set_lamda(static_cast<FuncType1>(&QDir::exists), []{
        return false;
    });
    stub.set_lamda(&QDir::mkpath, []{
        return true;
    });
    typedef bool(*FuncType2)(QIODevice::OpenMode);
    stub.set_lamda((FuncType2)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return true;
    });
    typedef bool(*FuncType3)(QFileDevice::Permissions);
    stub.set_lamda((FuncType3)(bool(QFile::*)(QFileDevice::Permissions))(&QFile::setPermissions), []{
        return true;
    });

    bool isOk = OperatorCenter::getInstance()->createDirAndFile();

    EXPECT_TRUE(isOk);
}

TEST(UT_OperatorCenter, savePasswordAndPasswordHint_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::secondSaveSaltAndCiphertext, []{
        return true;
    });
    stub.set_lamda(&OperatorCenter::makeVaultLocalPath, []{
        return "/UT_TEST";
    });
    typedef bool(*FuncType1)(QIODevice::OpenMode);
    stub.set_lamda((FuncType1)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return false;
    });

    bool isOk = OperatorCenter::getInstance()->savePasswordAndPasswordHint("UT_TEST", "UT_TEST");

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, savePasswordAndPasswordHint_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::secondSaveSaltAndCiphertext, []{
        return true;
    });
    stub.set_lamda(&OperatorCenter::makeVaultLocalPath, []{
        return "/UT_TEST";
    });
    typedef bool(*FuncType1)(QIODevice::OpenMode);
    stub.set_lamda((FuncType1)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return true;
    });
    typedef QVariant(VaultConfig::*FuncType2)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<FuncType2>(&VaultConfig::get), []{
        return QVariant::fromValue<QString>(kConfigKeyNotExist);
    });

    bool isOk = OperatorCenter::getInstance()->savePasswordAndPasswordHint("UT_TEST", "UT_TEST");

    EXPECT_TRUE(isOk);
}

TEST(UT_OperatorCenter, savePasswordAndPasswordHint_three)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::secondSaveSaltAndCiphertext, []{
        return true;
    });
    stub.set_lamda(&OperatorCenter::makeVaultLocalPath, []{
        return "/UT_TEST";
    });
    typedef bool(*FuncType1)(QIODevice::OpenMode);
    stub.set_lamda((FuncType1)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return true;
    });
    typedef QVariant(VaultConfig::*FuncType2)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<FuncType2>(&VaultConfig::get), []{
        return QVariant::fromValue<QString>("UT_TEST");
    });

    bool isOk = OperatorCenter::getInstance()->savePasswordAndPasswordHint("UT_TEST", "UT_TEST");

    EXPECT_TRUE(isOk);
}

TEST(UT_OperatorCenter, createKey_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::makeVaultLocalPath, []{
        return "/UT_TEST";
    });
    typedef bool(*FuncType1)(QIODevice::OpenMode);
    stub.set_lamda((FuncType1)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return false;
    });

    bool isOk = OperatorCenter::getInstance()->createKey("UT_TEST", 7);

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, createKey_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::makeVaultLocalPath, []{
        return "/UT_TEST";
    });
    typedef bool(*FuncType1)(QIODevice::OpenMode);
    stub.set_lamda((FuncType1)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return true;
    });

    bool isOk = OperatorCenter::getInstance()->createKey("UT_TEST", 7);

    EXPECT_TRUE(isOk);
}

TEST(UT_OperatorCenter, checkPassword_one)
{
    stub_ext::StubExt stub;
    typedef QVariant(VaultConfig::*FuncType)(const QString &, const QString &);
    stub.set_lamda(static_cast<FuncType>(&VaultConfig::get), []{
        return QVariant::fromValue<QString>(kConfigVaultVersion);
    });

    QString cipher;
    bool isOk = OperatorCenter::getInstance()->checkPassword("UT_TEST", cipher);

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, checkPassword_two)
{
    stub_ext::StubExt stub;
    typedef QVariant(VaultConfig::*FuncType)(const QString &, const QString &);
    stub.set_lamda(static_cast<FuncType>(&VaultConfig::get), []{
        return QVariant::fromValue<QString>(kConfigVaultVersion);
    });
    stub.set_lamda(&pbkdf2::pbkdf2EncrypyPassword, []{
        return "";
    });
    typedef QVariant(VaultConfig::*FuncType2)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<FuncType2>(&VaultConfig::get), []{
        return QVariant::fromValue<QString>(kConfigKeyNotExist);
    });

    QString cipher;
    bool isOk = OperatorCenter::getInstance()->checkPassword("UT_TEST", cipher);

    EXPECT_TRUE(isOk);
}

TEST(UT_OperatorCenter, checkPassword_three)
{
    stub_ext::StubExt stub;
    typedef QVariant(VaultConfig::*FuncType)(const QString &, const QString &);
    stub.set_lamda(static_cast<FuncType>(&VaultConfig::get), []{
        return QVariant::fromValue<QString>(kConfigVaultVersion);
    });
    stub.set_lamda(&pbkdf2::pbkdf2EncrypyPassword, []{
        return "";
    });
    typedef QVariant(VaultConfig::*FuncType2)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<FuncType2>(&VaultConfig::get), []{
        return QVariant::fromValue<QString>(kConfigVaultVersion);
    });

    QString cipher;
    bool isOk = OperatorCenter::getInstance()->checkPassword("UT_TEST", cipher);

    EXPECT_TRUE(isOk);
}

TEST(UT_OperatorCenter, checkPassword_four)
{
    stub_ext::StubExt stub;
    typedef QVariant(VaultConfig::*FuncType)(const QString &, const QString &);
    stub.set_lamda(static_cast<FuncType>(&VaultConfig::get), []{
        return QVariant::fromValue<QString>("UT_TEST");
    });
    typedef bool(*FuncType1)(QIODevice::OpenMode);
    stub.set_lamda((FuncType1)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return false;
    });

    QString cipher;
    bool isOk = OperatorCenter::getInstance()->checkPassword("UT_TEST", cipher);

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, checkPassword_five)
{
    stub_ext::StubExt stub;
    typedef QVariant(VaultConfig::*FuncType)(const QString &, const QString &);
    stub.set_lamda(static_cast<FuncType>(&VaultConfig::get), []{
        return QVariant::fromValue<QString>("UT_TEST");
    });
    typedef bool(*FuncType1)(QIODevice::OpenMode);
    stub.set_lamda((FuncType1)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return true;
    });
    stub.set_lamda(&pbkdf2::pbkdf2EncrypyPassword, []{
        return "UT_TEST";
    });

    QString cipher;
    bool isOk = OperatorCenter::getInstance()->checkPassword("UT_TEST", cipher);

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, checkPassword_six)
{
    stub_ext::StubExt stub;
    typedef QVariant(VaultConfig::*FuncType)(const QString &, const QString &);
    stub.set_lamda(static_cast<FuncType>(&VaultConfig::get), []{
        return QVariant::fromValue<QString>("UT_TEST");
    });
    typedef bool(*FuncType1)(QIODevice::OpenMode);
    stub.set_lamda((FuncType1)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return true;
    });
    stub.set_lamda(&pbkdf2::pbkdf2EncrypyPassword, []{
        return "";
    });
    stub.set_lamda(&OperatorCenter::secondSaveSaltAndCiphertext, []{
        return false;
    });

    QString cipher;
    bool isOk = OperatorCenter::getInstance()->checkPassword("UT_TEST", cipher);

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, checkPassword_seven)
{
    stub_ext::StubExt stub;
    typedef QVariant(VaultConfig::*FuncType)(const QString &, const QString &);
    stub.set_lamda(static_cast<FuncType>(&VaultConfig::get), []{
        return QVariant::fromValue<QString>("UT_TEST");
    });
    typedef bool(*FuncType1)(QIODevice::OpenMode);
    stub.set_lamda((FuncType1)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return true;
    });
    stub.set_lamda(&pbkdf2::pbkdf2EncrypyPassword, []{
        return "";
    });
    stub.set_lamda(&OperatorCenter::secondSaveSaltAndCiphertext, []{
        return true;
    });
    typedef bool(*FuncType2)(const QString &);
    stub.set_lamda(static_cast<FuncType2>(&QFile::remove), []{
        return true;
    });

    QString cipher;
    bool isOk = OperatorCenter::getInstance()->checkPassword("UT_TEST", cipher);

    EXPECT_TRUE(isOk);
}

TEST(UT_OperatorCenter, checkUserKey_one)
{
    stub_ext::StubExt stub;

    QString cipher;
    bool isOk = OperatorCenter::getInstance()->checkUserKey("UT_TESTUT_TESTUT_TESTUT_TESTUT_", cipher);

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, checkUserKey_two)
{
    stub_ext::StubExt stub;
    typedef bool(*FuncType1)(QIODevice::OpenMode);
    stub.set_lamda((FuncType1)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return false;
    });

    QString cipher;
    bool isOk = OperatorCenter::getInstance()->checkUserKey("UT_TESTUT_TESTUT_TESTUT_TESTUT_U", cipher);

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, checkUserKey_three)
{
    stub_ext::StubExt stub;
    typedef bool(*FuncType1)(QIODevice::OpenMode);
    stub.set_lamda((FuncType1)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return true;
    });
    stub.set_lamda(&OperatorCenter::checkPassword, []{
        return false;
    });

    QString cipher;
    bool isOk = OperatorCenter::getInstance()->checkUserKey("UT_TESTUT_TESTUT_TESTUT_TESTUT_U", cipher);

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, checkUserKey_four)
{
    stub_ext::StubExt stub;
    typedef bool(*FuncType1)(QIODevice::OpenMode);
    stub.set_lamda((FuncType1)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return true;
    });
    stub.set_lamda(&OperatorCenter::checkPassword, []{
        return true;
    });

    QString cipher;
    bool isOk = OperatorCenter::getInstance()->checkUserKey("UT_TESTUT_TESTUT_TESTUT_TESTUT_U", cipher);

    EXPECT_TRUE(isOk);
}

TEST(UT_OperatorCenter, getUserKey)
{
    OperatorCenter::getInstance()->strUserKey = "UT_TEST";
    QString key = OperatorCenter::getInstance()->getUserKey();

    EXPECT_TRUE(key == "UT_TEST");
}

TEST(UT_OperatorCenter, getPasswordHint_one)
{
    stub_ext::StubExt stub;
    typedef bool(*FuncType1)(QIODevice::OpenMode);
    stub.set_lamda((FuncType1)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return false;
    });

    QString hint;
    bool isOk = OperatorCenter::getInstance()->getPasswordHint(hint);

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, getPasswordHint_two)
{
    stub_ext::StubExt stub;
    typedef bool(*FuncType1)(QIODevice::OpenMode);
    stub.set_lamda((FuncType1)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] {
        return true;
    });

    QString hint;
    OperatorCenter::getInstance()->getPasswordHint(hint);

    EXPECT_TRUE(hint.isEmpty());
}

TEST(UT_OperatorCenter, getSaltAndPasswordCipher)
{
    OperatorCenter::getInstance()->strCryfsPassword = "UT_TEST";
    QString cipher = OperatorCenter::getInstance()->getSaltAndPasswordCipher();

    EXPECT_TRUE(cipher ==  "UT_TEST");
}

TEST(UT_OperatorCenter, clearSaltAndPasswordCipher)
{
    OperatorCenter::getInstance()->strCryfsPassword = "UT_TEST";
    OperatorCenter::getInstance()->clearSaltAndPasswordCipher();

    EXPECT_TRUE(OperatorCenter::getInstance()->strCryfsPassword.isEmpty());
}

TEST(UT_OperatorCenter, getEncryptDirPath)
{
    EXPECT_FALSE(OperatorCenter::getInstance()->getEncryptDirPath().isEmpty());
}

TEST(UT_OperatorCenter, getdecryptDirPath)
{
    EXPECT_FALSE(OperatorCenter::getInstance()->getdecryptDirPath().isEmpty());
}

TEST(UT_OperatorCenter, getConfigFilePath)
{
    EXPECT_FALSE(OperatorCenter::getInstance()->getConfigFilePath().isEmpty());
}

TEST(UT_OperatorCenter, autoGeneratePassword)
{
    EXPECT_TRUE(OperatorCenter::getInstance()->autoGeneratePassword(20).length() == 20);
}

TEST(UT_OperatorCenter, getRootPassword_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::runCmd, []{
        return true;
    });

    OperatorCenter::getInstance()->standOutput == "root";
    bool isOk = OperatorCenter::getInstance()->getRootPassword();

    EXPECT_TRUE(isOk);
}

TEST(UT_OperatorCenter, getRootPassword_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::runCmd, []{
        return false;
    });
    stub.set_lamda(&OperatorCenter::executeProcess, []{
        return false;
    });


    bool isOk = OperatorCenter::getInstance()->getRootPassword();

    EXPECT_FALSE(isOk);
}

TEST(UT_OperatorCenter, getRootPassword_three)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::runCmd, []{
        return false;
    });
    stub.set_lamda(&OperatorCenter::executeProcess, []{
        return true;
    });


    bool isOk = OperatorCenter::getInstance()->getRootPassword();

    EXPECT_TRUE(isOk);
}

TEST(UT_OperatorCenter, executionShellCommand_one)
{
    QStringList output;
    int result = OperatorCenter::getInstance()->executionShellCommand("", output);

    EXPECT_TRUE(result == -1);
}

TEST(UT_OperatorCenter, executionShellCommand_two)
{
    QStringList output;
    int result = OperatorCenter::getInstance()->executionShellCommand("UT_TEST", output);

    EXPECT_TRUE(result != -1);
}
