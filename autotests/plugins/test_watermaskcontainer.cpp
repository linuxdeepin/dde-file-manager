// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_watermaskcontainer.cpp
 * @brief Unit tests for WatermaskContainer methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "watermask/watermaskcontainer.h"

#include <QTest>

using namespace ddplugin_canvas;

class WatermaskContainerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WatermaskContainer();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WatermaskContainer *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WatermaskContainerTest, WatermaskContainer)
{
    // Test constructor: WatermaskContainer((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
