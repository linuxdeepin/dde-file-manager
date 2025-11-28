// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothmodel.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothadapter.h"

#include <QSignalSpy>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;

class UT_BluetoothModel : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        model = new BluetoothModel();
    }

    virtual void TearDown() override
    {
        // Clean up all adapters
        auto adapters = model->getAdapters();
        for (const BluetoothAdapter *adapter : adapters) {
            const_cast<BluetoothAdapter *>(adapter)->deleteLater();
        }

        delete model;
        model = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    BluetoothModel *model { nullptr };
};

/**
 * @brief Test constructor creates valid object with empty adapters
 */
TEST_F(UT_BluetoothModel, Constructor_CreatesValidObject)
{
    __DBG_STUB_INVOKE__

    EXPECT_NE(model, nullptr);
    EXPECT_EQ(model->getAdapters().size(), 0);
}

/**
 * @brief Test addAdapter with new adapter
 */
TEST_F(UT_BluetoothModel, addAdapter_NewAdapter_AdapterAdded)
{
    __DBG_STUB_INVOKE__

    QSignalSpy spy(model, &BluetoothModel::adapterAdded);

    BluetoothAdapter *adapter = new BluetoothAdapter(model);
    adapter->setId("/org/bluez/hci0");

    model->addAdapter(adapter);

    EXPECT_EQ(model->getAdapters().size(), 1);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).value<const BluetoothAdapter *>(), adapter);
}

/**
 * @brief Test addAdapter with duplicate adapter
 */
TEST_F(UT_BluetoothModel, addAdapter_DuplicateAdapter_DeletedAndNotAdded)
{
    __DBG_STUB_INVOKE__

    BluetoothAdapter *adapter1 = new BluetoothAdapter(model);
    adapter1->setId("/org/bluez/hci0");
    model->addAdapter(adapter1);

    QSignalSpy spy(model, &BluetoothModel::adapterAdded);

    // Try to add another adapter with same ID
    BluetoothAdapter *adapter2 = new BluetoothAdapter(model);
    adapter2->setId("/org/bluez/hci0");

    model->addAdapter(adapter2);

    // Should not add duplicate, and adapter2 should be deleted automatically
    EXPECT_EQ(model->getAdapters().size(), 1);
    EXPECT_EQ(spy.count(), 0);
}

/**
 * @brief Test addAdapter with multiple adapters
 */
TEST_F(UT_BluetoothModel, addAdapter_MultipleAdapters_AllAdded)
{
    __DBG_STUB_INVOKE__

    BluetoothAdapter *adapter1 = new BluetoothAdapter(model);
    adapter1->setId("/org/bluez/hci0");

    BluetoothAdapter *adapter2 = new BluetoothAdapter(model);
    adapter2->setId("/org/bluez/hci1");

    BluetoothAdapter *adapter3 = new BluetoothAdapter(model);
    adapter3->setId("/org/bluez/hci2");

    model->addAdapter(adapter1);
    model->addAdapter(adapter2);
    model->addAdapter(adapter3);

    EXPECT_EQ(model->getAdapters().size(), 3);
}

/**
 * @brief Test adapterById with existing adapter
 */
TEST_F(UT_BluetoothModel, adapterById_ExistingAdapter_ReturnsAdapter)
{
    __DBG_STUB_INVOKE__

    QString adapterId = "/org/bluez/hci0";

    BluetoothAdapter *adapter = new BluetoothAdapter(model);
    adapter->setId(adapterId);
    model->addAdapter(adapter);

    const BluetoothAdapter *result = model->adapterById(adapterId);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result, adapter);
    EXPECT_EQ(result->getId(), adapterId);
}

/**
 * @brief Test adapterById with non-existing adapter
 */
TEST_F(UT_BluetoothModel, adapterById_NonExistingAdapter_ReturnsNull)
{
    __DBG_STUB_INVOKE__

    const BluetoothAdapter *result = model->adapterById("/org/bluez/hci_NONEXISTENT");

    EXPECT_EQ(result, nullptr);
}

/**
 * @brief Test removeAdapater with existing adapter
 */
TEST_F(UT_BluetoothModel, removeAdapater_ExistingAdapter_AdapterRemoved)
{
    __DBG_STUB_INVOKE__

    QString adapterId = "/org/bluez/hci0";

    BluetoothAdapter *adapter = new BluetoothAdapter(model);
    adapter->setId(adapterId);
    model->addAdapter(adapter);

    QSignalSpy spy(model, &BluetoothModel::adapterRemoved);

    const BluetoothAdapter *removed = model->removeAdapater(adapterId);

    EXPECT_EQ(model->getAdapters().size(), 0);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).value<const BluetoothAdapter *>(), removed);
    EXPECT_EQ(removed, adapter);

    // Clean up removed adapter manually
    const_cast<BluetoothAdapter *>(removed)->deleteLater();
}

/**
 * @brief Test removeAdapater with non-existing adapter
 */
TEST_F(UT_BluetoothModel, removeAdapater_NonExistingAdapter_ReturnsNull)
{
    __DBG_STUB_INVOKE__

    QSignalSpy spy(model, &BluetoothModel::adapterRemoved);

    const BluetoothAdapter *removed = model->removeAdapater("/org/bluez/hci_NONEXISTENT");

    EXPECT_EQ(removed, nullptr);
    EXPECT_EQ(model->getAdapters().size(), 0);
    EXPECT_EQ(spy.count(), 0);
}

/**
 * @brief Test removeAdapater with multiple adapters
 */
TEST_F(UT_BluetoothModel, removeAdapater_MultipleAdapters_CorrectAdapterRemoved)
{
    __DBG_STUB_INVOKE__

    BluetoothAdapter *adapter1 = new BluetoothAdapter(model);
    adapter1->setId("/org/bluez/hci0");

    BluetoothAdapter *adapter2 = new BluetoothAdapter(model);
    adapter2->setId("/org/bluez/hci1");

    BluetoothAdapter *adapter3 = new BluetoothAdapter(model);
    adapter3->setId("/org/bluez/hci2");

    model->addAdapter(adapter1);
    model->addAdapter(adapter2);
    model->addAdapter(adapter3);

    const BluetoothAdapter *removed = model->removeAdapater(adapter2->getId());

    EXPECT_EQ(model->getAdapters().size(), 2);
    EXPECT_NE(model->adapterById(adapter1->getId()), nullptr);
    EXPECT_EQ(model->adapterById(adapter2->getId()), nullptr);
    EXPECT_NE(model->adapterById(adapter3->getId()), nullptr);
    EXPECT_EQ(removed, adapter2);

    // Clean up removed adapter
    const_cast<BluetoothAdapter *>(removed)->deleteLater();
}

/**
 * @brief Test getAdapters returns correct map
 */
TEST_F(UT_BluetoothModel, getAdapters_WithAdapters_ReturnsCorrectMap)
{
    __DBG_STUB_INVOKE__

    BluetoothAdapter *adapter1 = new BluetoothAdapter(model);
    adapter1->setId("/org/bluez/hci0");
    adapter1->setName("Adapter 1");

    BluetoothAdapter *adapter2 = new BluetoothAdapter(model);
    adapter2->setId("/org/bluez/hci1");
    adapter2->setName("Adapter 2");

    model->addAdapter(adapter1);
    model->addAdapter(adapter2);

    QMap<QString, const BluetoothAdapter *> adapters = model->getAdapters();

    EXPECT_EQ(adapters.size(), 2);
    EXPECT_TRUE(adapters.contains(adapter1->getId()));
    EXPECT_TRUE(adapters.contains(adapter2->getId()));
    EXPECT_EQ(adapters[adapter1->getId()], adapter1);
    EXPECT_EQ(adapters[adapter2->getId()], adapter2);
}

/**
 * @brief Test getAdapters on empty model
 */
TEST_F(UT_BluetoothModel, getAdapters_EmptyModel_ReturnsEmptyMap)
{
    __DBG_STUB_INVOKE__

    QMap<QString, const BluetoothAdapter *> adapters = model->getAdapters();

    EXPECT_TRUE(adapters.isEmpty());
    EXPECT_EQ(adapters.size(), 0);
}

/**
 * @brief Test add and remove adapter operations sequence
 */
TEST_F(UT_BluetoothModel, AddRemoveSequence_WorksCorrectly)
{
    __DBG_STUB_INVOKE__

    QString adapterId = "/org/bluez/hci0";

    // Add adapter
    BluetoothAdapter *adapter = new BluetoothAdapter(model);
    adapter->setId(adapterId);
    model->addAdapter(adapter);
    EXPECT_EQ(model->getAdapters().size(), 1);

    // Remove adapter
    const BluetoothAdapter *removed = model->removeAdapater(adapterId);
    EXPECT_EQ(model->getAdapters().size(), 0);

    // Add same adapter again (new instance)
    BluetoothAdapter *adapter2 = new BluetoothAdapter(model);
    adapter2->setId(adapterId);
    model->addAdapter(adapter2);
    EXPECT_EQ(model->getAdapters().size(), 1);

    // Clean up
    const_cast<BluetoothAdapter *>(removed)->deleteLater();
}

/**
 * @brief Test multiple signal emissions
 */
TEST_F(UT_BluetoothModel, MultipleOperations_MultipleSignalsEmitted)
{
    __DBG_STUB_INVOKE__

    QSignalSpy addedSpy(model, &BluetoothModel::adapterAdded);
    QSignalSpy removedSpy(model, &BluetoothModel::adapterRemoved);

    BluetoothAdapter *adapter1 = new BluetoothAdapter(model);
    adapter1->setId("/org/bluez/hci0");

    BluetoothAdapter *adapter2 = new BluetoothAdapter(model);
    adapter2->setId("/org/bluez/hci1");

    model->addAdapter(adapter1);
    model->addAdapter(adapter2);

    const BluetoothAdapter *removed = model->removeAdapater(adapter1->getId());

    EXPECT_EQ(addedSpy.count(), 2);
    EXPECT_EQ(removedSpy.count(), 1);

    // Clean up
    const_cast<BluetoothAdapter *>(removed)->deleteLater();
}

/**
 * @brief Test adapter with same ID override behavior
 */
TEST_F(UT_BluetoothModel, addAdapter_SameIdTwice_SecondOneDeleted)
{
    __DBG_STUB_INVOKE__

    QString adapterId = "/org/bluez/hci0";

    BluetoothAdapter *adapter1 = new BluetoothAdapter(model);
    adapter1->setId(adapterId);
    adapter1->setName("First Adapter");
    model->addAdapter(adapter1);

    BluetoothAdapter *adapter2 = new BluetoothAdapter(model);
    adapter2->setId(adapterId);
    adapter2->setName("Second Adapter");
    model->addAdapter(adapter2);   // Should be deleted automatically

    // Only the first adapter should remain
    EXPECT_EQ(model->getAdapters().size(), 1);
    const BluetoothAdapter *result = model->adapterById(adapterId);
    EXPECT_EQ(result->getName(), "First Adapter");
}

/**
 * @brief Test empty adapter ID handling
 */
TEST_F(UT_BluetoothModel, addAdapter_EmptyId_CanBeAdded)
{
    __DBG_STUB_INVOKE__

    BluetoothAdapter *adapter = new BluetoothAdapter(model);
    adapter->setId("");

    model->addAdapter(adapter);

    // Adapter with empty ID can be added
    EXPECT_EQ(model->getAdapters().size(), 1);
}

/**
 * @brief Test adapterById with empty ID
 */
TEST_F(UT_BluetoothModel, adapterById_EmptyId_ReturnsNull)
{
    __DBG_STUB_INVOKE__

    const BluetoothAdapter *result = model->adapterById("");

    EXPECT_EQ(result, nullptr);
}

/**
 * @brief Test complete model operations
 */
TEST_F(UT_BluetoothModel, CompleteOperations_AllWorkCorrectly)
{
    __DBG_STUB_INVOKE__

    // Add adapters
    BluetoothAdapter *adapter1 = new BluetoothAdapter(model);
    adapter1->setId("/org/bluez/hci0");
    adapter1->setName("Adapter 1");

    BluetoothAdapter *adapter2 = new BluetoothAdapter(model);
    adapter2->setId("/org/bluez/hci1");
    adapter2->setName("Adapter 2");

    model->addAdapter(adapter1);
    model->addAdapter(adapter2);
    EXPECT_EQ(model->getAdapters().size(), 2);

    // Query adapter
    const BluetoothAdapter *queried = model->adapterById("/org/bluez/hci0");
    EXPECT_NE(queried, nullptr);
    EXPECT_EQ(queried->getName(), "Adapter 1");

    // Remove one adapter
    const BluetoothAdapter *removed = model->removeAdapater("/org/bluez/hci1");
    EXPECT_EQ(model->getAdapters().size(), 1);
    EXPECT_NE(removed, nullptr);

    // Verify remaining adapter
    EXPECT_NE(model->adapterById("/org/bluez/hci0"), nullptr);
    EXPECT_EQ(model->adapterById("/org/bluez/hci1"), nullptr);

    // Clean up
    const_cast<BluetoothAdapter *>(removed)->deleteLater();
}
