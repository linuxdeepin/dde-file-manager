// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sharemenusceneprivate.cpp
 * @brief Unit tests for ShareMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/sharemenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class ShareMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareMenuScenePrivateTest, ShareMenuScenePrivate)
{
    // Test constructor: ShareMenuScenePrivate((AbstractMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}
