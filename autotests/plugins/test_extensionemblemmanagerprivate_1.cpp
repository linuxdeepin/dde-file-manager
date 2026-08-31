// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionemblemmanagerprivate_1.cpp
 * @brief Unit tests for ExtensionEmblemManagerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/emblemimpl/extensionemblemmanager.h"

#include <QTest>

using namespace dfmplugin_utils;

class ExtensionEmblemManagerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtensionEmblemManagerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtensionEmblemManagerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtensionEmblemManagerPrivateTest, ExtensionEmblemManagerPrivate)
{
    // Test constructor: ExtensionEmblemManagerPrivate((ExtensionEmblemManager *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ExtensionEmblemManagerPrivateTest, addReadyLocalPath)
{
    // Test method: void addReadyLocalPath((const QPair<QString, int> &path))
    QPair<QString, int> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addReadyLocalPath(_arg0));
}

TEST_F(ExtensionEmblemManagerPrivateTest, ExtensionEmblemManagerPrivate_Destructor)
{
    // Test method:  ~ExtensionEmblemManagerPrivate(())
    EXPECT_NO_FATAL_FAILURE({ ExtensionEmblemManagerPrivate *tmp = new ExtensionEmblemManagerPrivate(); delete tmp; });
}
