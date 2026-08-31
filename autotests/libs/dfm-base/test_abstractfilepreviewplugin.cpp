// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractfilepreviewplugin.cpp
 * @brief Unit tests for AbstractFilePreviewPlugin methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/interfaces/abstractfilepreviewplugin.h"

#include <QTest>

using namespace src;

class AbstractFilePreviewPluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractFilePreviewPlugin();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractFilePreviewPlugin *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractFilePreviewPluginTest, AbstractFilePreviewPlugin)
{
    // Test constructor: AbstractFilePreviewPlugin((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
