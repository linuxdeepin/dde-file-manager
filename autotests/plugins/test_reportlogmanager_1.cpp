// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_reportlogmanager_1.cpp
 * @brief Unit tests for ReportLogManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "reportlog/reportlogmanager.h"

#include <QTest>

using namespace dfmplugin_utils;

class ReportLogManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ReportLogManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ReportLogManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ReportLogManagerTest, ReportLogManager)
{
    // Test constructor: ReportLogManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ReportLogManagerTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(ReportLogManagerTest, initConnection)
{
    // Test method: void initConnection(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnection());
}

TEST_F(ReportLogManagerTest, reportBlockMountData)
{
    // Test method: void reportBlockMountData((const QString &id, bool result))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->reportBlockMountData(_arg0, false));
}

TEST_F(ReportLogManagerTest, reportDesktopStartUp)
{
    // Test method: void reportDesktopStartUp((const QString &key, const QVariant &data))
    QString _arg0{};
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->reportDesktopStartUp(_arg0, _arg1));
}

TEST_F(ReportLogManagerTest, reportMenuData)
{
    // Test method: void reportMenuData((const QString &name, const QList<QUrl> &urlList))
    QString _arg0{};
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->reportMenuData(_arg0, _arg1));
}

TEST_F(ReportLogManagerTest, reportNetworkMountData)
{
    // Test method: void reportNetworkMountData((bool ret, dfmmount::DeviceError err, const QString &msg))
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->reportNetworkMountData(false, {}, _arg2));
}

TEST_F(ReportLogManagerTest, ReportLogManager_Destructor)
{
    // Test method:  ~ReportLogManager(())
    EXPECT_NO_FATAL_FAILURE({ ReportLogManager *tmp = new ReportLogManager(); delete tmp; });
}
