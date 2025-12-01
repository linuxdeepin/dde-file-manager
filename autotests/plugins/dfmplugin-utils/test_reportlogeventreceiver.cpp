// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/reportlog/reportlogeventreceiver.h"
#include "plugins/common/dfmplugin-utils/reportlog/reportlogmanager.h"

#include <dfm-base/base/device/devicemanager.h>
#include <dfm-framework/dpf.h>

#include <QApplication>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_ReportLogEventReceiver : public testing::Test
{
protected:
    void SetUp() override
    {
        receiver = new ReportLogEventReceiver();
    }

    void TearDown() override
    {
        delete receiver;
        receiver = nullptr;
        stub.clear();
    }

    ReportLogEventReceiver *receiver { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_ReportLogEventReceiver, commit_CallsManager)
{
    bool managerCalled = false;
    QString capturedType;

    stub.set_lamda(ADDR(ReportLogManager, commit),
                   [&managerCalled, &capturedType](ReportLogManager *, const QString &type, const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                       managerCalled = true;
                       capturedType = type;
                   });

    receiver->commit("TestType", QVariantMap());

    EXPECT_TRUE(managerCalled);
    EXPECT_EQ(capturedType, "TestType");
}

TEST_F(UT_ReportLogEventReceiver, handleMenuData_CallsManager)
{
    bool managerCalled = false;
    QString capturedName;

    stub.set_lamda(ADDR(ReportLogManager, reportMenuData),
                   [&managerCalled, &capturedName](ReportLogManager *, const QString &name, const QList<QUrl> &) {
                       __DBG_STUB_INVOKE__
                       managerCalled = true;
                       capturedName = name;
                   });

    receiver->handleMenuData("TestMenu", QList<QUrl>());

    EXPECT_TRUE(managerCalled);
    EXPECT_EQ(capturedName, "TestMenu");
}

TEST_F(UT_ReportLogEventReceiver, handleBlockMountData_CallsManager)
{
    bool managerCalled = false;

    stub.set_lamda(ADDR(ReportLogManager, reportBlockMountData),
                   [&managerCalled](ReportLogManager *, const QString &, bool) {
                       __DBG_STUB_INVOKE__
                       managerCalled = true;
                   });

    receiver->handleBlockMountData("/dev/sda1", true);

    EXPECT_TRUE(managerCalled);
}

TEST_F(UT_ReportLogEventReceiver, handleMountNetworkResult_CallsManager)
{
    bool managerCalled = false;

    stub.set_lamda(ADDR(ReportLogManager, reportNetworkMountData),
                   [&managerCalled](ReportLogManager *, bool, dfmmount::DeviceError, const QString &) {
                       __DBG_STUB_INVOKE__
                       managerCalled = true;
                   });

    receiver->handleMountNetworkResult("smb://server", true, dfmmount::DeviceError::kNoError, "");

    EXPECT_TRUE(managerCalled);
}

TEST_F(UT_ReportLogEventReceiver, handleDesktopStartupData_CallsManager)
{
    bool managerCalled = false;

    stub.set_lamda(ADDR(ReportLogManager, reportDesktopStartUp),
                   [&managerCalled](ReportLogManager *, const QString &, const QVariant &) {
                       __DBG_STUB_INVOKE__
                       managerCalled = true;
                   });

    receiver->handleDesktopStartupData("testKey", QVariant("testData"));

    EXPECT_TRUE(managerCalled);
}
