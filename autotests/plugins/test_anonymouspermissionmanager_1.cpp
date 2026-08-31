// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_anonymouspermissionmanager_1.cpp
 * @brief Unit tests for AnonymousPermissionManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/anonymouspermissionmanager.h"

#include <QTest>

using namespace dfmplugin_dirshare;

class AnonymousPermissionManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AnonymousPermissionManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AnonymousPermissionManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AnonymousPermissionManagerTest, AnonymousPermissionManager)
{
    // Test constructor: AnonymousPermissionManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AnonymousPermissionManagerTest, cleanRecord)
{
    // Test method: void cleanRecord((const QString &filePath))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->cleanRecord(_arg0));
}

TEST_F(AnonymousPermissionManagerTest, getCurrentAnonymousShareCount)
{
    // Test getter: int getCurrentAnonymousShareCount()
    auto result = obj->getCurrentAnonymousShareCount();
    EXPECT_EQ(result, 0);

}

TEST_F(AnonymousPermissionManagerTest, getExpectedPermissions)
{
    // Test method: QFile::Permissions getExpectedPermissions((QFile::Permissions original))
    auto result = obj->getExpectedPermissions(QFile::Permissions());
    EXPECT_NO_FATAL_FAILURE({ obj->getExpectedPermissions(QFile::Permissions()); });

}

TEST_F(AnonymousPermissionManagerTest, getHomeExpectedPermissions)
{
    // Test method: QFile::Permissions getHomeExpectedPermissions((QFile::Permissions original))
    auto result = obj->getHomeExpectedPermissions(QFile::Permissions());
    EXPECT_NO_FATAL_FAILURE({ obj->getHomeExpectedPermissions(QFile::Permissions()); });

}

TEST_F(AnonymousPermissionManagerTest, restoreDirectoryPermissions)
{
    // Test method: bool restoreDirectoryPermissions((const QString &filePath))
    QString _arg0{};
    auto result = obj->restoreDirectoryPermissions(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(AnonymousPermissionManagerTest, restoreHomeDirectoryIfNoAnonymousShares)
{
    // Test bool getter: restoreHomeDirectoryIfNoAnonymousShares()
    bool result = obj->restoreHomeDirectoryIfNoAnonymousShares();
    EXPECT_FALSE(result);

}
