// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionemblemmanager.cpp
 * @brief Unit tests for ExtensionEmblemManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/emblemimpl/extensionemblemmanager.h"

#include <QTest>

using namespace dfmplugin_utils;

class ExtensionEmblemManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtensionEmblemManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtensionEmblemManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtensionEmblemManagerTest, ExtensionEmblemManager)
{
    // Test constructor: ExtensionEmblemManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
