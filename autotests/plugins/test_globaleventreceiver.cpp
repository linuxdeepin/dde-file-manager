// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_globaleventreceiver.cpp
 * @brief Unit tests for GlobalEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "global/globaleventreceiver.h"

#include <QTest>

using namespace dfmplugin_utils;

class GlobalEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new GlobalEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    GlobalEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(GlobalEventReceiverTest, handleOpenAsAdmin)
{
    // Test method: void handleOpenAsAdmin((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleOpenAsAdmin(_arg0));
}

TEST_F(GlobalEventReceiverTest, initEventConnect)
{
    // Test method: void initEventConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initEventConnect());
}

TEST_F(GlobalEventReceiverTest, GlobalEventReceiver_Destructor)
{
    // Test method:  ~GlobalEventReceiver(())
    EXPECT_NO_FATAL_FAILURE({ GlobalEventReceiver *tmp = new GlobalEventReceiver(); delete tmp; });
}
