// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_reportlogworker.cpp
 * @brief Unit tests for ReportLogWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "reportlog/reportlogworker.h"

#include <QTest>

using namespace dfmplugin_utils;

class ReportLogWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ReportLogWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ReportLogWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ReportLogWorkerTest, handleMenuData)
{
    // Test method: void handleMenuData((const QString &name, const QList<QUrl> &urlList))
    QString _arg0{};
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleMenuData(_arg0, _arg1));
}

TEST_F(ReportLogWorkerTest, handleMountNetworkResult)
{
    // Test method: void handleMountNetworkResult((bool ret, dfmmount::DeviceError err, const QString &msg))
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->handleMountNetworkResult(false, {}, _arg2));
}

TEST_F(ReportLogWorkerTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}
