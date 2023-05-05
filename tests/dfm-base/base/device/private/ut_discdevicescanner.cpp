// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include <dfm-base/base/device/private/discdevicescanner.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/base/device/devicemanager.h>

#include <QCoreApplication>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
using namespace GlobalServerDefines;
using namespace DiscDevice;

class UT_DiscDeviceScanner : public testing::Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_DiscDeviceScanner, bug_144643_Freezing)
{
    DiscDeviceScanner scanner;
    scanner.discDevIdGroup.append("test_id");
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo,
                   [](DeviceProxyManager *, const QString &id, bool) {
                       __DBG_STUB_INVOKE__
                       QVariantMap map;
                       if ("test_id" == id) {
                           map[DeviceProperty::kDevice] = "/dev/sr0";
                           map[DeviceProperty::kOptical] = true;
                       }
                       return map;
                   });
    // run in thread
    stub.set_lamda(VADDR(Scanner, run), [](Scanner *scanner) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(scanner->device, "/dev/sr0");
    });
    scanner.scanOpticalDisc();
}

TEST_F(UT_DiscDeviceScanner, bug_128271_ScannerEnable)
{
    DiscDeviceScanner scanner;
    stub.set_lamda(ADDR(QCoreApplication, applicationName), [] {
        __DBG_STUB_INVOKE__
        return "dde-desktop";
    });

    stub.set_lamda(ADDR(DeviceProxyManager, getAllBlockIds), [] {
        __DBG_STUB_INVOKE__
        return QStringList {};
    });

    stub.set_lamda(ADDR(DiscDeviceScanner, startScan), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(DiscDeviceScanner, updateScanState), [] {
        __DBG_STUB_INVOKE__
    });

    scanner.initialize();
    EXPECT_TRUE(scanner.discDevIdGroup.isEmpty());

    static constexpr char id[] { "/org/freedesktop/UDisks2/block_devices/sr0" };
    // blockDevPropertyChanged emited
    emit DevProxyMng->blockDevPropertyChanged(id,
                                              "Optical", QVariant::fromValue(true));
    EXPECT_EQ(scanner.discDevIdGroup.size(), 1);
    EXPECT_EQ(scanner.discDevIdGroup.first(), id);
    emit DevProxyMng->blockDevPropertyChanged(id,
                                              "Optical", QVariant::fromValue(false));
    EXPECT_TRUE(scanner.discDevIdGroup.isEmpty());

    // opticalDiscWorkStateChanged emited
    emit DevMngIns->opticalDiscWorkStateChanged(id, "", false);
    EXPECT_EQ(scanner.discDevIdGroup.size(), 1);
    EXPECT_EQ(scanner.discDevIdGroup.first(), id);

    emit DevMngIns->opticalDiscWorkStateChanged(id, "", true);
    EXPECT_TRUE(scanner.discDevIdGroup.isEmpty());
}
