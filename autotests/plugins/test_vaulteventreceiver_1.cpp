// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaulteventreceiver_1.cpp
 * @brief Unit tests for VaultEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/vaulteventreceiver.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultEventReceiverTest, connectEvent)
{
    // Test method: void connectEvent(())
    EXPECT_NO_FATAL_FAILURE(obj->connectEvent());
}

TEST_F(VaultEventReceiverTest, detailViewIcon)
{
    // Test method: bool detailViewIcon((const QUrl &url, QString *iconName))
    QUrl _arg0{};
    auto result = obj->detailViewIcon(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(VaultEventReceiverTest, fileDropHandleWithAction)
{
    // Test method: bool fileDropHandleWithAction((const QList<QUrl> &fromUrls, const QUrl &toUrl, Qt::DropAction *action))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    auto result = obj->fileDropHandleWithAction(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(VaultEventReceiverTest, handleCurrentUrlChanged)
{
    // Test method: void handleCurrentUrlChanged((const quint64 &winId, const QUrl &url))
    quint64 _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleCurrentUrlChanged(_arg0, _arg1));
}

TEST_F(VaultEventReceiverTest, handleFileCanTaged)
{
    // Test method: bool handleFileCanTaged((const QUrl &url, bool *canTag))
    QUrl _arg0{};
    auto result = obj->handleFileCanTaged(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(VaultEventReceiverTest, handlePathtoVirtual)
{
    // Test method: bool handlePathtoVirtual((const QList<QUrl> files, QList<QUrl> *virtualFiles))
    auto result = obj->handlePathtoVirtual(QList<QUrl>(), nullptr);
    EXPECT_FALSE(result);

}

TEST_F(VaultEventReceiverTest, handlePermissionViewAsh)
{
    // Test method: bool handlePermissionViewAsh((const QUrl &url, bool *isAsh))
    QUrl _arg0{};
    auto result = obj->handlePermissionViewAsh(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(VaultEventReceiverTest, handleShortCutPasteFiles)
{
    // Test method: bool handleShortCutPasteFiles((const quint64 &winId, const QList<QUrl> &fromUrls, const QUrl &to))
    quint64 _arg0{};
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    auto result = obj->handleShortCutPasteFiles(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(VaultEventReceiverTest, handleSideBarItemDragMoveData)
{
    // Test method: bool handleSideBarItemDragMoveData((const QList<QUrl> &urls, const QUrl &url, Qt::DropAction *action))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    auto result = obj->handleSideBarItemDragMoveData(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}
