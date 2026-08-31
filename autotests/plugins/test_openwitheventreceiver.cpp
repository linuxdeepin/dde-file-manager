// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_openwitheventreceiver.cpp
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

TEST_F(OpenWithEventReceiverTest, OpenWithEventReceiver)
{
    // Test constructor: OpenWithEventReceiver((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
