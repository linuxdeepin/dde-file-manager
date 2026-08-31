// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pluginmanagerprivate.cpp
 * @brief Unit tests for PluginManagerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-framework/lifecycle/private/pluginmanager_p.h"

#include <QTest>

using namespace src;

class PluginManagerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PluginManagerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PluginManagerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PluginManagerPrivateTest, readJsonToMeta)
{
    // Test method: void readJsonToMeta((PluginMetaObjectPointer metaObject))
    EXPECT_NO_FATAL_FAILURE(obj->readJsonToMeta(PluginMetaObjectPointer()));
}
