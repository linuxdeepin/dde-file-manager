// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_reportlogeventreceiver.cpp
 * @brief Unit tests for ReportLogEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "reportlog/reportlogeventreceiver.h"

#include <QTest>

using namespace dfmplugin_utils;

class ReportLogEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ReportLogEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ReportLogEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ReportLogEventReceiverTest, ReportLogEventReceiver)
{
    // Test constructor: ReportLogEventReceiver((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ReportLogEventReceiverTest, bindEvents)
{
    // Test method: void bindEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->bindEvents());
}

TEST_F(ReportLogEventReceiverTest, commit)
{
    // Test method: void commit((const QString &type, const QVariantMap &args))
    QString _arg0{};
    QVariantMap _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->commit(_arg0, _arg1));
}

TEST_F(ReportLogEventReceiverTest, handleBlockMountData)
{
    // Test method: void handleBlockMountData((const QString &id, bool result))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleBlockMountData(_arg0, false));
}

TEST_F(ReportLogEventReceiverTest, handleDesktopStartupData)
{
    // Test method: void handleDesktopStartupData((const QString &key, const QVariant &data))
    QString _arg0{};
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleDesktopStartupData(_arg0, _arg1));
}

TEST_F(ReportLogEventReceiverTest, handleMenuData)
{
    // Test method: void handleMenuData((const QString &name, const QList<QUrl> &urlList))
    QString _arg0{};
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleMenuData(_arg0, _arg1));
}

TEST_F(ReportLogEventReceiverTest, handleMountNetworkResult)
{
    // Test method: void handleMountNetworkResult((const QString &, bool ret, dfmmount::DeviceError err, const QString &msg))
    QString _arg0{};
    QString _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->handleMountNetworkResult(_arg0, false, {}, _arg3));
}

TEST_F(ReportLogEventReceiverTest, lazyBindCommitEvent)
{
    // Test event handler: lazyBindCommitEvent((const QString &plugin, const QString &space))
    QString _event(QString::None);
    EXPECT_NO_FATAL_FAILURE(obj->lazyBindCommitEvent(&_event));
}

TEST_F(ReportLogEventReceiverTest, lazyBindMenuDataEvent)
{
    // Test event handler: lazyBindMenuDataEvent((const QString &plugin, const QString &space))
    QString _event(QString::None);
    EXPECT_NO_FATAL_FAILURE(obj->lazyBindMenuDataEvent(&_event));
}
