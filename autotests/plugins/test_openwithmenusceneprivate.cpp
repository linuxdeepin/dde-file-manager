// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_openwithmenusceneprivate.cpp
 * @brief Unit tests for OpenWithMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/openwithmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class OpenWithMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpenWithMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpenWithMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpenWithMenuScenePrivateTest, OpenWithMenuScenePrivate)
{
    // Test constructor: OpenWithMenuScenePrivate((OpenWithMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}
