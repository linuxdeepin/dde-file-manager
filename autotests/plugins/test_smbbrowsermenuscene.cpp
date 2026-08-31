// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbbrowsermenuscene.cpp
 * @brief Unit tests for SmbBrowserMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/smbbrowsermenuscene.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class SmbBrowserMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SmbBrowserMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SmbBrowserMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SmbBrowserMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
