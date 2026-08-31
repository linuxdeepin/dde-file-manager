// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pluginloader.cpp
 * @brief Unit tests for PluginLoader methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-extractor/extractor/pluginloader.h"

#include <QTest>

using namespace src;

class PluginLoaderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PluginLoader();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PluginLoader *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PluginLoaderTest, plugins)
{
    // Test getter: QList<QSharedPointer<AbstractExtractorPlugin>> plugins()
    auto result = obj->plugins();
    EXPECT_TRUE(result.isEmpty());

}
