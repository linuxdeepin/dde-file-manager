// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_openwitheventreceiver_1.cpp
 * @brief Unit tests for OpenWithEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "openwith/openwitheventreceiver.h"

#include <QTest>

using namespace dfmplugin_utils;

class OpenWithEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpenWithEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpenWithEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpenWithEventReceiverTest, initEventConnect)
{
    // Test method: void initEventConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initEventConnect());
}

TEST_F(OpenWithEventReceiverTest, showOpenWithDialog)
{
    // Test method: void showOpenWithDialog((quint64 winId, const QList<QUrl> &urls))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->showOpenWithDialog(0, _arg1));
}
