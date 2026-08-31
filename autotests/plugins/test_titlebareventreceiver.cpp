// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_titlebareventreceiver.cpp
 * @brief Unit tests for TitleBarEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/titlebareventreceiver.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class TitleBarEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TitleBarEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TitleBarEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TitleBarEventReceiverTest, handleCustomRegister)
{
    // Test method: bool handleCustomRegister((const QString &scheme, const QVariantMap &properties))
    QString _arg0{};
    QVariantMap _arg1{};
    auto result = obj->handleCustomRegister(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TitleBarEventReceiverTest, handleRemoveHistory)
{
    // Test method: void handleRemoveHistory((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleRemoveHistory(0, _arg1));
}
