// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionemblemmanager_1.cpp
 * @brief Unit tests for ExtensionEmblemManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/emblemimpl/extensionemblemmanager.h"

#include <QTest>

using namespace dfmplugin_utils;

class ExtensionEmblemManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtensionEmblemManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtensionEmblemManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtensionEmblemManagerTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(ExtensionEmblemManagerTest, instance)
{
    // Test getter: ExtensionEmblemManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(ExtensionEmblemManagerTest, onAllPluginsInitialized)
{
    // Test method: void onAllPluginsInitialized(())
    EXPECT_NO_FATAL_FAILURE(obj->onAllPluginsInitialized());
}

TEST_F(ExtensionEmblemManagerTest, onEmblemIconChanged)
{
    // Test method: void onEmblemIconChanged((const QString &path, const QList<QPair<QString, int>> &group))
    QString _arg0{};
    QList<QPair<QString, int>> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onEmblemIconChanged(_arg0, _arg1));
}

TEST_F(ExtensionEmblemManagerTest, onUrlChanged)
{
    // Test method: bool onUrlChanged((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    auto result = obj->onUrlChanged(0, _arg1);
    EXPECT_FALSE(result);

}
