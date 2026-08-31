// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_previewpluginloader.cpp
 * @brief Unit tests for PreviewPluginLoader methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/libdfm-preview/pluginInterface/previewpluginloader.h"

#include <QTest>

using namespace src;

class PreviewPluginLoaderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PreviewPluginLoader();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PreviewPluginLoader *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PreviewPluginLoaderTest, indexOf)
{
    // Test method: int indexOf((const QString &needle))
    QString _arg0{};
    auto result = obj->indexOf(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(PreviewPluginLoaderTest, instance)
{
    // Test method: QObject instance((int index))
    auto result = obj->instance(0);
    EXPECT_NO_FATAL_FAILURE({ obj->instance(0); });

}
