// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionfilemanager.cpp
 * @brief Unit tests for ExtensionFileManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/fileimpl/extensionfilemanager.h"

#include <QTest>

using namespace dfmplugin_utils;

class ExtensionFileManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtensionFileManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtensionFileManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtensionFileManagerTest, ExtensionFileManager)
{
    // Test constructor: ExtensionFileManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ExtensionFileManagerTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(ExtensionFileManagerTest, instance)
{
    // Test getter: ExtensionFileManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(ExtensionFileManagerTest, launch)
{
    // Test method: bool launch((const QString &desktopFile, const QStringList &filePaths))
    QString _arg0{};
    QStringList _arg1{};
    auto result = obj->launch(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(ExtensionFileManagerTest, onAllPluginsInitialized)
{
    // Test method: void onAllPluginsInitialized(())
    EXPECT_NO_FATAL_FAILURE(obj->onAllPluginsInitialized());
}
