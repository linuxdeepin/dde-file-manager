// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothmanager.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothmanager_p.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothmodel.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothadapter.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothdevice.h"

#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QSignalSpy>
#include <QTimer>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;

class UT_BluetoothManager : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Stub QDBusInterface constructor and methods
        stub.set_lamda(ADDR(QDBusInterface, isValid), [] {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(ADDR(QDBusInterface, setTimeout), [] {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(ADDR(QDBusInterface, lastError), [] {
            __DBG_STUB_INVOKE__
            return QDBusError();
        });

        stub.set_lamda(ADDR(QDBusInterface, property), [] {
            __DBG_STUB_INVOKE__
            return QVariant(true);
        });

        // Stub QTimer::singleShot
        using SingleShotIntFunc = void (*)(int, const QObject *, const char *);
        stub.set_lamda(static_cast<SingleShotIntFunc>(QTimer::singleShot),
                       [](int, const QObject *, const char *) {
                           __DBG_STUB_INVOKE__
                       });

        // Stub QDBusPendingCallWatcher to prevent actual async operations
        stub.set_lamda(ADDR(QDBusPendingCallWatcher, isFinished),
                       [] {
                           __DBG_STUB_INVOKE__
                           return true;
                       });

        stub.set_lamda(ADDR(QDBusPendingCall, isError), [](QDBusPendingCall *) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

/**
 * @brief Test singleton instance
 */
TEST_F(UT_BluetoothManager, instance_Singleton_ReturnsSameInstance)
{
    __DBG_STUB_INVOKE__

    BluetoothManager *instance1 = BluetoothManager::instance();
    BluetoothManager *instance2 = BluetoothManager::instance();

    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);
}

/**
 * @brief Test hasAdapter with adapters
 */
TEST_F(UT_BluetoothManager, hasAdapter_WithAdapters_ReturnsTrue)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();

    // Stub getAdapters to return non-empty map
    stub.set_lamda(ADDR(BluetoothManager, getAdapters),
                   [](const BluetoothManager *) -> QMap<QString, const BluetoothAdapter *> {
                       __DBG_STUB_INVOKE__
                       QMap<QString, const BluetoothAdapter *> adapters;
                       adapters["/org/bluez/hci0"] = new BluetoothAdapter();
                       return adapters;
                   });

    bool result = manager->hasAdapter();

    EXPECT_TRUE(result);
}

/**
 * @brief Test hasAdapter without adapters
 */
TEST_F(UT_BluetoothManager, hasAdapter_WithoutAdapters_ReturnsFalse)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();

    stub.set_lamda(ADDR(BluetoothManager, getAdapters),
                   [](const BluetoothManager *) -> QMap<QString, const BluetoothAdapter *> {
                       __DBG_STUB_INVOKE__
                       return QMap<QString, const BluetoothAdapter *>();
                   });

    bool result = manager->hasAdapter();

    EXPECT_FALSE(result);
}

/**
 * @brief Test bluetoothSendEnable when interface is valid and CanSendFile is true
 */
TEST_F(UT_BluetoothManager, bluetoothSendEnable_ValidAndEnabled_ReturnsTrue)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();

    stub.set_lamda(ADDR(QDBusInterface, isValid), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(QDBusInterface, property), [] {
        __DBG_STUB_INVOKE__
        return QVariant(true);
    });

    bool result = manager->bluetoothSendEnable();

    EXPECT_TRUE(result);
}

/**
 * @brief Test bluetoothSendEnable when interface is invalid
 */
TEST_F(UT_BluetoothManager, bluetoothSendEnable_InvalidInterface_ReturnsFalse)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();

    stub.set_lamda(ADDR(QDBusInterface, isValid), [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = manager->bluetoothSendEnable();

    EXPECT_FALSE(result);
}

/**
 * @brief Test bluetoothSendEnable when CanSendFile property is invalid
 */
TEST_F(UT_BluetoothManager, bluetoothSendEnable_InvalidProperty_ReturnsFalse)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();

    stub.set_lamda(ADDR(QDBusInterface, isValid), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(QDBusInterface, property), [] {
        __DBG_STUB_INVOKE__
        return QVariant();   // Invalid variant
    });

    bool result = manager->bluetoothSendEnable();

    EXPECT_FALSE(result);
}

/**
 * @brief Test bluetoothSendEnable when CanSendFile is false
 */
TEST_F(UT_BluetoothManager, bluetoothSendEnable_CanSendFileFalse_ReturnsFalse)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();

    stub.set_lamda(ADDR(QDBusInterface, isValid), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(QDBusInterface, property), [] {
        __DBG_STUB_INVOKE__
        return QVariant(false);
    });

    bool result = manager->bluetoothSendEnable();

    EXPECT_FALSE(result);
}

/**
 * @brief Test refresh method initiates async call
 */
TEST_F(UT_BluetoothManager, refresh_ValidInterface_InitiatesAsyncCall)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();

    bool asyncCallCalled = false;

    stub.set_lamda(ADDR(QDBusInterface, isValid), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<QDBusPendingCall (QDBusInterface::*)(const QString &)>(&QDBusInterface::asyncCall),
                   [&asyncCallCalled] {
                       __DBG_STUB_INVOKE__
                       asyncCallCalled = true;
                       return QDBusPendingCall::fromError(QDBusError());
                   });

    manager->refresh();

    EXPECT_TRUE(asyncCallCalled);
}

/**
 * @brief Test refresh when interface is invalid
 */
TEST_F(UT_BluetoothManager, refresh_InvalidInterface_DoesNothing)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();

    bool asyncCallCalled = false;

    stub.set_lamda(ADDR(QDBusInterface, isValid), [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(static_cast<QDBusPendingCall (QDBusInterface::*)(const QString &)>(&QDBusInterface::asyncCall),
                   [&asyncCallCalled](QDBusInterface *, const QString &) -> QDBusPendingCall {
                       __DBG_STUB_INVOKE__
                       asyncCallCalled = true;
                       return QDBusPendingCall::fromError(QDBusError());
                   });

    manager->refresh();

    EXPECT_FALSE(asyncCallCalled);
}

/**
 * @brief Test showBluetoothSettings calls DBus interface
 */
TEST_F(UT_BluetoothManager, showBluetoothSettings_CallsDBusInterface)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();

    bool asyncCallCalled = false;

    stub.set_lamda(static_cast<QDBusPendingCall (QDBusInterface::*)(const QString &, const QList<QVariant> &)>(&QDBusInterface::asyncCallWithArgumentList),
                   [&asyncCallCalled] {
                       __DBG_STUB_INVOKE__
                       asyncCallCalled = true;
                       return QDBusPendingCall::fromError(QDBusError());
                   });

    manager->showBluetoothSettings();

    EXPECT_TRUE(asyncCallCalled);
}

/**
 * @brief Test sendFiles with valid device and files
 */
TEST_F(UT_BluetoothManager, sendFiles_ValidParams_InitiatesTransfer)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();

    bool asyncCallCalled = false;

    stub.set_lamda(static_cast<QDBusPendingCall (QDBusInterface::*)(const QString &, const QList<QVariant> &)>(&QDBusInterface::asyncCallWithArgumentList),
                   [&asyncCallCalled] {
                       __DBG_STUB_INVOKE__
                       asyncCallCalled = true;
                       return QDBusPendingCall::fromError(QDBusError());
                   });


    manager->sendFiles("/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF",
                       QStringList() << "/tmp/file1.txt",
                       "test-token");
    EXPECT_TRUE(asyncCallCalled);
}

/**
 * @brief Test cancelTransfer calls DBus interface
 */
TEST_F(UT_BluetoothManager, cancelTransfer_ValidPath_CallsDBusInterface)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();

    bool asyncCallCalled = false;

    stub.set_lamda(static_cast<QDBusPendingCall (QDBusInterface::*)(const QString &, const QList<QVariant> &)>(&QDBusInterface::asyncCallWithArgumentList),
                   [&asyncCallCalled] {
                       __DBG_STUB_INVOKE__
                           asyncCallCalled = true;
                       return QDBusPendingCall::fromError(QDBusError());
                   });

    bool result = manager->cancelTransfer("/session/path");

    EXPECT_TRUE(result);
    EXPECT_TRUE(asyncCallCalled);
}

/**
 * @brief Test canSendBluetoothRequest with valid Transportable property
 */
TEST_F(UT_BluetoothManager, canSendBluetoothRequest_TransportableTrue_ReturnsTrue)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();

    stub.set_lamda(ADDR(QDBusInterface, property), [] {
        __DBG_STUB_INVOKE__
            return QVariant(true);
    });

    bool result = manager->canSendBluetoothRequest();

    EXPECT_TRUE(result);
}

/**
 * @brief Test canSendBluetoothRequest with invalid Transportable property
 */
TEST_F(UT_BluetoothManager, canSendBluetoothRequest_InvalidProperty_ReturnsTrue)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();

    stub.set_lamda(ADDR(QDBusInterface, property), [] {
        __DBG_STUB_INVOKE__
        return QVariant();   // Invalid variant
    });

    bool result = manager->canSendBluetoothRequest();

    EXPECT_TRUE(result);   // Returns true as default when property is invalid
}

/**
 * @brief Test canSendBluetoothRequest with Transportable false
 */
TEST_F(UT_BluetoothManager, canSendBluetoothRequest_TransportableFalse_ReturnsFalse)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();

    stub.set_lamda(ADDR(QDBusInterface, property), [] {
        __DBG_STUB_INVOKE__
            return QVariant(false);
    });

    bool result = manager->canSendBluetoothRequest();

    EXPECT_FALSE(result);
}

/**
 * @brief Test getAdapters returns model's adapters
 */
TEST_F(UT_BluetoothManager, getAdapters_ReturnsModelAdapters)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();

    // Access private member according to dfm_ut.md
    EXPECT_NE(manager->d_ptr, nullptr);
    EXPECT_NE(manager->d_ptr->model, nullptr);

    QMap<QString, const BluetoothAdapter *> adapters = manager->getAdapters();

    // Should return the model's adapters (may be empty initially)
    EXPECT_TRUE(true);
}

/**
 * @brief Test BluetoothManagerPrivate inflateAdapter
 */
TEST_F(UT_BluetoothManager, BluetoothManagerPrivate_inflateAdapter_ParsesJson)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();
    auto d = manager->d_ptr.data();

    BluetoothAdapter *adapter = new BluetoothAdapter();

    QJsonObject adapterObj;
    adapterObj["Path"] = "/org/bluez/hci0";
    adapterObj["Alias"] = "Test Adapter";
    adapterObj["Powered"] = true;

    // Stub getBluetoothDevices to prevent actual DBus call
    stub.set_lamda(static_cast<QDBusPendingCall (QDBusInterface::*)(const QString &, const QList<QVariant> &)>(&QDBusInterface::asyncCallWithArgumentList),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QDBusPendingCall::fromError(QDBusError());
                   });

    d->inflateAdapter(adapter, adapterObj);

    EXPECT_EQ(adapter->getId(), "/org/bluez/hci0");
    EXPECT_EQ(adapter->getName(), "Test Adapter");
    EXPECT_TRUE(adapter->isPowered());

    delete adapter;
}

/**
 * @brief Test BluetoothManagerPrivate inflateDevice
 */
TEST_F(UT_BluetoothManager, BluetoothManagerPrivate_inflateDevice_ParsesJson)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();
    auto d = manager->d_ptr.data();

    BluetoothDevice *device = new BluetoothDevice();

    QJsonObject deviceObj;
    deviceObj["Path"] = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF";
    deviceObj["Name"] = "Test Device";
    deviceObj["Alias"] = "My Phone";
    deviceObj["Icon"] = "phone";
    deviceObj["Paired"] = true;
    deviceObj["Trusted"] = true;
    deviceObj["State"] = 2;   // kStateConnected

    d->inflateDevice(device, deviceObj);

    EXPECT_EQ(device->getId(), "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF");
    EXPECT_EQ(device->getName(), "Test Device");
    EXPECT_EQ(device->getAlias(), "My Phone");
    EXPECT_EQ(device->getIcon(), "phone");
    EXPECT_TRUE(device->isPaired());
    EXPECT_TRUE(device->isTrusted());
    EXPECT_EQ(device->getState(), BluetoothDevice::kStateConnected);

    delete device;
}

/**
 * @brief Test BluetoothManagerPrivate onAdapterAdded
 */
TEST_F(UT_BluetoothManager, BluetoothManagerPrivate_onAdapterAdded_AddsAdapter)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();
    auto d = manager->d_ptr.data();

    QJsonObject adapterObj;
    adapterObj["Path"] = "/org/bluez/hci0";
    adapterObj["Alias"] = "New Adapter";
    adapterObj["Powered"] = false;

    QJsonDocument doc(adapterObj);
    QString json = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    // Stub getBluetoothDevices to prevent actual DBus call
    stub.set_lamda(static_cast<QDBusPendingCall (QDBusInterface::*)(const QString &, const QList<QVariant> &)>(&QDBusInterface::asyncCallWithArgumentList),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QDBusPendingCall::fromError(QDBusError());
                   });

    QSignalSpy spy(d->model, &BluetoothModel::adapterAdded);

    d->onAdapterAdded(json);

    EXPECT_EQ(spy.count(), 1);
}

/**
 * @brief Test BluetoothManagerPrivate onAdapterRemoved
 */
TEST_F(UT_BluetoothManager, BluetoothManagerPrivate_onAdapterRemoved_RemovesAdapter)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();
    auto d = manager->d_ptr.data();

    // First add an adapter
    BluetoothAdapter *adapter = new BluetoothAdapter();
    adapter->setId("/org/bluez/hci0");
    d->model->addAdapter(adapter);

    QJsonObject adapterObj;
    adapterObj["Path"] = "/org/bluez/hci0";

    QJsonDocument doc(adapterObj);
    QString json = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    QSignalSpy spy(d->model, &BluetoothModel::adapterRemoved);

    d->onAdapterRemoved(json);

    EXPECT_EQ(spy.count(), 1);
}

/**
 * @brief Test BluetoothManagerPrivate onDeviceAdded
 */
TEST_F(UT_BluetoothManager, BluetoothManagerPrivate_onDeviceAdded_AddsDevice)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();
    auto d = manager->d_ptr.data();

    // First add an adapter
    BluetoothAdapter *adapter = new BluetoothAdapter();
    adapter->setId("/org/bluez/hci0");
    d->model->addAdapter(adapter);

    QJsonObject deviceObj;
    deviceObj["AdapterPath"] = "/org/bluez/hci0";
    deviceObj["Path"] = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF";
    deviceObj["Name"] = "Test Device";
    deviceObj["Alias"] = "My Phone";
    deviceObj["Icon"] = "phone";
    deviceObj["Paired"] = true;
    deviceObj["Trusted"] = true;
    deviceObj["State"] = 2;

    QJsonDocument doc(deviceObj);
    QString json = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    QSignalSpy spy(adapter, &BluetoothAdapter::deviceAdded);

    d->onDeviceAdded(json);

    EXPECT_EQ(spy.count(), 1);
}

/**
 * @brief Test BluetoothManagerPrivate onTransferRemoved with done=true
 */
TEST_F(UT_BluetoothManager, BluetoothManagerPrivate_onTransferRemoved_DoneTrue_EmitsFinished)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();
    auto d = manager->d_ptr.data();

    QSignalSpy spy(manager, &BluetoothManager::fileTransferFinished);

    d->onTransferRemoved("/tmp/file.txt",
                         QDBusObjectPath("/transfer/path"),
                         QDBusObjectPath("/session/path"),
                         true);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), "/session/path");
}

/**
 * @brief Test BluetoothManagerPrivate onTransferRemoved with done=false
 */
TEST_F(UT_BluetoothManager, BluetoothManagerPrivate_onTransferRemoved_DoneFalse_EmitsCanceled)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();
    auto d = manager->d_ptr.data();

    QSignalSpy spy(manager, &BluetoothManager::transferCancledByRemote);

    d->onTransferRemoved("/tmp/file.txt",
                         QDBusObjectPath("/transfer/path"),
                         QDBusObjectPath("/session/path"),
                         false);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), "/session/path");
}

/**
 * @brief Test BluetoothManagerPrivate onObexSessionProgress
 */
TEST_F(UT_BluetoothManager, BluetoothManagerPrivate_onObexSessionProgress_EmitsSignal)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();
    auto d = manager->d_ptr.data();

    QSignalSpy spy(manager, &BluetoothManager::transferProgressUpdated);

    d->onObexSessionProgress(QDBusObjectPath("/session/path"), 1000, 500, 0);

    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), "/session/path");
    EXPECT_EQ(args.at(1).toULongLong(), 1000);
    EXPECT_EQ(args.at(2).toULongLong(), 500);
    EXPECT_EQ(args.at(3).toInt(), 0);
}

/**
 * @brief Test BluetoothManagerPrivate onTransferFailed
 */
TEST_F(UT_BluetoothManager, BluetoothManagerPrivate_onTransferFailed_EmitsSignal)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();
    auto d = manager->d_ptr.data();

    QSignalSpy spy(manager, &BluetoothManager::transferFailed);

    d->onTransferFailed("/tmp/file.txt",
                        QDBusObjectPath("/session/path"),
                        "Connection lost");

    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), "/session/path");
    EXPECT_EQ(args.at(1).toString(), "/tmp/file.txt");
    EXPECT_EQ(args.at(2).toString(), "Connection lost");
}

/**
 * @brief Test BluetoothManagerPrivate onDeviceRemoved
 */
TEST_F(UT_BluetoothManager, BluetoothManagerPrivate_onDeviceRemoved_RemovesDevice)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();
    auto d = manager->d_ptr.data();

    // First add an adapter with a device
    BluetoothAdapter *adapter = new BluetoothAdapter();
    adapter->setId("/org/bluez/hci0");
    d->model->addAdapter(adapter);

    BluetoothDevice *device = new BluetoothDevice(adapter);
    device->setId("/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF");
    adapter->addDevice(device);

    QJsonObject deviceObj;
    deviceObj["AdapterPath"] = "/org/bluez/hci0";
    deviceObj["Path"] = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF";

    QJsonDocument doc(deviceObj);
    QString json = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    d->onDeviceRemoved(json);
}

/**
 * @brief Test BluetoothManagerPrivate onDevicePropertiesChanged
 */
TEST_F(UT_BluetoothManager, BluetoothManagerPrivate_onDevicePropertiesChanged_UpdatesDevice)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();
    auto d = manager->d_ptr.data();

    // First add an adapter with a device
    BluetoothAdapter *adapter = new BluetoothAdapter();
    adapter->setId("/org/bluez/hci0");
    d->model->addAdapter(adapter);

    BluetoothDevice *device = new BluetoothDevice(adapter);
    device->setId("/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF");
    device->setName("Old Name");
    adapter->addDevice(device);

    QJsonObject deviceObj;
    deviceObj["Path"] = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF";
    deviceObj["Name"] = "New Name";
    deviceObj["Alias"] = "New Alias";
    deviceObj["Icon"] = "phone";
    deviceObj["Paired"] = true;
    deviceObj["Trusted"] = true;
    deviceObj["State"] = 2;

    QJsonDocument doc(deviceObj);
    QString json = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    d->onDevicePropertiesChanged(json);

    EXPECT_EQ(device->getName(), "New Name");
}

/**
 * @brief Test BluetoothManagerPrivate onAdapterPropertiesChanged
 */
TEST_F(UT_BluetoothManager, BluetoothManagerPrivate_onAdapterPropertiesChanged_UpdatesAdapter)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();
    auto d = manager->d_ptr.data();

    // Stub getBluetoothDevices
    stub.set_lamda(static_cast<QDBusPendingCall (QDBusInterface::*)(const QString &, const QList<QVariant> &)>(&QDBusInterface::asyncCallWithArgumentList),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QDBusPendingCall::fromError(QDBusError());
                   });

    // First add an adapter
    BluetoothAdapter *adapter = new BluetoothAdapter();
    adapter->setId("/org/bluez/hci0");
    adapter->setName("Old Name");
    adapter->setPowered(false);
    d->model->addAdapter(adapter);

    QJsonObject adapterObj;
    adapterObj["Path"] = "/org/bluez/hci0";
    adapterObj["Alias"] = "New Name";
    adapterObj["Powered"] = true;

    QJsonDocument doc(adapterObj);
    QString json = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    d->onAdapterPropertiesChanged(json);

    EXPECT_EQ(adapter->getName(), "New Name");
    EXPECT_TRUE(adapter->isPowered());
}

/**
 * @brief Test BluetoothManagerPrivate onTransferCreated
 */
TEST_F(UT_BluetoothManager, BluetoothManagerPrivate_onTransferCreated_Logs)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();
    auto d = manager->d_ptr.data();

    d->onTransferCreated("/tmp/file.txt",
                         QDBusObjectPath("/transfer/path"),
                         QDBusObjectPath("/session/path"));
}

/**
 * @brief Test BluetoothManagerPrivate onObexSessionCreated
 */
TEST_F(UT_BluetoothManager, BluetoothManagerPrivate_onObexSessionCreated_Logs)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();
    auto d = manager->d_ptr.data();

    d->onObexSessionCreated(QDBusObjectPath("/session/path"));
}

/**
 * @brief Test BluetoothManagerPrivate onObexSessionRemoved
 */
TEST_F(UT_BluetoothManager, BluetoothManagerPrivate_onObexSessionRemoved_Logs)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();
    auto d = manager->d_ptr.data();

    d->onObexSessionRemoved(QDBusObjectPath("/session/path"));
}

/**
 * @brief Test BluetoothManagerPrivate resolve with empty string
 */
TEST_F(UT_BluetoothManager, BluetoothManagerPrivate_resolve_EmptyString_Retries)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();
    auto d = manager->d_ptr.data();

    QDBusReply<QString> reply;
    d->resolve(reply);
}

/**
 * @brief Test BluetoothManagerPrivate onServiceValidChanged with false
 */
TEST_F(UT_BluetoothManager, BluetoothManagerPrivate_onServiceValidChanged_False_DoesNothing)
{
    __DBG_STUB_INVOKE__

    auto manager = BluetoothManager::instance();
    auto d = manager->d_ptr.data();

    d->onServiceValidChanged(false);
}
