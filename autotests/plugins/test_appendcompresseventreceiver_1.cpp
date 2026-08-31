// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_appendcompresseventreceiver_1.cpp
 * @brief Unit tests for AppendCompressEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "appendcompress/appendcompresseventreceiver.h"

#include <QTest>

using namespace dfmplugin_utils;

class AppendCompressEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AppendCompressEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AppendCompressEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AppendCompressEventReceiverTest, handleDragDropCompress)
{
    // Test method: bool handleDragDropCompress((const QList<QUrl> &fromUrls, const QUrl &toUrl))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    auto result = obj->handleDragDropCompress(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(AppendCompressEventReceiverTest, handleDragDropCompressOnDesktop)
{
    // Test method: bool handleDragDropCompressOnDesktop((int viewIndex, const QMimeData *md, const QPoint &viewPos, void *extData))
    QPoint _arg2{};
    auto result = obj->handleDragDropCompressOnDesktop(0, nullptr, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(AppendCompressEventReceiverTest, handleDragDropCompressOnOsrganizer)
{
    // Test method: bool handleDragDropCompressOnOsrganizer((const QString &viewId, const QMimeData *md, const QPoint &viewPos, void *extData))
    QString _arg0{};
    QPoint _arg2{};
    auto result = obj->handleDragDropCompressOnOsrganizer(_arg0, nullptr, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(AppendCompressEventReceiverTest, handleIsDrop)
{
    // Test method: bool handleIsDrop((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->handleIsDrop(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(AppendCompressEventReceiverTest, handleSetMouseStyle)
{
    // Test method: bool handleSetMouseStyle((const QList<QUrl> &fromUrls, const QUrl &toUrl, Qt::DropAction *type))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    auto result = obj->handleSetMouseStyle(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(AppendCompressEventReceiverTest, handleSetMouseStyleOnDesktop)
{
    // Test method: bool handleSetMouseStyleOnDesktop((int viewIndex, const QMimeData *mime, const QPoint &viewPos, void *extData))
    QPoint _arg2{};
    auto result = obj->handleSetMouseStyleOnDesktop(0, nullptr, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(AppendCompressEventReceiverTest, handleSetMouseStyleOnOrganizer)
{
    // Test method: bool handleSetMouseStyleOnOrganizer((const QString &viewId, const QMimeData *mime, const QPoint &viewPos, void *extData))
    QString _arg0{};
    QPoint _arg2{};
    auto result = obj->handleSetMouseStyleOnOrganizer(_arg0, nullptr, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(AppendCompressEventReceiverTest, initEventConnect)
{
    // Test method: void initEventConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initEventConnect());
}
