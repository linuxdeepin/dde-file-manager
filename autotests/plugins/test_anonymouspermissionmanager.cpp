// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_anonymouspermissionmanager.cpp
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

TEST_F(AnonymousPermissionManagerTest, instance)
{
    // Test getter: DFMBASE_USE_NAMESPACE instance()
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });
}

TEST_F(AnonymousPermissionManagerTest, loadFromFile)
{
    // Test bool getter: loadFromFile()
    bool result = obj->loadFromFile();
    EXPECT_FALSE(result);

}

TEST_F(AnonymousPermissionManagerTest, saveToFile)
{
    // Test bool getter: saveToFile()
    bool result = obj->saveToFile();
    EXPECT_FALSE(result);

}

TEST_F(AnonymousPermissionManagerTest, setAnonymousPermissions)
{
    // Test method: bool setAnonymousPermissions((const QString &filePath, DirectoryType type, bool writable))
    QString _arg0{};
    auto result = obj->setAnonymousPermissions(_arg0, DirectoryType(), false);
    EXPECT_FALSE(result);

}
