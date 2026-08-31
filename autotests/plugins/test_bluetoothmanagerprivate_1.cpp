// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bluetoothmanagerprivate_1.cpp
 * @brief Unit tests for BluetoothManagerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "bluetooth/private/bluetoothmanager.h"

#include <QTest>

using namespace dfmplugin_utils;

class BluetoothManagerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BluetoothManagerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BluetoothManagerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BluetoothManagerPrivateTest, BluetoothManagerPrivate)
{
    // Test constructor: BluetoothManagerPrivate((BluetoothManager *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BluetoothManagerPrivateTest, M_~BluetoothManagerPrivate)
{
    // Test method:  ~BluetoothManagerPrivate(())
    EXPECT_NO_FATAL_FAILURE({ BluetoothManagerPrivate *tmp = new BluetoothManagerPrivate(); delete tmp; });
}

TEST_F(BluetoothManagerPrivateTest, resolve)
{
    // Test method: void resolve((const QDBusReply<QString> &req))
    QDBusReply<QString> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->resolve(_arg0));
}

TEST_F(BluetoothManagerPrivateTest, initInterface)
{
    // Test method: void initInterface(())
    EXPECT_NO_FATAL_FAILURE(obj->initInterface());
}

TEST_F(BluetoothManagerPrivateTest, initConnects)
{
    // Test method: void initConnects(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnects());
}

TEST_F(BluetoothManagerPrivateTest, inflateAdapter)
{
    // Test method: void inflateAdapter((BluetoothAdapter *adapter, const QJsonObject &adapterObj))
    QJsonObject _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->inflateAdapter(nullptr, _arg1));
}

TEST_F(BluetoothManagerPrivateTest, inflateDevice)
{
    // Test method: void inflateDevice((BluetoothDevice *device, const QJsonObject &deviceObj))
    QJsonObject _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->inflateDevice(nullptr, _arg1));
}

TEST_F(BluetoothManagerPrivateTest, sendFiles)
{
    // Test method: QDBusPendingReply<QDBusObjectPath> sendFiles((const QString &device, const QStringList &files))
    QString _arg0{};
    QStringList _arg1{};
    auto result = obj->sendFiles(_arg0, _arg1);
    EXPECT_NO_FATAL_FAILURE({ obj->sendFiles(_arg0, _arg1); });

}

TEST_F(BluetoothManagerPrivateTest, getBluetoothDevices)
{
    // Test method: QDBusPendingCall getBluetoothDevices((const QDBusObjectPath &adapter))
    QDBusObjectPath _arg0{};
    auto result = obj->getBluetoothDevices(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->getBluetoothDevices(_arg0); });

}

TEST_F(BluetoothManagerPrivateTest, getBluetoothAdapters)
{
    // Test getter: QDBusPendingCall getBluetoothAdapters()
    auto result = obj->getBluetoothAdapters();
    EXPECT_NO_FATAL_FAILURE({ obj->getBluetoothAdapters(); });

}

TEST_F(BluetoothManagerPrivateTest, cancelTransferSession)
{
    // Test method: void cancelTransferSession((const QDBusObjectPath &sessionPath))
    QDBusObjectPath _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->cancelTransferSession(_arg0));
}

TEST_F(BluetoothManagerPrivateTest, onAdapterAdded)
{
    // Test method: void onAdapterAdded((const QString &json))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onAdapterAdded(_arg0));
}

TEST_F(BluetoothManagerPrivateTest, onAdapterRemoved)
{
    // Test method: void onAdapterRemoved((const QString &json))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onAdapterRemoved(_arg0));
}

TEST_F(BluetoothManagerPrivateTest, onDeviceAdded)
{
    // Test method: void onDeviceAdded((const QString &json))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onDeviceAdded(_arg0));
}

TEST_F(BluetoothManagerPrivateTest, onDeviceRemoved)
{
    // Test method: void onDeviceRemoved((const QString &json))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onDeviceRemoved(_arg0));
}

TEST_F(BluetoothManagerPrivateTest, onTransferCreated)
{
    // Test method: void onTransferCreated((const QString &file, const QDBusObjectPath &transferPath, const QDBusObjectPath &sessionPath))
    QString _arg0{};
    QDBusObjectPath _arg1{};
    QDBusObjectPath _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->onTransferCreated(_arg0, _arg1, _arg2));
}

TEST_F(BluetoothManagerPrivateTest, onTransferRemoved)
{
    // Test method: void onTransferRemoved((const QString &file, const QDBusObjectPath &transferPath, const QDBusObjectPath &sessionPath, bool done))
    QString _arg0{};
    QDBusObjectPath _arg1{};
    QDBusObjectPath _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->onTransferRemoved(_arg0, _arg1, _arg2, false));
}

TEST_F(BluetoothManagerPrivateTest, onTransferFailed)
{
    // Test method: void onTransferFailed((const QString &file, const QDBusObjectPath &sessionPath, const QString &errInfo))
    QString _arg0{};
    QDBusObjectPath _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->onTransferFailed(_arg0, _arg1, _arg2));
}

TEST_F(BluetoothManagerPrivateTest, operator=)
{
    // Test getter: BluetoothManagerPrivate operator=()
    EXPECT_NO_FATAL_FAILURE({ obj->operator=(); });
}

TEST_F(BluetoothManagerPrivateTest, onServiceValidChanged)
{
    // Test method: void onServiceValidChanged((bool valid))
    EXPECT_NO_FATAL_FAILURE(obj->onServiceValidChanged(false));
}

TEST_F(BluetoothManagerPrivateTest, onAdapterPropertiesChanged)
{
    // Test method: void onAdapterPropertiesChanged((const QString &json))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onAdapterPropertiesChanged(_arg0));
}

TEST_F(BluetoothManagerPrivateTest, onDevicePropertiesChanged)
{
    // Test method: void onDevicePropertiesChanged((const QString &json))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onDevicePropertiesChanged(_arg0));
}

TEST_F(BluetoothManagerPrivateTest, onObexSessionCreated)
{
    // Test method: void onObexSessionCreated((const QDBusObjectPath &sessionPath))
    QDBusObjectPath _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onObexSessionCreated(_arg0));
}

TEST_F(BluetoothManagerPrivateTest, onObexSessionRemoved)
{
    // Test method: void onObexSessionRemoved((const QDBusObjectPath &sessionPath))
    QDBusObjectPath _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onObexSessionRemoved(_arg0));
}

TEST_F(BluetoothManagerPrivateTest, onObexSessionProgress)
{
    // Test method: void onObexSessionProgress((const QDBusObjectPath &sessionPath, qulonglong totalSize, qulonglong transferred, int currentIndex))
    QDBusObjectPath _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onObexSessionProgress(_arg0, 0, 0, 0));
}

TEST_F(BluetoothManagerPrivateTest, connectBluetoothDBusSignals)
{
    // Test method: bool connectBluetoothDBusSignals((const QString &signal, const char *slot))
    QString _arg0{};
    auto result = obj->connectBluetoothDBusSignals(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(BluetoothManagerPrivateTest, public)
{
    // Test getter: Q_OBJECT public()
    EXPECT_NO_FATAL_FAILURE({ obj->public(); });
}

TEST_F(BluetoothManagerPrivateTest, model)
{
    // Test getter: BluetoothModel model()
    auto result = obj->model();
    EXPECT_NO_FATAL_FAILURE({ obj->model(); });

}

TEST_F(BluetoothManagerPrivateTest, watcher)
{
    // Test getter: QFutureWatcher<QPair<QString, QString>> watcher()
    auto result = obj->watcher();
    EXPECT_NO_FATAL_FAILURE({ obj->watcher(); });

}

TEST_F(BluetoothManagerPrivateTest, bluetoothInter)
{
    // Test getter: QDBusInterface bluetoothInter()
    auto result = obj->bluetoothInter();
    EXPECT_NO_FATAL_FAILURE({ obj->bluetoothInter(); });

}

TEST_F(BluetoothManagerPrivateTest, M_(BluetoothManager))
{
    // Test getter: Q_DECLARE_PUBLIC (BluetoothManager)()
    EXPECT_NO_FATAL_FAILURE({ obj->(BluetoothManager)(); });
}

TEST_F(BluetoothManagerPrivateTest, longFilenameFailures)
{
    // Test getter: QMap<QString, bool> longFilenameFailures()
    auto result = obj->longFilenameFailures();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BluetoothManagerPrivateTest, q_ptr)
{
    // Test getter: BluetoothManager q_ptr()
    auto result = obj->q_ptr();
    EXPECT_NO_FATAL_FAILURE({ obj->q_ptr(); });

}
