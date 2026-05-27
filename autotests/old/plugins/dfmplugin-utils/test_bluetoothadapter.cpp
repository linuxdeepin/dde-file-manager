// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothadapter.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothdevice.h"

#include <QSignalSpy>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;

class UT_BluetoothAdapter : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        adapter = new BluetoothAdapter();
    }

    virtual void TearDown() override
    {
        // Clean up all devices
        auto devices = adapter->getDevices();
        for (const BluetoothDevice *dev : devices) {
            const_cast<BluetoothDevice *>(dev)->deleteLater();
        }

        delete adapter;
        adapter = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    BluetoothAdapter *adapter { nullptr };
};

/**
 * @brief Test constructor creates valid object with default values
 */
TEST_F(UT_BluetoothAdapter, Constructor_CreatesValidObject)
{
    __DBG_STUB_INVOKE__

    EXPECT_NE(adapter, nullptr);
    EXPECT_TRUE(adapter->getId().isEmpty());
    EXPECT_TRUE(adapter->getName().isEmpty());
    EXPECT_FALSE(adapter->isPowered());
    EXPECT_EQ(adapter->getDevices().size(), 0);
}

/**
 * @brief Test setId and getId
 */
TEST_F(UT_BluetoothAdapter, setId_ValidId_IdIsSet)
{
    __DBG_STUB_INVOKE__

    QString testId = "/org/bluez/hci0";
    adapter->setId(testId);

    EXPECT_EQ(adapter->getId(), testId);
}

/**
 * @brief Test setName and getName with nameChanged signal
 */
TEST_F(UT_BluetoothAdapter, setName_ValidName_EmitsSignal)
{
    __DBG_STUB_INVOKE__

    QSignalSpy spy(adapter, &BluetoothAdapter::nameChanged);

    QString testName = "Bluetooth Adapter";
    adapter->setName(testName);

    EXPECT_EQ(adapter->getName(), testName);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), testName);
}

/**
 * @brief Test setName with same name doesn't emit signal
 */
TEST_F(UT_BluetoothAdapter, setName_SameName_NoSignalEmitted)
{
    __DBG_STUB_INVOKE__

    QString testName = "Bluetooth Adapter";
    adapter->setName(testName);

    QSignalSpy spy(adapter, &BluetoothAdapter::nameChanged);
    adapter->setName(testName);   // Set same name again

    EXPECT_EQ(spy.count(), 0);
}

/**
 * @brief Test setPowered and isPowered with poweredChanged signal
 */
TEST_F(UT_BluetoothAdapter, setPowered_True_EmitsSignal)
{
    __DBG_STUB_INVOKE__

    QSignalSpy spy(adapter, &BluetoothAdapter::poweredChanged);

    adapter->setPowered(true);

    EXPECT_TRUE(adapter->isPowered());
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(spy.takeFirst().at(0).toBool());
}

/**
 * @brief Test setPowered with same value doesn't emit signal
 */
TEST_F(UT_BluetoothAdapter, setPowered_SameValue_NoSignalEmitted)
{
    __DBG_STUB_INVOKE__

    adapter->setPowered(true);

    QSignalSpy spy(adapter, &BluetoothAdapter::poweredChanged);
    adapter->setPowered(true);   // Set same value again

    EXPECT_EQ(spy.count(), 0);
}

/**
 * @brief Test addDevice with new device
 */
TEST_F(UT_BluetoothAdapter, addDevice_NewDevice_DeviceAdded)
{
    __DBG_STUB_INVOKE__

    QSignalSpy spy(adapter, &BluetoothAdapter::deviceAdded);

    BluetoothDevice *device = new BluetoothDevice(adapter);
    device->setId("/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF");

    adapter->addDevice(device);

    EXPECT_EQ(adapter->getDevices().size(), 1);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).value<const BluetoothDevice *>(), device);
}

/**
 * @brief Test addDevice with duplicate device
 */
TEST_F(UT_BluetoothAdapter, addDevice_DuplicateDevice_NotAdded)
{
    __DBG_STUB_INVOKE__

    BluetoothDevice *device = new BluetoothDevice(adapter);
    device->setId("/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF");
    adapter->addDevice(device);

    QSignalSpy spy(adapter, &BluetoothAdapter::deviceAdded);

    // Try to add the same device again
    adapter->addDevice(device);

    EXPECT_EQ(adapter->getDevices().size(), 1);
    EXPECT_EQ(spy.count(), 0);   // No signal emitted for duplicate
}

/**
 * @brief Test addDevice with multiple devices
 */
TEST_F(UT_BluetoothAdapter, addDevice_MultipleDevices_AllAdded)
{
    __DBG_STUB_INVOKE__

    BluetoothDevice *device1 = new BluetoothDevice(adapter);
    device1->setId("/org/bluez/hci0/dev_11_11_11_11_11_11");

    BluetoothDevice *device2 = new BluetoothDevice(adapter);
    device2->setId("/org/bluez/hci0/dev_22_22_22_22_22_22");

    BluetoothDevice *device3 = new BluetoothDevice(adapter);
    device3->setId("/org/bluez/hci0/dev_33_33_33_33_33_33");

    adapter->addDevice(device1);
    adapter->addDevice(device2);
    adapter->addDevice(device3);

    EXPECT_EQ(adapter->getDevices().size(), 3);
}

/**
 * @brief Test deviceById with existing device
 */
TEST_F(UT_BluetoothAdapter, deviceById_ExistingDevice_ReturnsDevice)
{
    __DBG_STUB_INVOKE__

    QString deviceId = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF";

    BluetoothDevice *device = new BluetoothDevice(adapter);
    device->setId(deviceId);
    adapter->addDevice(device);

    const BluetoothDevice *result = adapter->deviceById(deviceId);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result, device);
    EXPECT_EQ(result->getId(), deviceId);
}

/**
 * @brief Test deviceById with non-existing device
 */
TEST_F(UT_BluetoothAdapter, deviceById_NonExistingDevice_ReturnsNull)
{
    __DBG_STUB_INVOKE__

    const BluetoothDevice *result = adapter->deviceById("/org/bluez/hci0/dev_NONEXISTENT");

    EXPECT_EQ(result, nullptr);
}

/**
 * @brief Test removeDevice with existing device
 */
TEST_F(UT_BluetoothAdapter, removeDevice_ExistingDevice_DeviceRemoved)
{
    __DBG_STUB_INVOKE__

    QString deviceId = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF";

    BluetoothDevice *device = new BluetoothDevice(adapter);
    device->setId(deviceId);
    adapter->addDevice(device);

    QSignalSpy spy(adapter, &BluetoothAdapter::deviceRemoved);

    adapter->removeDevice(deviceId);

    EXPECT_EQ(adapter->getDevices().size(), 0);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), deviceId);

    // Clean up device manually since it's removed from adapter
    device->deleteLater();
}

/**
 * @brief Test removeDevice with non-existing device
 */
TEST_F(UT_BluetoothAdapter, removeDevice_NonExistingDevice_NoChange)
{
    __DBG_STUB_INVOKE__

    QSignalSpy spy(adapter, &BluetoothAdapter::deviceRemoved);

    adapter->removeDevice("/org/bluez/hci0/dev_NONEXISTENT");

    EXPECT_EQ(adapter->getDevices().size(), 0);
    EXPECT_EQ(spy.count(), 0);
}

/**
 * @brief Test removeDevice with multiple devices
 */
TEST_F(UT_BluetoothAdapter, removeDevice_MultipleDevices_CorrectDeviceRemoved)
{
    __DBG_STUB_INVOKE__

    BluetoothDevice *device1 = new BluetoothDevice(adapter);
    device1->setId("/org/bluez/hci0/dev_11_11_11_11_11_11");

    BluetoothDevice *device2 = new BluetoothDevice(adapter);
    device2->setId("/org/bluez/hci0/dev_22_22_22_22_22_22");

    BluetoothDevice *device3 = new BluetoothDevice(adapter);
    device3->setId("/org/bluez/hci0/dev_33_33_33_33_33_33");

    adapter->addDevice(device1);
    adapter->addDevice(device2);
    adapter->addDevice(device3);

    adapter->removeDevice(device2->getId());

    EXPECT_EQ(adapter->getDevices().size(), 2);
    EXPECT_NE(adapter->deviceById(device1->getId()), nullptr);
    EXPECT_EQ(adapter->deviceById(device2->getId()), nullptr);
    EXPECT_NE(adapter->deviceById(device3->getId()), nullptr);

    // Clean up removed device
    device2->deleteLater();
}

/**
 * @brief Test getDevices returns correct map
 */
TEST_F(UT_BluetoothAdapter, getDevices_WithDevices_ReturnsCorrectMap)
{
    __DBG_STUB_INVOKE__

    BluetoothDevice *device1 = new BluetoothDevice(adapter);
    device1->setId("/org/bluez/hci0/dev_11_11_11_11_11_11");
    device1->setName("Device 1");

    BluetoothDevice *device2 = new BluetoothDevice(adapter);
    device2->setId("/org/bluez/hci0/dev_22_22_22_22_22_22");
    device2->setName("Device 2");

    adapter->addDevice(device1);
    adapter->addDevice(device2);

    QMap<QString, const BluetoothDevice *> devices = adapter->getDevices();

    EXPECT_EQ(devices.size(), 2);
    EXPECT_TRUE(devices.contains(device1->getId()));
    EXPECT_TRUE(devices.contains(device2->getId()));
    EXPECT_EQ(devices[device1->getId()], device1);
    EXPECT_EQ(devices[device2->getId()], device2);
}

/**
 * @brief Test complete adapter configuration
 */
TEST_F(UT_BluetoothAdapter, CompleteConfiguration_AllPropertiesSet)
{
    __DBG_STUB_INVOKE__

    QString id = "/org/bluez/hci0";
    QString name = "My Bluetooth Adapter";

    adapter->setId(id);
    adapter->setName(name);
    adapter->setPowered(true);

    BluetoothDevice *device = new BluetoothDevice(adapter);
    device->setId("/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF");
    adapter->addDevice(device);

    EXPECT_EQ(adapter->getId(), id);
    EXPECT_EQ(adapter->getName(), name);
    EXPECT_TRUE(adapter->isPowered());
    EXPECT_EQ(adapter->getDevices().size(), 1);
}

/**
 * @brief Test powered state transitions
 */
TEST_F(UT_BluetoothAdapter, setPowered_StateTransitions_EmitsSignals)
{
    __DBG_STUB_INVOKE__

    QSignalSpy spy(adapter, &BluetoothAdapter::poweredChanged);

    // Off -> On
    adapter->setPowered(true);
    EXPECT_EQ(spy.count(), 1);

    // On -> Off
    adapter->setPowered(false);
    EXPECT_EQ(spy.count(), 2);

    // Off -> On again
    adapter->setPowered(true);
    EXPECT_EQ(spy.count(), 3);
}

/**
 * @brief Test add and remove device operations sequence
 */
TEST_F(UT_BluetoothAdapter, AddRemoveSequence_WorksCorrectly)
{
    __DBG_STUB_INVOKE__

    QString deviceId = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF";

    // Add device
    BluetoothDevice *device = new BluetoothDevice(adapter);
    device->setId(deviceId);
    adapter->addDevice(device);
    EXPECT_EQ(adapter->getDevices().size(), 1);

    // Remove device
    adapter->removeDevice(deviceId);
    EXPECT_EQ(adapter->getDevices().size(), 0);

    // Add same device again (new instance)
    BluetoothDevice *device2 = new BluetoothDevice(adapter);
    device2->setId(deviceId);
    adapter->addDevice(device2);
    EXPECT_EQ(adapter->getDevices().size(), 1);

    // Clean up
    device->deleteLater();
}

/**
 * @brief Test empty adapter name
 */
TEST_F(UT_BluetoothAdapter, setName_EmptyName_AcceptedAndSetCorrectly)
{
    __DBG_STUB_INVOKE__

    adapter->setName("Test Adapter");
    adapter->setName("");

    EXPECT_TRUE(adapter->getName().isEmpty());
}

/**
 * @brief Test getDevices on empty adapter
 */
TEST_F(UT_BluetoothAdapter, getDevices_EmptyAdapter_ReturnsEmptyMap)
{
    __DBG_STUB_INVOKE__

    QMap<QString, const BluetoothDevice *> devices = adapter->getDevices();

    EXPECT_TRUE(devices.isEmpty());
    EXPECT_EQ(devices.size(), 0);
}
