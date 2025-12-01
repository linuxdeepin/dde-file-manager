// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/reportlog/virtualreportlogplugin.h"
#include "plugins/common/dfmplugin-utils/reportlog/reportlogeventreceiver.h"
#include "plugins/common/dfmplugin-utils/reportlog/reportlogmanager.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QTimer>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE

class UT_VirtualReportLogPlugin : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(ADDR(ReportLogManager, init),
                       [](ReportLogManager *) {
                           __DBG_STUB_INVOKE__
                       });

        stub.set_lamda(ADDR(ReportLogEventReceiver, bindEvents),
                       [](ReportLogEventReceiver *) {
                           __DBG_STUB_INVOKE__
                       });

        plugin = new VirtualReportLogPlugin();
    }

    void TearDown() override
    {
        delete plugin;
        plugin = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    VirtualReportLogPlugin *plugin { nullptr };
};

TEST_F(UT_VirtualReportLogPlugin, Constructor_EventReceiverInitialized)
{
    EXPECT_NE(plugin->eventReceiver.data(), nullptr);
}

TEST_F(UT_VirtualReportLogPlugin, initialize_CallsManagerInitAndBindEvents)
{
    bool managerInitCalled = false;
    bool bindEventsCalled = false;

    stub.set_lamda(ADDR(ReportLogManager, init),
                   [&managerInitCalled](ReportLogManager *) {
                       __DBG_STUB_INVOKE__
                       managerInitCalled = true;
                   });

    stub.set_lamda(ADDR(ReportLogEventReceiver, bindEvents),
                   [&bindEventsCalled](ReportLogEventReceiver *) {
                       __DBG_STUB_INVOKE__
                       bindEventsCalled = true;
                   });

    plugin->initialize();

    EXPECT_TRUE(managerInitCalled);
    EXPECT_TRUE(bindEventsCalled);
}

TEST_F(UT_VirtualReportLogPlugin, start_ReturnsTrue)
{
    stub.set_lamda(ADDR(ReportLogManager, commit),
                   [](ReportLogManager *, const QString &, const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(ADDR(DConfigManager, value),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = plugin->start();

    EXPECT_TRUE(result);
}
