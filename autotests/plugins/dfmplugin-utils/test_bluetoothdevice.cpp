// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothdevice.h"

#include <QSignalSpy>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;

class UT_BluetoothDevice : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        device = new BluetoothDevice();
    }

    virtual void TearDown() override
    {
        delete device;
        device = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    BluetoothDevice *device { nullptr };
};

/**
 * @brief Test constructor creates valid object with default values
 */
TEST_F(UT_BluetoothDevice, Constructor_CreatesValidObject)
{
    __DBG_STUB_INVOKE__

    EXPECT_NE(device, nullptr);
    EXPECT_TRUE(device->getId().isEmpty());
    EXPECT_TRUE(device->getName().isEmpty());
    EXPECT_TRUE(device->getAlias().isEmpty());
    EXPECT_TRUE(device->getIcon().isEmpty());
    EXPECT_FALSE(device->isPaired());
    EXPECT_FALSE(device->isTrusted());
    EXPECT_EQ(device->getState(), BluetoothDevice::kStateUnavailable);
}

/**
 * @brief Test setId and getId
 */
TEST_F(UT_BluetoothDevice, setId_ValidId_IdIsSet)
{
    __DBG_STUB_INVOKE__

    QString testId = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF";
    device->setId(testId);

    EXPECT_EQ(device->getId(), testId);
}

/**
 * @brief Test setName and getName with nameChanged signal
 */
TEST_F(UT_BluetoothDevice, setName_ValidName_EmitsSignal)
{
    __DBG_STUB_INVOKE__

    QSignalSpy spy(device, &BluetoothDevice::nameChanged);

    QString testName = "Test Device";
    device->setName(testName);

    EXPECT_EQ(device->getName(), testName);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), testName);
}

/**
 * @brief Test setName with same name doesn't emit signal
 */
TEST_F(UT_BluetoothDevice, setName_SameName_NoSignalEmitted)
{
    __DBG_STUB_INVOKE__

    QString testName = "Test Device";
    device->setName(testName);

    QSignalSpy spy(device, &BluetoothDevice::nameChanged);
    device->setName(testName);   // Set same name again

    EXPECT_EQ(spy.count(), 0);
}

/**
 * @brief Test setAlias and getAlias with aliasChanged signal
 */
TEST_F(UT_BluetoothDevice, setAlias_ValidAlias_EmitsSignal)
{
    __DBG_STUB_INVOKE__

    QSignalSpy spy(device, &BluetoothDevice::aliasChanged);

    QString testAlias = "My Device";
    device->setAlias(testAlias);

    EXPECT_EQ(device->getAlias(), testAlias);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), testAlias);
}

/**
 * @brief Test setAlias with same alias doesn't emit signal
 */
TEST_F(UT_BluetoothDevice, setAlias_SameAlias_NoSignalEmitted)
{
    __DBG_STUB_INVOKE__

    QString testAlias = "My Device";
    device->setAlias(testAlias);

    QSignalSpy spy(device, &BluetoothDevice::aliasChanged);
    device->setAlias(testAlias);   // Set same alias again

    EXPECT_EQ(spy.count(), 0);
}

/**
 * @brief Test setIcon and getIcon
 */
TEST_F(UT_BluetoothDevice, setIcon_ValidIcon_IconIsSet)
{
    __DBG_STUB_INVOKE__

    QString testIcon = "phone";
    device->setIcon(testIcon);

    EXPECT_EQ(device->getIcon(), testIcon);
}

/**
 * @brief Test setIcon with different icon types
 */
TEST_F(UT_BluetoothDevice, setIcon_DifferentTypes_AllAccepted)
{
    __DBG_STUB_INVOKE__

    device->setIcon("phone");
    EXPECT_EQ(device->getIcon(), "phone");

    device->setIcon("computer");
    EXPECT_EQ(device->getIcon(), "computer");

    device->setIcon("headset");
    EXPECT_EQ(device->getIcon(), "headset");
}

/**
 * @brief Test setPaired and isPaired with pairedChanged signal
 */
TEST_F(UT_BluetoothDevice, setPaired_True_EmitsSignal)
{
    __DBG_STUB_INVOKE__

    QSignalSpy spy(device, &BluetoothDevice::pairedChanged);

    device->setPaired(true);

    EXPECT_TRUE(device->isPaired());
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(spy.takeFirst().at(0).toBool());
}

/**
 * @brief Test setPaired with same value doesn't emit signal
 */
TEST_F(UT_BluetoothDevice, setPaired_SameValue_NoSignalEmitted)
{
    __DBG_STUB_INVOKE__

    device->setPaired(true);

    QSignalSpy spy(device, &BluetoothDevice::pairedChanged);
    device->setPaired(true);   // Set same value again

    EXPECT_EQ(spy.count(), 0);
}

/**
 * @brief Test setTrusted and isTrusted with trustedChanged signal
 */
TEST_F(UT_BluetoothDevice, setTrusted_True_EmitsSignal)
{
    __DBG_STUB_INVOKE__

    QSignalSpy spy(device, &BluetoothDevice::trustedChanged);

    device->setTrusted(true);

    EXPECT_TRUE(device->isTrusted());
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(spy.takeFirst().at(0).toBool());
}

/**
 * @brief Test setTrusted with same value doesn't emit signal
 */
TEST_F(UT_BluetoothDevice, setTrusted_SameValue_NoSignalEmitted)
{
    __DBG_STUB_INVOKE__

    device->setTrusted(true);

    QSignalSpy spy(device, &BluetoothDevice::trustedChanged);
    device->setTrusted(true);   // Set same value again

    EXPECT_EQ(spy.count(), 0);
}

/**
 * @brief Test setState and getState with stateChanged signal
 */
TEST_F(UT_BluetoothDevice, setState_Available_EmitsSignal)
{
    __DBG_STUB_INVOKE__

    QSignalSpy spy(device, &BluetoothDevice::stateChanged);

    device->setState(BluetoothDevice::kStateAvailable);

    EXPECT_EQ(device->getState(), BluetoothDevice::kStateAvailable);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).value<BluetoothDevice::State>(),
              BluetoothDevice::kStateAvailable);
}

/**
 * @brief Test setState with Connected state
 */
TEST_F(UT_BluetoothDevice, setState_Connected_EmitsSignal)
{
    __DBG_STUB_INVOKE__

    QSignalSpy spy(device, &BluetoothDevice::stateChanged);

    device->setState(BluetoothDevice::kStateConnected);

    EXPECT_EQ(device->getState(), BluetoothDevice::kStateConnected);
    EXPECT_EQ(spy.count(), 1);
}

/**
 * @brief Test setState with same state doesn't emit signal
 */
TEST_F(UT_BluetoothDevice, setState_SameState_NoSignalEmitted)
{
    __DBG_STUB_INVOKE__

    device->setState(BluetoothDevice::kStateConnected);

    QSignalSpy spy(device, &BluetoothDevice::stateChanged);
    device->setState(BluetoothDevice::kStateConnected);   // Set same state again

    EXPECT_EQ(spy.count(), 0);
}

/**
 * @brief Test state transitions
 */
TEST_F(UT_BluetoothDevice, setState_StateTransitions_AllWork)
{
    __DBG_STUB_INVOKE__

    QSignalSpy spy(device, &BluetoothDevice::stateChanged);

    // Unavailable -> Available
    device->setState(BluetoothDevice::kStateAvailable);
    EXPECT_EQ(spy.count(), 1);

    // Available -> Connected
    device->setState(BluetoothDevice::kStateConnected);
    EXPECT_EQ(spy.count(), 2);

    // Connected -> Unavailable
    device->setState(BluetoothDevice::kStateUnavailable);
    EXPECT_EQ(spy.count(), 3);
}

/**
 * @brief Test complete device configuration
 */
TEST_F(UT_BluetoothDevice, CompleteConfiguration_AllPropertiesSet)
{
    __DBG_STUB_INVOKE__

    QString id = "/org/bluez/hci0/dev_11_22_33_44_55_66";
    QString name = "Samsung Galaxy";
    QString alias = "My Phone";
    QString icon = "phone";

    device->setId(id);
    device->setName(name);
    device->setAlias(alias);
    device->setIcon(icon);
    device->setPaired(true);
    device->setTrusted(true);
    device->setState(BluetoothDevice::kStateConnected);

    EXPECT_EQ(device->getId(), id);
    EXPECT_EQ(device->getName(), name);
    EXPECT_EQ(device->getAlias(), alias);
    EXPECT_EQ(device->getIcon(), icon);
    EXPECT_TRUE(device->isPaired());
    EXPECT_TRUE(device->isTrusted());
    EXPECT_EQ(device->getState(), BluetoothDevice::kStateConnected);
}

/**
 * @brief Test multiple signal emissions
 */
TEST_F(UT_BluetoothDevice, MultipleChanges_MultipleSignalsEmitted)
{
    __DBG_STUB_INVOKE__

    QSignalSpy nameSpy(device, &BluetoothDevice::nameChanged);
    QSignalSpy aliasSpy(device, &BluetoothDevice::aliasChanged);
    QSignalSpy pairedSpy(device, &BluetoothDevice::pairedChanged);
    QSignalSpy trustedSpy(device, &BluetoothDevice::trustedChanged);
    QSignalSpy stateSpy(device, &BluetoothDevice::stateChanged);

    device->setName("Name1");
    device->setName("Name2");
    device->setAlias("Alias1");
    device->setPaired(true);
    device->setTrusted(true);
    device->setState(BluetoothDevice::kStateConnected);

    EXPECT_EQ(nameSpy.count(), 2);
    EXPECT_EQ(aliasSpy.count(), 1);
    EXPECT_EQ(pairedSpy.count(), 1);
    EXPECT_EQ(trustedSpy.count(), 1);
    EXPECT_EQ(stateSpy.count(), 1);
}

/**
 * @brief Test empty strings
 */
TEST_F(UT_BluetoothDevice, EmptyStrings_AcceptedAndSetCorrectly)
{
    __DBG_STUB_INVOKE__

    device->setName("Test");
    device->setName("");
    EXPECT_TRUE(device->getName().isEmpty());

    device->setAlias("Test");
    device->setAlias("");
    EXPECT_TRUE(device->getAlias().isEmpty());

    device->setIcon("");
    EXPECT_TRUE(device->getIcon().isEmpty());
}
