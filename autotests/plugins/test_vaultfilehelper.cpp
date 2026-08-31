// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultfilehelper.cpp
 * @brief Unit tests for VaultFileHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/vaultfilehelper.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultFileHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultFileHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultFileHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultFileHelperTest, checkDragDropAction)
{
    // Test method: bool checkDragDropAction((const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    auto result = obj->checkDragDropAction(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(VaultFileHelperTest, deleteFile)
{
    // Test method: bool deleteFile((const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlags flags))
    auto result = obj->deleteFile(0, QList<QUrl>(), AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}

TEST_F(VaultFileHelperTest, handleDropFiles)
{
    // Test method: bool handleDropFiles((const QList<QUrl> &fromUrls, const QUrl &toUrl))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    auto result = obj->handleDropFiles(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(VaultFileHelperTest, scheme)
{
    // Test getter: QString scheme()
    auto result = obj->scheme();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultFileHelperTest, writeUrlsToClipboard)
{
    // Test method: bool writeUrlsToClipboard((const quint64 windowId, const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action, const QList<QUrl> urls))
    auto result = obj->writeUrlsToClipboard(0, DFMBASE_NAMESPACE::ClipBoard::ClipboardAction(), QList<QUrl>());
    EXPECT_FALSE(result);

}
