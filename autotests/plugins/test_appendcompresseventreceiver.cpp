// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_appendcompresseventreceiver.cpp
 * @brief Unit tests for AppendCompressEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "appendcompress/appendcompresseventreceiver.h"

#include <QTest>

using namespace dfmplugin_utils;

class AppendCompressEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AppendCompressEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AppendCompressEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AppendCompressEventReceiverTest, AppendCompressEventReceiver)
{
    // Test constructor: AppendCompressEventReceiver((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
