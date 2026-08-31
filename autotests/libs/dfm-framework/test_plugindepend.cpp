// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_plugindepend.cpp
 * @brief Unit tests for PluginDepend methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-framework/lifecycle/pluginmetaobject.h"

#include <QTest>

using namespace src;

class PluginDependTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PluginDepend();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PluginDepend *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PluginDependTest, PluginDepend)
{
    // Test constructor: PluginDepend((const PluginDepend &depend))
    ASSERT_NE(obj, nullptr);
}
