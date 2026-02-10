// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/reportlog/reportlogmanager.h"
#include "plugins/common/dfmplugin-utils/reportlog/reportlogworker.h"

#include <QThread>
#include <QSignalSpy>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;

class UT_ReportLogManager : public testing::Test
{
protected:
    void SetUp() override
    {
        manager = ReportLogManager::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

    ReportLogManager *manager { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_ReportLogManager, instance_ReturnsSingleton)
{
    ReportLogManager *instance1 = ReportLogManager::instance();
    ReportLogManager *instance2 = ReportLogManager::instance();

    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

TEST_F(UT_ReportLogManager, commit_EmitsSignal)
{
    QSignalSpy spy(manager, &ReportLogManager::requestCommitLog);

    manager->commit("TestType", QVariantMap());

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_ReportLogManager, reportMenuData_EmitsSignal)
{
    QSignalSpy spy(manager, &ReportLogManager::requestReportMenuData);

    manager->reportMenuData("TestMenu", QList<QUrl>());

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_ReportLogManager, reportBlockMountData_EmitsSignal)
{
    QSignalSpy spy(manager, &ReportLogManager::requestReportBlockMountData);

    manager->reportBlockMountData("/dev/sda1", true);

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_ReportLogManager, reportNetworkMountData_EmitsSignal)
{
    QSignalSpy spy(manager, &ReportLogManager::requestReportNetworkMountData);

    manager->reportNetworkMountData(true, dfmmount::DeviceError::kNoError, "");

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_ReportLogManager, reportDesktopStartUp_EmitsSignal)
{
    QSignalSpy spy(manager, &ReportLogManager::requestReportDesktopStartUp);

    manager->reportDesktopStartUp("testKey", QVariant("testData"));

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_ReportLogManager, init_WorkerInitFailed_ReturnsEarly)
{
    stub.set_lamda(ADDR(ReportLogWorker, init),
                   [](ReportLogWorker *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    manager->init();

    EXPECT_EQ(manager->reportWorkThread, nullptr);
}
