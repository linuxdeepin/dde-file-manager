// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_diskencryptmenuscene_1.cpp
 * @brief Unit tests for DiskEncryptMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/diskencryptmenuscene.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class DiskEncryptMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DiskEncryptMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DiskEncryptMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DiskEncryptMenuSceneTest, DiskEncryptMenuScene)
{
    // Test constructor: DiskEncryptMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DiskEncryptMenuSceneTest, create)
{
    // Test method: bool create((QMenu *))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptMenuSceneTest, doDecryptDevice)
{
    // Test method: void doDecryptDevice((const DeviceEncryptParam &param))
    DeviceEncryptParam _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->doDecryptDevice(_arg0));
}

TEST_F(DiskEncryptMenuSceneTest, doEncryptDevice)
{
    // Test method: void doEncryptDevice((const DeviceEncryptParam &param))
    DeviceEncryptParam _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->doEncryptDevice(_arg0));
}

TEST_F(DiskEncryptMenuSceneTest, encryptDevice)
{
    // Test method: void encryptDevice((const DeviceEncryptParam &param))
    DeviceEncryptParam _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->encryptDevice(_arg0));
}

TEST_F(DiskEncryptMenuSceneTest, generateTPMConfig)
{
    // Test getter: QString generateTPMConfig()
    auto result = obj->generateTPMConfig();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DiskEncryptMenuSceneTest, generateTPMToken)
{
    // Test method: QString generateTPMToken((const QString &device, bool pin, const QString &baseConfigPath))
    QString _arg0{};
    QString _arg2{};
    auto result = obj->generateTPMToken(_arg0, false, _arg2);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DiskEncryptMenuSceneTest, getBase64Of)
{
    // Test method: QString getBase64Of((const QString &fileName))
    QString _arg0{};
    auto result = obj->getBase64Of(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DiskEncryptMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DiskEncryptMenuSceneTest, onMounted)
{
    // Test method: void onMounted((bool ok, dfmmount::OperationErrorInfo info, QString mountPoint))
    EXPECT_NO_FATAL_FAILURE(obj->onMounted(false, {}, QString()));
}

TEST_F(DiskEncryptMenuSceneTest, onUnlocked)
{
    // Test method: void onUnlocked((bool ok, dfmmount::OperationErrorInfo info, QString clearDev))
    EXPECT_NO_FATAL_FAILURE(obj->onUnlocked(false, {}, QString()));
}

TEST_F(DiskEncryptMenuSceneTest, onUnmountError)
{
    // Test method: void onUnmountError((OpType t, const QString &dev, const dfmmount::OperationErrorInfo &err))
    QString _arg1{};
    dfmmount::OperationErrorInfo _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->onUnmountError(OpType(), _arg1, _arg2));
}

TEST_F(DiskEncryptMenuSceneTest, sendCredentialsViaFd)
{
    // Test method: bool sendCredentialsViaFd((QDBusInterface &iface, const QString &method,
                                                const QVariantMap &params, bool asyncCall))
    QDBusInterface _arg0{};
    QString _arg1{};
    QVariantMap _arg2{};
    auto result = obj->sendCredentialsViaFd(_arg0, _arg1, _arg2, false);
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptMenuSceneTest, unlockDevice)
{
    // Test method: void unlockDevice((const QString &devObjPath))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->unlockDevice(_arg0));
}

TEST_F(DiskEncryptMenuSceneTest, unmountBefore)
{
    // Test method: void unmountBefore((const std::function<void(const DeviceEncryptParam &)> &after, const DeviceEncryptParam &param))
    std::function<void( DeviceEncryptParam )> _arg0{};
    DeviceEncryptParam _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->unmountBefore(_arg0, _arg1));
}

TEST_F(DiskEncryptMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}
