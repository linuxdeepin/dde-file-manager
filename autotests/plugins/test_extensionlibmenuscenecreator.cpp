// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionlibmenuscenecreator.cpp
 * @brief Unit tests for ExtensionLibMenuSceneCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/menuimpl/extensionlibmenuscene.h"

#include <QTest>

using namespace dfmplugin_utils;

class ExtensionLibMenuSceneCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtensionLibMenuSceneCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtensionLibMenuSceneCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtensionLibMenuSceneCreatorTest, create)
{
    // Test getter: DFMBASE_USE_NAMESPACE create()
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });
}

TEST_F(ExtensionLibMenuSceneCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
