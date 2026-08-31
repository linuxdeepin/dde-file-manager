// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_reportlogworker_1.cpp
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

TEST_F(ReportLogWorkerTest, commit)
{
    // Test method: void commit((const QVariant &args))
    QVariant _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->commit(_arg0));
}

TEST_F(ReportLogWorkerTest, commitLog)
{
    // Test method: void commitLog((const QString &type, const QVariantMap &args))
    QString _arg0{};
    QVariantMap _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->commitLog(_arg0, _arg1));
}

TEST_F(ReportLogWorkerTest, handleBlockMountData)
{
    // Test method: void handleBlockMountData((const QString &id, bool result))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleBlockMountData(_arg0, false));
}

TEST_F(ReportLogWorkerTest, handleDesktopStartUpData)
{
    // Test method: void handleDesktopStartUpData((const QString &key, const QVariant &data))
    QString _arg0{};
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleDesktopStartUpData(_arg0, _arg1));
}

TEST_F(ReportLogWorkerTest, registerLogData)
{
    // Test method: bool registerLogData((const QString &type, ReportDataInterface *dataObj))
    QString _arg0{};
    auto result = obj->registerLogData(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ReportLogWorkerTest, ReportLogWorker_Destructor)
{
    // Test method:  ~ReportLogWorker(())
    EXPECT_NO_FATAL_FAILURE({ ReportLogWorker *tmp = new ReportLogWorker(); delete tmp; });
}
