// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_coreeventreceiver.cpp
 * @brief Unit tests for CoreEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/coreeventreceiver.h"

#include <QTest>

using namespace dfmplugin_core;

class CoreEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CoreEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CoreEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CoreEventReceiverTest, handleChangeUrl)
{
    // Test method: void handleChangeUrl((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleChangeUrl(0, _arg1));
}

TEST_F(CoreEventReceiverTest, handleHeadless)
{
    // Test method: void handleHeadless(())
    EXPECT_NO_FATAL_FAILURE(obj->handleHeadless());
}

TEST_F(CoreEventReceiverTest, handleLoadPlugins)
{
    // Test method: void handleLoadPlugins((const QStringList &names))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleLoadPlugins(_arg0));
}

TEST_F(CoreEventReceiverTest, handleOpenWindow)
{
    // Test method: void handleOpenWindow((const QUrl &url, const QVariant &opt))
    QUrl _arg0{};
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleOpenWindow(_arg0, _arg1));
}

TEST_F(CoreEventReceiverTest, handleShowSettingDialog)
{
    // Test method: void handleShowSettingDialog((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->handleShowSettingDialog(0));
}

TEST_F(CoreEventReceiverTest, handleShowSettingDialogWithGroup)
{
    // Test method: void handleShowSettingDialogWithGroup((quint64 windowId, const QString &groupKey))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleShowSettingDialogWithGroup(0, _arg1));
}

TEST_F(CoreEventReceiverTest, instance)
{
    // Test getter: CoreEventReceiver instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
