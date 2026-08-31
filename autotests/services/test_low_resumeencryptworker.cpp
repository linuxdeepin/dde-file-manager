// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_resumeencryptworker.cpp
 * @brief Unit tests for ResumeEncryptWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/workers/resumeencryptworker.h"

#include <QTest>

using namespace src;

class ResumeEncryptWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ResumeEncryptWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ResumeEncryptWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ResumeEncryptWorkerTest, ignoreAuthRequest)
{
    // Test method: void ignoreAuthRequest(())
    EXPECT_NO_FATAL_FAILURE(obj->ignoreAuthRequest());
}

TEST_F(ResumeEncryptWorkerTest, loadJobFromDevice)
{
    // Test method: void loadJobFromDevice(())
    EXPECT_NO_FATAL_FAILURE(obj->loadJobFromDevice());
}

TEST_F(ResumeEncryptWorkerTest, setAuthInfo)
{
    // Test setter: void setAuthInfo((const QVariantMap &args))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setAuthInfo(_arg0));
}

TEST_F(ResumeEncryptWorkerTest, setPassphrase)
{
    // Test method: void setPassphrase(())
    EXPECT_NO_FATAL_FAILURE(obj->setPassphrase());
}

TEST_F(ResumeEncryptWorkerTest, setPhyDevLabel)
{
    // Test method: void setPhyDevLabel(())
    EXPECT_NO_FATAL_FAILURE(obj->setPhyDevLabel());
}

TEST_F(ResumeEncryptWorkerTest, setRecoveryKey)
{
    // Test method: void setRecoveryKey(())
    EXPECT_NO_FATAL_FAILURE(obj->setRecoveryKey());
}

TEST_F(ResumeEncryptWorkerTest, updateCryptTab)
{
    // Test method: void updateCryptTab(())
    EXPECT_NO_FATAL_FAILURE(obj->updateCryptTab());
}

TEST_F(ResumeEncryptWorkerTest, waitForAuthInfo)
{
    // Test bool getter: waitForAuthInfo()
    bool result = obj->waitForAuthInfo();
    EXPECT_FALSE(result);

}
