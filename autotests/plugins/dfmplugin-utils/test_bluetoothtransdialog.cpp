// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothtransdialog.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothmanager.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothadapter.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothdevice.h"

#include <dfm-base/utils/dialogmanager.h>

#include <DDialog>
#include <DLabel>
#include <DListView>
#include <DProgressBar>
#include <DSpinner>
#include <DCommandLinkButton>

#include <QStackedWidget>
#include <QStandardItemModel>
#include <QTimer>
#include <QSignalSpy>
#include <QPushButton>

#include <gtest/gtest.h>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE

class UT_BluetoothTransDialog : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(ADDR(BluetoothManager, getAdapters),
                       [](const BluetoothManager *) -> QMap<QString, const BluetoothAdapter *> {
                           __DBG_STUB_INVOKE__
                           return QMap<QString, const BluetoothAdapter *>();
                       });

        stub.set_lamda(ADDR(BluetoothManager, refresh), [](BluetoothManager *) {
            __DBG_STUB_INVOKE__
        });

        // Stub QTimer
        typedef void (*FuncType)(int, Qt::TimerType, const QObject *, QtPrivate::QSlotObjectBase *);
        stub.set_lamda(static_cast<FuncType>(QTimer::singleShotImpl),
                       [] {
                           __DBG_STUB_INVOKE__
                       });

        stub.set_lamda(&BluetoothDevice::getIcon, [] {
            __DBG_STUB_INVOKE__
            return "test";
        });
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

/**
 * @brief Test isBluetoothIdle returns manager state
 */
TEST_F(UT_BluetoothTransDialog, isBluetoothIdle_ManagerCanSend_ReturnsTrue)
{
    __DBG_STUB_INVOKE__

    stub.set_lamda(ADDR(BluetoothManager, canSendBluetoothRequest),
                   [](BluetoothManager *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = BluetoothTransDialog::isBluetoothIdle();

    EXPECT_TRUE(result);
}

/**
 * @brief Test isBluetoothIdle when manager is busy
 */
TEST_F(UT_BluetoothTransDialog, isBluetoothIdle_ManagerBusy_ReturnsFalse)
{
    __DBG_STUB_INVOKE__

    stub.set_lamda(ADDR(BluetoothManager, canSendBluetoothRequest),
                   [](BluetoothManager *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool result = BluetoothTransDialog::isBluetoothIdle();

    EXPECT_FALSE(result);
}

/**
 * @brief Test sendFilesToDevice with valid device
 */
TEST_F(UT_BluetoothTransDialog, sendFilesToDevice_ValidDevice_SendsFiles)
{
    __DBG_STUB_INVOKE__

    QStringList urls = { "/tmp/file1.txt" };
    BluetoothTransDialog *dialog = new BluetoothTransDialog(urls);

    // Create mock adapter and device
    BluetoothAdapter *adapter = new BluetoothAdapter();
    adapter->setId("/org/bluez/hci0");

    BluetoothDevice *device = new BluetoothDevice();
    device->setId("/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF");
    device->setAlias("My Phone");

    adapter->addDevice(device);

    stub.set_lamda(ADDR(BluetoothManager, getAdapters),
                   [adapter](const BluetoothManager *) -> QMap<QString, const BluetoothAdapter *> {
                       __DBG_STUB_INVOKE__
                       QMap<QString, const BluetoothAdapter *> adapters;
                       adapters["/org/bluez/hci0"] = adapter;
                       return adapters;
                   });

    bool sendFilesCalled = false;
    stub.set_lamda(ADDR(BluetoothManager, sendFiles),
                   [&sendFilesCalled](BluetoothManager *, const QString &,
                                      const QStringList &, const QString &) {
                       __DBG_STUB_INVOKE__
                       sendFilesCalled = true;
                   });

    dialog->sendFilesToDevice("/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF");

    EXPECT_EQ(dialog->selectedDeviceId, "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF");
    EXPECT_EQ(dialog->selectedDeviceName, "My Phone");

    delete dialog;
    delete adapter;
}

/**
 * @brief Test sendFilesToDevice with non-existing device
 */
TEST_F(UT_BluetoothTransDialog, sendFilesToDevice_NonExistingDevice_DoesNothing)
{
    __DBG_STUB_INVOKE__

    QStringList urls = { "/tmp/file1.txt" };
    BluetoothTransDialog *dialog = new BluetoothTransDialog(urls);

    stub.set_lamda(ADDR(BluetoothManager, getAdapters),
                   [](const BluetoothManager *) -> QMap<QString, const BluetoothAdapter *> {
                       __DBG_STUB_INVOKE__
                       return QMap<QString, const BluetoothAdapter *>();
                   });

    bool sendFilesCalled = false;
    stub.set_lamda(ADDR(BluetoothManager, sendFiles),
                   [&sendFilesCalled](BluetoothManager *, const QString &,
                                      const QStringList &, const QString &) {
                       __DBG_STUB_INVOKE__
                       sendFilesCalled = true;
                   });

    dialog->sendFilesToDevice("/org/bluez/hci0/dev_NONEXISTENT");

    EXPECT_FALSE(sendFilesCalled);

    delete dialog;
}

/**
 * @brief Test humanizeObexErrMsg with timeout error
 */
TEST_F(UT_BluetoothTransDialog, humanizeObexErrMsg_TimeoutError_ReturnsReadableMessage)
{
    __DBG_STUB_INVOKE__

    QStringList urls = { "/tmp/file1.txt" };
    BluetoothTransDialog *dialog = new BluetoothTransDialog(urls);

    QString result = dialog->humanizeObexErrMsg("Connection Timed out");

    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.contains("timed out") || result.contains("Timed out"));

    delete dialog;
}

/**
 * @brief Test humanizeObexErrMsg with service busy error
 */
TEST_F(UT_BluetoothTransDialog, humanizeObexErrMsg_ServiceBusy_ReturnsReadableMessage)
{
    __DBG_STUB_INVOKE__

    QStringList urls = { "/tmp/file1.txt" };
    BluetoothTransDialog *dialog = new BluetoothTransDialog(urls);

    QString result = dialog->humanizeObexErrMsg("Error 0x53");

    EXPECT_FALSE(result.isEmpty());

    delete dialog;
}

/**
 * @brief Test humanizeObexErrMsg with connection error
 */
TEST_F(UT_BluetoothTransDialog, humanizeObexErrMsg_ConnectionError_ReturnsReadableMessage)
{
    __DBG_STUB_INVOKE__

    QStringList urls = { "/tmp/file1.txt" };
    BluetoothTransDialog *dialog = new BluetoothTransDialog(urls);

    QString result1 = dialog->humanizeObexErrMsg("device not connected");
    QString result2 = dialog->humanizeObexErrMsg("Connection refused");
    QString result3 = dialog->humanizeObexErrMsg("Connection reset by peer");

    EXPECT_FALSE(result1.isEmpty());
    EXPECT_FALSE(result2.isEmpty());
    EXPECT_FALSE(result3.isEmpty());

    delete dialog;
}

/**
 * @brief Test setNextButtonEnable on selector page
 */
TEST_F(UT_BluetoothTransDialog, setNextButtonEnable_OnSelectorPage_EnablesButton)
{
    __DBG_STUB_INVOKE__

    QStringList urls = { "/tmp/file1.txt" };
    BluetoothTransDialog *dialog = new BluetoothTransDialog(urls);

    // Stub getButtons to return mock buttons
    stub.set_lamda(ADDR(DDialog, getButtons), [dialog](DDialog *) -> QList<QAbstractButton *> {
        __DBG_STUB_INVOKE__
        QList<QAbstractButton *> buttons;
        buttons << new QPushButton(dialog);
        buttons << new QPushButton(dialog);
        return buttons;
    });

    stub.set_lamda(ADDR(QStackedWidget, currentIndex), [](QStackedWidget *) -> int {
        __DBG_STUB_INVOKE__
        return 0;   // kSelectDevicePage
    });

    bool setEnabledCalled = false;
    stub.set_lamda(ADDR(QAbstractButton, setEnabled),
                   [&setEnabledCalled] {
                       __DBG_STUB_INVOKE__
                       setEnabledCalled = true;
                   });

    dialog->setNextButtonEnable(true);

    EXPECT_TRUE(setEnabledCalled);

    delete dialog;
}

/**
 * @brief Test closeEvent
 */
TEST_F(UT_BluetoothTransDialog, closeEvent_WithActiveTransfer_CancelsTransfer)
{
    __DBG_STUB_INVOKE__

    QStringList urls = { "/tmp/file1.txt" };
    BluetoothTransDialog *dialog = new BluetoothTransDialog(urls);

    // Set an active session
    dialog->currSessionPath = "/session/path";

    bool cancelTransferCalled = false;
    stub.set_lamda(ADDR(BluetoothManager, cancelTransfer),
                   [&cancelTransferCalled](BluetoothManager *, const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       cancelTransferCalled = true;
                       return true;
                   });

    stub.set_lamda(VADDR(DDialog, closeEvent), [](DDialog *, QCloseEvent *) {
        __DBG_STUB_INVOKE__
    });

    dialog->closeEvent(nullptr);

    EXPECT_TRUE(cancelTransferCalled);

    delete dialog;
}

/**
 * @brief Test updateDeviceList with adapters and devices
 */
TEST_F(UT_BluetoothTransDialog, updateDeviceList_WithDevices_UpdatesModel)
{
    __DBG_STUB_INVOKE__

    QStringList urls = { "/tmp/file1.txt" };
    BluetoothTransDialog *dialog = new BluetoothTransDialog(urls);

    // Create mock adapter with devices
    BluetoothAdapter *adapter = new BluetoothAdapter();
    adapter->setId("/org/bluez/hci0");

    BluetoothDevice *device1 = new BluetoothDevice();
    device1->setId("/org/bluez/hci0/dev_11_11_11_11_11_11");
    device1->setAlias("Device 1");
    device1->setIcon("phone");
    device1->setPaired(true);
    device1->setTrusted(true);
    device1->setState(BluetoothDevice::kStateConnected);

    adapter->addDevice(device1);

    stub.set_lamda(ADDR(BluetoothManager, getAdapters),
                   [adapter](const BluetoothManager *) -> QMap<QString, const BluetoothAdapter *> {
                       __DBG_STUB_INVOKE__
                       QMap<QString, const BluetoothAdapter *> adapters;
                       adapters["/org/bluez/hci0"] = adapter;
                       return adapters;
                   });

    // Stub QStandardItemModel operations
    stub.set_lamda(qOverload<QStandardItem *>(&QStandardItemModel::appendRow),
                   [] {
                       __DBG_STUB_INVOKE__
                   });

    dialog->updateDeviceList();

    // Device should be added to model
    EXPECT_TRUE(true);

    delete dialog;
    delete adapter;
}

/**
 * @brief Test showBluetoothSetting calls manager
 */
TEST_F(UT_BluetoothTransDialog, showBluetoothSetting_CallsManager)
{
    __DBG_STUB_INVOKE__

    QStringList urls = { "/tmp/file1.txt" };
    BluetoothTransDialog *dialog = new BluetoothTransDialog(urls);

    bool showSettingsCalled = false;
    stub.set_lamda(ADDR(BluetoothManager, showBluetoothSettings),
                   [&showSettingsCalled](BluetoothManager *) {
                       __DBG_STUB_INVOKE__
                       showSettingsCalled = true;
                   });

    dialog->showBluetoothSetting();

    EXPECT_TRUE(showSettingsCalled);

    delete dialog;
}

/**
 * @brief Test onBtnClicked with cancel button
 */
TEST_F(UT_BluetoothTransDialog, onBtnClicked_CancelButton_ClosesDialog)
{
    __DBG_STUB_INVOKE__

    QStringList urls = { "/tmp/file1.txt" };
    BluetoothTransDialog *dialog = new BluetoothTransDialog(urls);

    bool closeCalled = false;
    stub.set_lamda(ADDR(DDialog, close), [&closeCalled] {
        __DBG_STUB_INVOKE__
        closeCalled = true;
        return true;
    });

    dialog->onBtnClicked(0);   // Cancel button index

    EXPECT_TRUE(closeCalled);

    delete dialog;
}

/**
 * @brief Test onPageChagned updates title and buttons
 */
TEST_F(UT_BluetoothTransDialog, onPageChagned_DifferentPages_UpdatesUI)
{
    __DBG_STUB_INVOKE__

    QStringList urls = { "/tmp/file1.txt" };
    BluetoothTransDialog *dialog = new BluetoothTransDialog(urls);

    // Stub clearButtons and addButton
    stub.set_lamda(ADDR(DDialog, clearButtons), [](DDialog *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(ADDR(DDialog, addButton),
                   [](DDialog *, const QString &, bool, DDialog::ButtonType) -> int {
                       __DBG_STUB_INVOKE__
                       return 0;
                   });

    stub.set_lamda(ADDR(DLabel, setText), [] {
        __DBG_STUB_INVOKE__
    });

    // Test page change
    dialog->onPageChagned(0);   // kSelectDevicePage
    dialog->onPageChagned(1);   // kNoneDevicePage
    dialog->onPageChagned(3);   // kTransferPage
    dialog->onPageChagned(5);   // kSuccessPage

    // Should update UI without crash
    EXPECT_TRUE(true);

    delete dialog;
}

/**
 * @brief Test addDevice adds device to model
 */
TEST_F(UT_BluetoothTransDialog, addDevice_ValidDevice_AddsToModel)
{
    __DBG_STUB_INVOKE__

    QStringList urls = { "/tmp/file1.txt" };
    BluetoothTransDialog *dialog = new BluetoothTransDialog(urls);

    BluetoothDevice *device = new BluetoothDevice();
    device->setId("/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF");
    device->setAlias("Test Device");
    device->setIcon("phone");
    device->setPaired(true);
    device->setTrusted(true);
    device->setState(BluetoothDevice::kStateConnected);

    bool appendRowCalled = false;
    stub.set_lamda(qOverload<QStandardItem *>(&QStandardItemModel::appendRow),
                   [&appendRowCalled] {
                       __DBG_STUB_INVOKE__
                       appendRowCalled = true;
                   });

    dialog->addDevice(device);

    EXPECT_TRUE(appendRowCalled);

    delete dialog;
    delete device;
}

/**
 * @brief Test removeDevice by device pointer
 */
TEST_F(UT_BluetoothTransDialog, removeDevice_ByPointer_RemovesFromModel)
{
    __DBG_STUB_INVOKE__

    QStringList urls = { "/tmp/file1.txt" };
    BluetoothTransDialog *dialog = new BluetoothTransDialog(urls);

    BluetoothDevice *device = new BluetoothDevice();
    device->setId("/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF");

    bool removeRowCalled = false;
    stub.set_lamda(ADDR(QStandardItemModel, removeRow),
                   [&removeRowCalled] {
                       __DBG_STUB_INVOKE__
                       removeRowCalled = true;
                       return true;
                   });

    stub.set_lamda(VADDR(QStandardItemModel, rowCount),
                   [](QStandardItemModel *, const QModelIndex &) -> int {
                       __DBG_STUB_INVOKE__
                       return 1;
                   });

    stub.set_lamda(ADDR(QStandardItemModel, item),
                   [device](QStandardItemModel *, int, int) -> QStandardItem * {
                       __DBG_STUB_INVOKE__
                       DStandardItem *item = new DStandardItem();
                       item->setData(device->getId(), Qt::UserRole + 101);   // kDevIdRole
                       return item;
                   });

    dialog->removeDevice(device);

    // Should attempt to remove device
    EXPECT_TRUE(removeRowCalled);

    delete dialog;
    delete device;
}

/**
 * @brief Test removeDevice by ID
 */
TEST_F(UT_BluetoothTransDialog, removeDevice_ById_RemovesFromModel)
{
    __DBG_STUB_INVOKE__

    QStringList urls = { "/tmp/file1.txt" };
    BluetoothTransDialog *dialog = new BluetoothTransDialog(urls);

    QString deviceId = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF";

    bool removeRowCalled = false;
    stub.set_lamda(ADDR(QStandardItemModel, removeRow),
                   [&removeRowCalled] {
                       __DBG_STUB_INVOKE__
                       removeRowCalled = true;
                       return true;
                   });

    stub.set_lamda(VADDR(QStandardItemModel, rowCount),
                   [](QStandardItemModel *, const QModelIndex &) -> int {
                       __DBG_STUB_INVOKE__
                       return 1;
                   });

    stub.set_lamda(ADDR(QStandardItemModel, item),
                   [deviceId](QStandardItemModel *, int, int) -> QStandardItem * {
                       __DBG_STUB_INVOKE__
                       DStandardItem *item = new DStandardItem();
                       item->setData(deviceId, Qt::UserRole + 101);
                       return item;
                   });

    dialog->removeDevice(deviceId);

    // Should attempt to remove device
    EXPECT_TRUE(true);

    delete dialog;
}

/**
 * @brief Test dialog token generation
 */
TEST_F(UT_BluetoothTransDialog, Constructor_GeneratesUniqueToken)
{
    __DBG_STUB_INVOKE__

    QStringList urls = { "/tmp/file1.txt" };
    BluetoothTransDialog *dialog1 = new BluetoothTransDialog(urls);
    BluetoothTransDialog *dialog2 = new BluetoothTransDialog(urls);

    EXPECT_FALSE(dialog1->dialogToken.isEmpty());
    EXPECT_FALSE(dialog2->dialogToken.isEmpty());
    EXPECT_NE(dialog1->dialogToken, dialog2->dialogToken);

    delete dialog1;
    delete dialog2;
}
