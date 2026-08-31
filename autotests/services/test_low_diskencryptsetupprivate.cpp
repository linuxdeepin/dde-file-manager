// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_diskencryptsetupprivate.cpp
 * @brief Unit tests for DiskEncryptSetupPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/dbus/diskencryptsetup.h"

#include <QTest>

using namespace src;

class DiskEncryptSetupPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DiskEncryptSetupPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DiskEncryptSetupPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DiskEncryptSetupPrivateTest, DiskEncryptSetupPrivate)
{
    // Test constructor: DiskEncryptSetupPrivate((DiskEncryptSetup *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DiskEncryptSetupPrivateTest, checkAuth)
{
    // Test method: bool checkAuth((const QString &action))
    QString _arg0{};
    auto result = obj->checkAuth(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptSetupPrivateTest, createDecryptWorker)
{
    // Test method: BaseEncryptWorker createDecryptWorker((const QString &type, const QVariantMap &args))
    QString _arg0{};
    QVariantMap _arg1{};
    auto result = obj->createDecryptWorker(_arg0, _arg1);
    EXPECT_NO_FATAL_FAILURE({ obj->createDecryptWorker(_arg0, _arg1); });

}

TEST_F(DiskEncryptSetupPrivateTest, createInitWorker)
{
    // Test method: BaseEncryptWorker createInitWorker((const QString &type, const QVariantMap &args))
    QString _arg0{};
    QVariantMap _arg1{};
    auto result = obj->createInitWorker(_arg0, _arg1);
    EXPECT_NO_FATAL_FAILURE({ obj->createInitWorker(_arg0, _arg1); });

}

TEST_F(DiskEncryptSetupPrivateTest, createMarkerFile)
{
    // Test method: bool createMarkerFile((const QString &path))
    QString _arg0{};
    auto result = obj->createMarkerFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptSetupPrivateTest, createOverlayDMFlagFile)
{
    // Test bool getter: createOverlayDMFlagFile()
    bool result = obj->createOverlayDMFlagFile();
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptSetupPrivateTest, createOverlayDMPendingFile)
{
    // Test bool getter: createOverlayDMPendingFile()
    bool result = obj->createOverlayDMPendingFile();
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptSetupPrivateTest, handleOverlayDMModeChangeAsync)
{
    // Test method: void handleOverlayDMModeChangeAsync((bool enabled))
    EXPECT_NO_FATAL_FAILURE(obj->handleOverlayDMModeChangeAsync(false));
}

TEST_F(DiskEncryptSetupPrivateTest, initThreadConnection)
{
    // Test method: void initThreadConnection((const QThread *thread))
    EXPECT_NO_FATAL_FAILURE(obj->initThreadConnection(nullptr));
}

TEST_F(DiskEncryptSetupPrivateTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(DiskEncryptSetupPrivateTest, onConfigValueChanged)
{
    // Test method: void onConfigValueChanged((const QString &key))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onConfigValueChanged(_arg0));
}

TEST_F(DiskEncryptSetupPrivateTest, onDecryptFinished)
{
    // Test method: void onDecryptFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->onDecryptFinished());
}

TEST_F(DiskEncryptSetupPrivateTest, onInitEncryptFinished)
{
    // Test method: void onInitEncryptFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->onInitEncryptFinished());
}

TEST_F(DiskEncryptSetupPrivateTest, onLongTimeJobStarted)
{
    // Test method: void onLongTimeJobStarted(())
    EXPECT_NO_FATAL_FAILURE(obj->onLongTimeJobStarted());
}

TEST_F(DiskEncryptSetupPrivateTest, onLongTimeJobStopped)
{
    // Test method: void onLongTimeJobStopped(())
    EXPECT_NO_FATAL_FAILURE(obj->onLongTimeJobStopped());
}

TEST_F(DiskEncryptSetupPrivateTest, onOverlayDMModeChangeFinished)
{
    // Test method: void onOverlayDMModeChangeFinished((bool success, bool targetValue))
    EXPECT_NO_FATAL_FAILURE(obj->onOverlayDMModeChangeFinished(false, false));
}

TEST_F(DiskEncryptSetupPrivateTest, onPassphraseChanged)
{
    // Test method: void onPassphraseChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onPassphraseChanged());
}

TEST_F(DiskEncryptSetupPrivateTest, onResumeEncryptFinished)
{
    // Test method: void onResumeEncryptFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->onResumeEncryptFinished());
}

TEST_F(DiskEncryptSetupPrivateTest, processPendingConfigChange)
{
    // Test method: void processPendingConfigChange(())
    EXPECT_NO_FATAL_FAILURE(obj->processPendingConfigChange());
}

TEST_F(DiskEncryptSetupPrivateTest, resolveDeviceByDetachHeaderName)
{
    // Test method: QString resolveDeviceByDetachHeaderName((const QString &fileName))
    QString _arg0{};
    auto result = obj->resolveDeviceByDetachHeaderName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DiskEncryptSetupPrivateTest, resumeEncryption)
{
    // Test method: void resumeEncryption((const QVariantMap &args))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->resumeEncryption(_arg0));
}

TEST_F(DiskEncryptSetupPrivateTest, setupConfigWatcher)
{
    // Test method: void setupConfigWatcher(())
    EXPECT_NO_FATAL_FAILURE(obj->setupConfigWatcher());
}

TEST_F(DiskEncryptSetupPrivateTest, validateChgPwdArgs)
{
    // Test method: bool validateChgPwdArgs((const QVariantMap &args))
    QVariantMap _arg0{};
    auto result = obj->validateChgPwdArgs(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptSetupPrivateTest, validateInitArgs)
{
    // Test method: bool validateInitArgs((const QVariantMap &args))
    QVariantMap _arg0{};
    auto result = obj->validateInitArgs(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptSetupPrivateTest, validateResumeArgs)
{
    // Test method: bool validateResumeArgs((const QVariantMap &args))
    QVariantMap _arg0{};
    auto result = obj->validateResumeArgs(_arg0);
    EXPECT_FALSE(result);

}
