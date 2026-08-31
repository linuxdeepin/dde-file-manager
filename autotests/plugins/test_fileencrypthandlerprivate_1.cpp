// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileencrypthandlerprivate_1.cpp
 * @brief Unit tests for FileEncryptHandlerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileencrypthandle.h"

#include <QTest>

using namespace dfmplugin_vault;

class FileEncryptHandlerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileEncryptHandlerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileEncryptHandlerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileEncryptHandlerPrivateTest, FileEncryptHandlerPrivate)
{
    // Test constructor: FileEncryptHandlerPrivate((FileEncryptHandle *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileEncryptHandlerPrivateTest, algoNameOfSupport)
{
    // Test getter: QStringList algoNameOfSupport()
    auto result = obj->algoNameOfSupport();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileEncryptHandlerPrivateTest, initEncryptType)
{
    // Test method: void initEncryptType(())
    EXPECT_NO_FATAL_FAILURE(obj->initEncryptType());
}

TEST_F(FileEncryptHandlerPrivateTest, isSupportAlgoName)
{
    // Test method: bool isSupportAlgoName((const QString &algoName))
    QString _arg0{};
    auto result = obj->isSupportAlgoName(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileEncryptHandlerPrivateTest, lockVaultProcess)
{
    // Test method: int lockVaultProcess((QString unlockFileDir, bool isForced))
    auto result = obj->lockVaultProcess(QString(), false);
    EXPECT_GE(result, 0);

}

TEST_F(FileEncryptHandlerPrivateTest, runVaultProcess)
{
    // Test method: int runVaultProcess((QString lockBaseDir, QString unlockFileDir, QString DSecureString, EncryptType type, int blockSize))
    auto result = obj->runVaultProcess(QString(), QString(), QString(), EncryptType(), 0);
    EXPECT_GE(result, 0);

}

TEST_F(FileEncryptHandlerPrivateTest, runVaultProcessAndGetOutput)
{
    // Test method: void runVaultProcessAndGetOutput((const QStringList &arguments, QString &standardError, QString &standardOutput))
    QStringList _arg0{};
    QString _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->runVaultProcessAndGetOutput(_arg0, _arg1, _arg2));
}

TEST_F(FileEncryptHandlerPrivateTest, setEnviroment)
{
    // Test setter: void setEnviroment((const QPair<QString, QString> &value))
    QPair<QString, QString> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setEnviroment(_arg0));
}

TEST_F(FileEncryptHandlerPrivateTest, syncGroupPolicyAlgoName)
{
    // Test method: void syncGroupPolicyAlgoName(())
    EXPECT_NO_FATAL_FAILURE(obj->syncGroupPolicyAlgoName());
}

TEST_F(FileEncryptHandlerPrivateTest, FileEncryptHandlerPrivate_Destructor)
{
    // Test method:  ~FileEncryptHandlerPrivate(())
    EXPECT_NO_FATAL_FAILURE({ FileEncryptHandlerPrivate *tmp = new FileEncryptHandlerPrivate(); delete tmp; });
}
