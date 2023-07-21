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
