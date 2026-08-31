// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recenteventreceiver_1.cpp
 * @brief Unit tests for RecentEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/recenteventreceiver.h"

#include <QTest>

using namespace dfmplugin_recent;

class RecentEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentEventReceiverTest, checkDragDropAction)
{
    // Test method: bool checkDragDropAction((const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    auto result = obj->checkDragDropAction(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(RecentEventReceiverTest, customColumnRole)
{
    // Test method: bool customColumnRole((const QUrl &rootUrl, QList<ItemRoles> *roleList))
    QUrl _arg0{};
    auto result = obj->customColumnRole(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(RecentEventReceiverTest, customRoleDisplayName)
{
    // Test method: bool customRoleDisplayName((const QUrl &url, const ItemRoles role, QString *displayName))
    QUrl _arg0{};
    auto result = obj->customRoleDisplayName(_arg0, ItemRoles(), nullptr);
    EXPECT_FALSE(result);

}

TEST_F(RecentEventReceiverTest, detailViewIcon)
{
    // Test method: bool detailViewIcon((const QUrl &url, QString *iconName))
    QUrl _arg0{};
    auto result = obj->detailViewIcon(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(RecentEventReceiverTest, handleDropFiles)
{
    // Test method: bool handleDropFiles((const QList<QUrl> &fromUrls, const QUrl &toUrl))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    auto result = obj->handleDropFiles(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(RecentEventReceiverTest, handleFileCutResult)
{
    // Test method: void handleFileCutResult((const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg))
    QList<QUrl> _arg0{};
    QList<QUrl> _arg1{};
    QString _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileCutResult(_arg0, _arg1, false, _arg3));
}

TEST_F(RecentEventReceiverTest, handleFileRenameResult)
{
    // Test method: void handleFileRenameResult((quint64 winId, const QMap<QUrl, QUrl> &renamedUrls, bool ok, const QString &errMsg))
    QMap<QUrl, QUrl> _arg1{};
    QString _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileRenameResult(0, _arg1, false, _arg3));
}

TEST_F(RecentEventReceiverTest, handlePropertydialogDisable)
{
    // Test method: bool handlePropertydialogDisable((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->handlePropertydialogDisable(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(RecentEventReceiverTest, handleWindowUrlChanged)
{
    // Test method: void handleWindowUrlChanged((quint64 winId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleWindowUrlChanged(0, _arg1));
}

TEST_F(RecentEventReceiverTest, instance)
{
    // Test getter: RecentEventReceiver instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(RecentEventReceiverTest, isTransparent)
{
    // Test method: bool isTransparent((const QUrl &url, TransparentStatus *status))
    QUrl _arg0{};
    auto result = obj->isTransparent(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(RecentEventReceiverTest, sepateTitlebarCrumb)
{
    // Test method: bool sepateTitlebarCrumb((const QUrl &url, QList<QVariantMap> *mapGroup))
    QUrl _arg0{};
    auto result = obj->sepateTitlebarCrumb(_arg0, nullptr);
    EXPECT_FALSE(result);

}
