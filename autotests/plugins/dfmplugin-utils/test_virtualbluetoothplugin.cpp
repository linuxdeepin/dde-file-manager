// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/bluetooth/virtualbluetoothplugin.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothmanager.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothtransdialog.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothdevice.h"

#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-framework/dpf.h>

#include <QTimer>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE

class UT_VirtualBluetoothPlugin : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(&BluetoothDevice::getIcon, [] {
            __DBG_STUB_INVOKE__
            return "test";
        });

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

        plugin = new VirtualBluetoothPlugin();
    }

    virtual void TearDown() override
    {
        delete plugin;
        plugin = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    VirtualBluetoothPlugin *plugin { nullptr };
};

/**
 * @brief Test constructor creates valid object
 */
TEST_F(UT_VirtualBluetoothPlugin, Constructor_CreatesValidObject)
{
    __DBG_STUB_INVOKE__

    EXPECT_NE(plugin, nullptr);
}

/**
 * @brief Test initialize when not running as admin
 */
TEST_F(UT_VirtualBluetoothPlugin, initialize_NotAdmin_InitializesNormally)
{
    __DBG_STUB_INVOKE__

    bool singleShotCalled = false;

    stub.set_lamda(ADDR(SysInfoUtils, isOpenAsAdmin), []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    typedef void (*FuncType)(int, Qt::TimerType, const QObject *, QtPrivate::QSlotObjectBase *);
    stub.set_lamda(static_cast<FuncType>(QTimer::singleShotImpl),
                   [&singleShotCalled] {
                       __DBG_STUB_INVOKE__
                       singleShotCalled = true;
                   });

    plugin->initialize();

    EXPECT_TRUE(singleShotCalled);
}

/**
 * @brief Test start method returns true
 */
TEST_F(UT_VirtualBluetoothPlugin, start_ReturnsTrue)
{
    __DBG_STUB_INVOKE__

    bool result = plugin->start();

    EXPECT_TRUE(result);
}

/**
 * @brief Test bluetoothAvailable when bluetooth is enabled and has adapter
 */
TEST_F(UT_VirtualBluetoothPlugin, bluetoothAvailable_EnabledWithAdapter_ReturnsTrue)
{
    __DBG_STUB_INVOKE__

    stub.set_lamda(ADDR(BluetoothManager, bluetoothSendEnable), [](BluetoothManager *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(BluetoothManager, hasAdapter), [](BluetoothManager *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = plugin->bluetoothAvailable();

    EXPECT_TRUE(result);
}

/**
 * @brief Test sendFiles with empty paths
 */
TEST_F(UT_VirtualBluetoothPlugin, sendFiles_EmptyPaths_DoesNothing)
{
    stub.set_lamda(ADDR(BluetoothTransDialog, isBluetoothIdle), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QStringList emptyPaths;
    EXPECT_NO_THROW(plugin->sendFiles(emptyPaths, "device-id"));
}

/**
 * @brief Test sendFiles when bluetooth is busy
 */
TEST_F(UT_VirtualBluetoothPlugin, sendFiles_BluetoothBusy_ShowsMessage)
{
    bool messageShown = false;

    stub.set_lamda(ADDR(BluetoothTransDialog, isBluetoothIdle), []() -> bool {
        __DBG_STUB_INVOKE__
        return false;   // Bluetooth is busy
    });

    stub.set_lamda(qOverload<const QString &, const QString &, QString>(&DialogManager::showMessageDialog),
                   [&messageShown] {
                       __DBG_STUB_INVOKE__
                       messageShown = true;
                       return 0;
                   });

    QStringList paths = { "/tmp/file1.txt" };
    plugin->sendFiles(paths, "device-id");

    EXPECT_TRUE(messageShown);
}
