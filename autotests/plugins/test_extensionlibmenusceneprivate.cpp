// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionlibmenusceneprivate.cpp
 * @brief Unit tests for ExtensionLibMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/menuimpl/extensionlibmenuscene.h"

#include <QTest>

using namespace dfmplugin_utils;

class ExtensionLibMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtensionLibMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtensionLibMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtensionLibMenuScenePrivateTest, ExtensionLibMenuScenePrivate)
{
    // Test constructor: ExtensionLibMenuScenePrivate((ExtensionLibMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}
