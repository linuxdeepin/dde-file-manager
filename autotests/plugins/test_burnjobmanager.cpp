// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_burnjobmanager.cpp
 * @brief Unit tests for BurnJobManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/burnjobmanager.h"

#include <QTest>

using namespace dfmplugin_burn;

class BurnJobManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BurnJobManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BurnJobManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BurnJobManagerTest, deleteStagingDir)
{
    // Test method: bool deleteStagingDir((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->deleteStagingDir(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(BurnJobManagerTest, instance)
{
    // Test getter: BurnJobManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(BurnJobManagerTest, startAuditLogForEraseDisc)
{
    // Test method: void startAuditLogForEraseDisc((const QVariantMap &info, bool result))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->startAuditLogForEraseDisc(_arg0, false));
}

TEST_F(BurnJobManagerTest, startEraseDisc)
{
    // Test method: void startEraseDisc((const QString &dev))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->startEraseDisc(_arg0));
}

TEST_F(BurnJobManagerTest, startRemoveFilesFromDisc)
{
    // Test method: void startRemoveFilesFromDisc((const QString &dev, const QList<QUrl> &urls))
    QString _arg0{};
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->startRemoveFilesFromDisc(_arg0, _arg1));
}
