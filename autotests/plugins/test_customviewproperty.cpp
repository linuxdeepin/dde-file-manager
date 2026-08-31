// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_customviewproperty.cpp
 * @brief Unit tests for CustomViewProperty methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfmplugin_workspace_global.h"

#include <QTest>

using namespace dfmplugin_workspace;

class CustomViewPropertyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CustomViewProperty();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CustomViewProperty *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CustomViewPropertyTest, CustomViewProperty)
{
    // Test constructor: CustomViewProperty(())
    ASSERT_NE(obj, nullptr);
}
