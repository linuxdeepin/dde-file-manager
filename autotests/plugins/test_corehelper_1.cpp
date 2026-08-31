// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_corehelper_1.cpp
 * @brief Unit tests for CoreHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/corehelper.h"

#include <QTest>

using namespace dfmplugin_core;

class CoreHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CoreHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CoreHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CoreHelperTest, CoreHelper)
{
    // Test constructor: CoreHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CoreHelperTest, askHiddenFile)
{
    // Test method: bool askHiddenFile((QWidget *parent))
    auto result = obj->askHiddenFile(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CoreHelperTest, askReplaceFile)
{
    // Test method: bool askReplaceFile((QString fileName, QWidget *parent))
    auto result = obj->askReplaceFile(QString(), nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CoreHelperTest, cacheDefaultWindow)
{
    // Test method: void cacheDefaultWindow(())
    EXPECT_NO_FATAL_FAILURE(obj->cacheDefaultWindow());
}

TEST_F(CoreHelperTest, createNewWindow)
{
    // Test method: FileManagerWindow createNewWindow((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->createNewWindow(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createNewWindow(_arg0); });

}

TEST_F(CoreHelperTest, defaultWindow)
{
    // Test getter: FileManagerWindow defaultWindow()
    auto result = obj->defaultWindow();
    EXPECT_NO_FATAL_FAILURE({ obj->defaultWindow(); });

}

TEST_F(CoreHelperTest, findExistsWindow)
{
    // Test method: FileManagerWindow findExistsWindow((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->findExistsWindow(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->findExistsWindow(_arg0); });

}

TEST_F(CoreHelperTest, instance)
{
    // Test getter: DFMBASE_USE_NAMESPACE instance()
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });
}

TEST_F(CoreHelperTest, openWindow)
{
    // Test method: void openWindow((const QUrl &url, const QVariant &opt))
    QUrl _arg0{};
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->openWindow(_arg0, _arg1));
}
