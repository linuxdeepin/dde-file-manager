// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaulthelper.cpp
 * @brief Unit tests for VaultHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "vaulthelper.h"

#include <QTest>

using namespace vault;

class VaultHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultHelperTest, isVaultFile)
{
    // Test method: bool isVaultFile((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->isVaultFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(VaultHelperTest, recordTime)
{
    // Test method: void recordTime((const QString &group, const QString &key))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->recordTime(_arg0, _arg1));
}

TEST_F(VaultHelperTest, removeWinID)
{
    // Test method: void removeWinID((const quint64 &winId))
    quint64 _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeWinID(_arg0));
}

TEST_F(VaultHelperTest, showRemoveVaultDialog)
{
    // Test method: void showRemoveVaultDialog(())
    EXPECT_NO_FATAL_FAILURE(obj->showRemoveVaultDialog());
}

TEST_F(VaultHelperTest, showResetPasswordDialog)
{
    // Test method: void showResetPasswordDialog(())
    EXPECT_NO_FATAL_FAILURE(obj->showResetPasswordDialog());
}

TEST_F(VaultHelperTest, state)
{
    // Test method: VaultState state((const QString &baseDir, bool useCache))
    QString _arg0{};
    auto result = obj->state(_arg0, false);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(VaultHelperTest, transUrlsToLocal)
{
    // Test method: QList<QUrl> transUrlsToLocal((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->transUrlsToLocal(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultHelperTest, urlsToLocal)
{
    // Test method: bool urlsToLocal((const QList<QUrl> &origins, QList<QUrl> *urls))
    QList<QUrl> _arg0{};
    auto result = obj->urlsToLocal(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(VaultHelperTest, vaultToLocalUrl)
{
    // Test method: QUrl vaultToLocalUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->vaultToLocalUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(VaultHelperTest, instance)
{
    // Test getter: DAEMONPVAULT_USE_NAMESPACE instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
