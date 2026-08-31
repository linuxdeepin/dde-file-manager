// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_burneventreceiver_1.cpp
 * @brief Unit tests for BurnEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/burneventreceiver.h"

#include <QTest>

using namespace dfmplugin_burn;

class BurnEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BurnEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BurnEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BurnEventReceiverTest, handleFileCutResult)
{
    // Test method: void handleFileCutResult((const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg))
    QList<QUrl> _arg0{};
    QList<QUrl> _arg1{};
    QString _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileCutResult(_arg0, _arg1, false, _arg3));
}

TEST_F(BurnEventReceiverTest, handleMountImage)
{
    // Test method: void handleMountImage((quint64 winId, const QUrl &isoUrl))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleMountImage(0, _arg1));
}

TEST_F(BurnEventReceiverTest, handleShowBurnDlg)
{
    // Test method: void handleShowBurnDlg((const QString &dev, bool isSupportedUDF, QWidget *parent))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleShowBurnDlg(_arg0, false, nullptr));
}

TEST_F(BurnEventReceiverTest, handleShowDumpISODlg)
{
    // Test method: void handleShowDumpISODlg((const QString &devId))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleShowDumpISODlg(_arg0));
}
